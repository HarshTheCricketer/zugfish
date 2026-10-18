#include "reviewer.hpp"
#include <chess.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>

namespace zugfish {

std::string to_string(MoveClassification c) {
    switch (c) {
        case MoveClassification::Brilliant: return "Brilliant";
        case MoveClassification::Great:     return "Great";
        case MoveClassification::Best:      return "Best";
        case MoveClassification::Excellent: return "Excellent";
        case MoveClassification::Good:      return "Good";
        case MoveClassification::Book:      return "Book";
        case MoveClassification::Inaccuracy:return "Inaccuracy";
        case MoveClassification::Mistake:   return "Mistake";
        case MoveClassification::Blunder:   return "Blunder";
        case MoveClassification::Miss:      return "Miss";
        case MoveClassification::Forced:    return "Forced";
    }
    return "Unknown";
}

std::string to_symbol(MoveClassification c) {
    switch (c) {
        case MoveClassification::Brilliant: return "!!";
        case MoveClassification::Great:     return "!";
        case MoveClassification::Best:      return "[*]";
        case MoveClassification::Excellent: return "[!]";
        case MoveClassification::Good:      return "[+]";
        case MoveClassification::Book:      return "[B]";
        case MoveClassification::Inaccuracy:return "[?!]";
        case MoveClassification::Mistake:   return "[?]";
        case MoveClassification::Blunder:   return "[??]";
        case MoveClassification::Miss:      return "[X]";
        case MoveClassification::Forced:    return "[]";
    }
    return "";
}

GameReviewer::GameReviewer(const std::string& engine_path)
    : engine_(engine_path) {}

GameReviewer::GameReviewer(UciEngine engine)
    : engine_(std::move(engine)) {}

static void update_stats(PlayerStats& stats, MoveClassification c) {
    switch (c) {
        case MoveClassification::Brilliant: stats.brilliant_count++; break;
        case MoveClassification::Great:     stats.great_count++; break;
        case MoveClassification::Best:      stats.best_count++; break;
        case MoveClassification::Excellent: stats.excellent_count++; break;
        case MoveClassification::Good:      stats.good_count++; break;
        case MoveClassification::Book:      stats.book_count++; break;
        case MoveClassification::Inaccuracy: stats.inaccuracy_count++; break;
        case MoveClassification::Mistake:   stats.mistake_count++; break;
        case MoveClassification::Blunder:   stats.blunder_count++; break;
        case MoveClassification::Miss:      stats.miss_count++; break;
        case MoveClassification::Forced:    stats.forced_count++; break;
    }
}

static int piece_val(chess::PieceType pt) {
    switch (pt.internal()) {
        case chess::PieceType::underlying::PAWN:   return 1;
        case chess::PieceType::underlying::KNIGHT: return 3;
        case chess::PieceType::underlying::BISHOP: return 3;
        case chess::PieceType::underlying::ROOK:   return 5;
        case chess::PieceType::underlying::QUEEN:  return 9;
        default: return 0;
    }
}

static int total_material(const chess::Board& board, chess::Color color) {
    int sum = 0;
    for (int sq = 0; sq < 64; ++sq) {
        chess::Piece p = board.at(chess::Square(sq));
        if (p != chess::Piece::NONE && p.color() == color) {
            sum += piece_val(p.type());
        }
    }
    return sum;
}

static std::string uci_to_san(const std::string& fen, const std::string& uci) {
    if (uci.empty() || uci == "(none)") return "";
    try {
        chess::Board board(fen);
        chess::Move m = chess::uci::uciToMove(board, uci);
        return chess::uci::moveToSan(board, m);
    } catch (...) {
        return uci;
    }
}

static double compute_game_accuracy(const std::vector<double>& decision_accs,
                                    const std::vector<double>& all_accs) {
    const auto& accs = decision_accs.empty() ? all_accs : decision_accs;
    if (accs.empty()) return 100.0;

    double sum = std::accumulate(accs.begin(), accs.end(), 0.0);
    double arith_mean = sum / accs.size();

    double harmonic_sum = 0.0;
    for (double a : accs) {
        harmonic_sum += 1.0 / std::max(a, 1.0);
    }
    double harmonic_mean = static_cast<double>(accs.size()) / harmonic_sum;

    // Chess.com CAPS2 weighting: 25% arithmetic + 75% harmonic mean.
    // Gives realistic 40-65% scores for games with multiple blunders/mistakes,
    // while preserving 92-99% on masterly played games.
    double blended = (0.25 * arith_mean) + (0.75 * harmonic_mean);
    return std::clamp(blended, 0.0, 100.0);
}

GameReview GameReviewer::review(const Game& game, int depth, ProgressCallback progress_cb, CancelCallback cancel_cb) {
    GameReview review;
    review.headers = game.headers;

    if (game.plies.empty()) {
        return review;
    }

    engine_.new_game();

    // Determine the position after the final move
    std::string after_last_fen;
    try {
        chess::Board last_board(game.plies.back().fen_before);
        chess::Move m = chess::uci::uciToMove(last_board, game.plies.back().uci);
        last_board.makeMove(m);
        after_last_fen = last_board.getFen();
    } catch (...) {
        after_last_fen = game.plies.back().fen_before;
    }

    // Evaluate initial position before move 0
    engine_.set_position_fen(game.plies[0].fen_before);
    auto prev_eval = engine_.analyze_depth(depth);
    review.initial_eval = prev_eval;
    review.initial_best_san = uci_to_san(game.plies[0].fen_before, prev_eval.best_move);

    std::vector<double> white_decision_accuracies;
    std::vector<double> black_decision_accuracies;
    std::vector<double> white_all_accuracies;
    std::vector<double> black_all_accuracies;
    std::vector<int> white_cpls;
    std::vector<int> black_cpls;

    std::vector<std::string> uci_history;
    bool still_in_book = true;

    for (size_t i = 0; i < game.plies.size(); ++i) {
        if (cancel_cb && cancel_cb()) {
            break;
        }

        const auto& ply = game.plies[i];
        bool white_to_move = (ply.fen_before.find(" w ") != std::string::npos);

        std::string next_fen = (i + 1 < game.plies.size()) 
                               ? game.plies[i + 1].fen_before 
                               : after_last_fen;

        engine_.set_position_fen(next_fen);
        auto next_eval = engine_.analyze_depth(depth);

        // Win probabilities from White's perspective
        // In prev_eval: side to move is white_to_move
        // In next_eval: side to move is !white_to_move (the opponent)
        double win_before_white = prev_eval.score.win_probability(white_to_move);
        double win_after_white  = next_eval.score.win_probability(!white_to_move);

        // Win probabilities from the moving player's perspective
        double player_win_before = white_to_move ? win_before_white : (1.0 - win_before_white);
        double player_win_after  = white_to_move ? win_after_white  : (1.0 - win_after_white);

        double win_loss = 0.0;
        if (ply.uci == prev_eval.best_move) {
            win_loss = 0.0;
        } else {
            win_loss = std::max(0.0, player_win_before - player_win_after);
        }

        // Centipawn loss
        auto to_cp = [](const Score& s) -> int {
            if (s.is_mate) {
                return s.value > 0 ? 1000 : -1000;
            }
            return std::clamp(s.value, -1000, 1000);
        };

        Score score_before_white = prev_eval.score.to_white_perspective(white_to_move);
        Score score_after_white  = next_eval.score.to_white_perspective(!white_to_move);

        int cp_before = white_to_move ? to_cp(score_before_white) : -to_cp(score_before_white);
        int cp_after  = white_to_move ? to_cp(score_after_white)  : -to_cp(score_after_white);

        int cpl = (ply.uci == prev_eval.best_move) ? 0 : std::max(0, cp_before - cp_after);

        // 1. Check Forced Move
        chess::Board board_before(ply.fen_before);
        chess::Movelist legals;
        chess::movegen::legalmoves(legals, board_before);
        bool is_forced = (legals.size() == 1);

        // 2. Check Book Move
        bool is_book = false;
        if (still_in_book) {
            if (OpeningBook::instance().is_book_move(uci_history, ply.uci)) {
                is_book = true;
            } else {
                still_in_book = false;
            }
        }
        uci_history.push_back(ply.uci);

        // 3. Check Brilliant Sacrifice (Strict Chess.com Standard)
        bool is_best_move = (ply.uci == prev_eval.best_move);
        bool is_brilliant = false;
        
        chess::Color player_c = white_to_move ? chess::Color::WHITE : chess::Color::BLACK;
        chess::Color opp_c    = white_to_move ? chess::Color::BLACK : chess::Color::WHITE;
        int mat_player = total_material(board_before, player_c);
        int mat_opp    = total_material(board_before, opp_c);
        int material_lead = mat_player - mat_opp;

        // Strict Criteria for Brilliant (!!):
        // a) Must be the top engine move
        // b) Not a book move, not forced
        // c) Must not already have overwhelming material dominance (material_lead <= 3 pawns)
        // d) Position must be completely sound after (player_win_after >= 0.50 and win_loss <= 0.01)
        // e) Must involve an intentional piece sacrifice directly initiated by this move
        if (is_best_move && !is_book && !is_forced && 
            material_lead <= 3 && player_win_after >= 0.50 && win_loss <= 0.01) {

            try {
                chess::Move move_played = chess::uci::uciToMove(board_before, ply.uci);
                chess::Piece moved_piece = board_before.at(move_played.from());
                chess::Square to_sq = move_played.to();
                int moved_val = piece_val(moved_piece.type());

                // Pawn pushes are not piece sacrifices; only minor pieces, rooks, queens
                if (moved_val >= 3) {
                    chess::Board b_after(next_fen);
                    // Check if opponent's best response captures the sacrificed piece
                    if (!next_eval.best_move.empty() && next_eval.best_move != "(none)") {
                        chess::Move opp_move = chess::uci::uciToMove(b_after, next_eval.best_move);

                        // Opponent captures the moved piece on its destination square
                        if (opp_move.to() == to_sq && b_after.isCapture(opp_move)) {
                            // Net material sacrificed in the exchange
                            // e.g. Morphy 13. Rxd7 (Rook for Knight, 5 - 3 = 2)
                            // e.g. Morphy 16. Qb8+ (Queen for 0, 9 - 0 = 9)
                            chess::Piece cap_piece = board_before.at(to_sq);
                            int cap_val = (cap_piece != chess::Piece::NONE) ? piece_val(cap_piece.type()) : 0;
                            int net_sac = moved_val - cap_val;

                            if (net_sac >= 2) {
                                is_brilliant = true;
                            }
                        }
                    }
                }
            } catch (...) {}
        }

        // 4. Check Great Move (!)
        bool is_great = false;
        if (is_best_move && !is_book && !is_forced && !is_brilliant) {
            // A Great move is a critical move that creates a decisive winning breakthrough
            // (>= 12% win chance jump to >= 70%) or finds the only saving defense from a losing position
            if ((player_win_before <= 0.72 && player_win_after >= 0.72 && (player_win_after - player_win_before) >= 0.12) ||
                (player_win_before <= 0.30 && player_win_after >= 0.45 && (player_win_after - player_win_before) >= 0.15)) {
                is_great = true;
            }
        }

        // Move classification
        MoveClassification classification;
        if (is_book) {
            classification = MoveClassification::Book;
        } else if (is_forced) {
            classification = MoveClassification::Forced;
        } else if (is_brilliant) {
            classification = MoveClassification::Brilliant;
        } else if (is_great) {
            classification = MoveClassification::Great;
        } else if (is_best_move || win_loss <= 0.005) {
            classification = MoveClassification::Best;
        } else if (win_loss <= 0.025) {
            classification = MoveClassification::Excellent;
        } else if (win_loss <= 0.060) {
            classification = MoveClassification::Good;
        } else if (win_loss <= 0.140) {
            classification = MoveClassification::Inaccuracy;
        } else if (win_loss <= 0.250) {
            classification = MoveClassification::Mistake;
        } else {
            if (player_win_before >= 0.75) {
                classification = MoveClassification::Miss;
            } else {
                classification = MoveClassification::Blunder;
            }
        }

        // Accuracy score for this single move [0.0, 100.0] via CAPS2 exponential decay formula:
        // Best moves score 100%, minor inaccuracies score 70-80%, mistakes score 20-35%,
        // and blunders/misses drop to 0-10%.
        double delta_win_pct = std::max(0.0, win_loss * 100.0);
        double move_accuracy = 100.0;
        if (delta_win_pct > 0.0) {
            double raw = 103.0 * std::exp(-0.065 * delta_win_pct) - 3.0;
            move_accuracy = std::clamp(raw, 0.0, 100.0);
        }

        if (white_to_move) {
            white_all_accuracies.push_back(move_accuracy);
            if (!is_book && !is_forced) {
                white_decision_accuracies.push_back(move_accuracy);
            }
            white_cpls.push_back(cpl);
            update_stats(review.white_stats, classification);
        } else {
            black_all_accuracies.push_back(move_accuracy);
            if (!is_book && !is_forced) {
                black_decision_accuracies.push_back(move_accuracy);
            }
            black_cpls.push_back(cpl);
            update_stats(review.black_stats, classification);
        }

        AnalyzedPly analyzed;
        analyzed.ply = ply;
        analyzed.before_eval = prev_eval;
        analyzed.after_eval = next_eval;
        analyzed.best_was_san = uci_to_san(ply.fen_before, prev_eval.best_move);
        analyzed.next_best_san = uci_to_san(next_fen, next_eval.best_move);
        analyzed.score_white_perspective = score_after_white;
        analyzed.win_percent_white = win_after_white * 100.0;
        analyzed.win_percent_loss = win_loss;
        analyzed.centipawn_loss = cpl;
        analyzed.classification = classification;

        review.plies.push_back(analyzed);

        if (progress_cb) {
            ReviewProgress prog;
            prog.current_ply = i + 1;
            prog.total_plies = game.plies.size();
            prog.move_san = ply.san;
            prog.last_classification = classification;
            prog.current_accuracy_white = compute_game_accuracy(white_decision_accuracies, white_all_accuracies);
            prog.current_accuracy_black = compute_game_accuracy(black_decision_accuracies, black_all_accuracies);

            progress_cb(prog);
        }

        prev_eval = std::move(next_eval);
    }

    auto avg_int = [](const std::vector<int>& v) {
        if (v.empty()) return 0.0;
        return static_cast<double>(std::accumulate(v.begin(), v.end(), 0)) / v.size();
    };

    review.white_stats.accuracy = compute_game_accuracy(white_decision_accuracies, white_all_accuracies);
    review.white_stats.acpl = avg_int(white_cpls);
    review.black_stats.accuracy = compute_game_accuracy(black_decision_accuracies, black_all_accuracies);
    review.black_stats.acpl = avg_int(black_cpls);

    review.opening = OpeningBook::instance().identify_opening(uci_history);

    return review;
}

void GameReviewer::print_review(const GameReview& review, std::ostream& os) {
    auto get_header = [&](const std::string& key, const std::string& def = "?") {
        auto it = review.headers.find(key);
        return it != review.headers.end() ? it->second : def;
    };

    os << "\n================================================================================\n";
    os << "                           ZUGFISH GAME REVIEW\n";
    os << "================================================================================\n";
    os << " White: " << std::left << std::setw(32) << get_header("White")
       << " Black: " << get_header("Black") << "\n";
    if (!review.opening.name.empty()) {
        os << " Opening:  " << review.opening.eco << " - " << review.opening.name << "\n";
    }
    os << " Accuracy: " << std::fixed << std::setprecision(1) << review.white_stats.accuracy << "%"
       << std::setw(26) << ""
       << " Accuracy: " << review.black_stats.accuracy << "%\n";
    os << " ACPL: " << std::setprecision(1) << review.white_stats.acpl
       << std::setw(30) << ""
       << " ACPL: " << review.black_stats.acpl << "\n";
    os << "--------------------------------------------------------------------------------\n";
    os << " Move Classification                       White              Black\n";
    os << "--------------------------------------------------------------------------------\n";

    auto print_row = [&](const std::string& symbol, const std::string& name, int w, int b) {
        std::string label = symbol + " " + name;
        os << " " << std::left << std::setw(42) << label
           << std::setw(19) << w
           << b << "\n";
    };

    print_row("!!", "Brilliant Moves", review.white_stats.brilliant_count, review.black_stats.brilliant_count);
    print_row("!", "Great Moves", review.white_stats.great_count, review.black_stats.great_count);
    print_row("[*]", "Best Moves", review.white_stats.best_count, review.black_stats.best_count);
    print_row("[!]", "Excellent Moves", review.white_stats.excellent_count, review.black_stats.excellent_count);
    print_row("[+]", "Good Moves", review.white_stats.good_count, review.black_stats.good_count);
    print_row("[B]", "Book Moves", review.white_stats.book_count, review.black_stats.book_count);
    print_row("[?!]", "Inaccuracies", review.white_stats.inaccuracy_count, review.black_stats.inaccuracy_count);
    print_row("[?]", "Mistakes", review.white_stats.mistake_count, review.black_stats.mistake_count);
    print_row("[??]", "Blunders", review.white_stats.blunder_count, review.black_stats.blunder_count);
    print_row("[X]", "Missed Wins", review.white_stats.miss_count, review.black_stats.miss_count);
    print_row("[]", "Forced Moves", review.white_stats.forced_count, review.black_stats.forced_count);

    os << "================================================================================\n";

    // Print critical mistakes / blunders / misses
    bool has_critical = false;
    for (size_t i = 0; i < review.plies.size(); ++i) {
        const auto& ap = review.plies[i];
        if (ap.classification == MoveClassification::Blunder ||
            ap.classification == MoveClassification::Mistake ||
            ap.classification == MoveClassification::Miss) {
            if (!has_critical) {
                os << " Key Mistakes & Blunders:\n";
                has_critical = true;
            }
            int move_num = static_cast<int>(i / 2) + 1;
            std::string prefix = (i % 2 == 0) ? (std::to_string(move_num) + ". ")
                                              : (std::to_string(move_num) + "... ");
            std::string best_display = !ap.best_was_san.empty() ? ap.best_was_san : ap.before_eval.best_move;
            os << "   " << std::left << std::setw(12) << (prefix + ap.ply.san)
               << std::setw(16) << ("(" + to_string(ap.classification) + ")")
               << " Loss: -" << std::setw(3) << static_cast<int>(ap.win_percent_loss * 100.0) << "%"
               << " | Best was: " << best_display << "\n";
        }
    }
    if (has_critical) {
        os << "================================================================================\n";
    }
    os << "\n";
}

} // namespace zugfish
