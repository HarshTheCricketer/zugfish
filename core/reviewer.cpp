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

struct MoveEvalRecord {
    double accuracy = 100.0;
    double weight = 1.0;
    int phase = 0; // 0 = Opening, 1 = Middlegame, 2 = Endgame
};

static double compute_phase_accuracy(const std::vector<MoveEvalRecord>& records, int target_phase = -1) {
    double sum_w = 0.0;
    double sum_weighted_acc = 0.0;
    double sum_harmonic = 0.0;

    for (const auto& r : records) {
        if (target_phase >= 0 && r.phase != target_phase) {
            continue;
        }
        sum_w += r.weight;
        sum_weighted_acc += r.weight * r.accuracy;
        sum_harmonic += r.weight / std::max(r.accuracy, 12.0);
    }

    if (sum_w <= 0.0) return (target_phase >= 0) ? 0.0 : 100.0;

    double arith = sum_weighted_acc / sum_w;
    double harmonic = sum_w / sum_harmonic;
    double combined = 0.80 * arith + 0.20 * harmonic;
    return std::clamp(combined, 0.0, 100.0);
}

static int parse_elo_header(const std::map<std::string, std::string>& headers, const std::string& key) {
    auto it = headers.find(key);
    if (it != headers.end() && !it->second.empty()) {
        try {
            int elo = std::stoi(it->second);
            if (elo >= 100 && elo <= 3800) return elo;
        } catch (...) {}
    }
    return 0;
}

static int calculate_estimated_rating(double accuracy, double acpl,
                                      const PlayerStats& stats,
                                      int actual_elo) {
    if (actual_elo >= 100 && actual_elo <= 3800) {
        // Model expected accuracy at player's rating level
        double expected_acc = 50.0 + 38.0 / (1.0 + std::exp(-(actual_elo - 1100.0) / 450.0));
        double delta_acc = accuracy - expected_acc;

        // Base rating offset: ~22 rating points per 1% accuracy difference from expectation
        double perf = actual_elo + delta_acc * 22.0;

        // Tactical and error adjustments
        perf += stats.brilliant_count * 50.0;
        perf += stats.great_count * 30.0;
        perf -= stats.blunder_count * 40.0;
        perf -= stats.miss_count * 20.0;
        perf -= stats.mistake_count * 10.0;

        // Clean play bonus: zero blunders in a substantial game
        if (stats.blunder_count == 0 && (stats.best_count + stats.excellent_count >= 10)) {
            perf += 60.0;
        }

        // Single-game performance spread around actual rating
        double min_bound = std::max(100.0, actual_elo - 450.0);
        double max_bound = std::min(3200.0, actual_elo + 550.0);
        perf = std::clamp(perf, min_bound, max_bound);

        int rounded = static_cast<int>(std::round(perf / 50.0)) * 50;
        return std::clamp(rounded, 100, 3200);
    }

    // Historical or unrated games without Elo header (pure engine-based performance scale)
    double base = 0.0;
    if (accuracy >= 95.0) {
        base = 2400.0 + (accuracy - 95.0) * 100.0;
    } else if (accuracy >= 88.0) {
        base = 1900.0 + (accuracy - 88.0) * 70.0;
    } else if (accuracy >= 78.0) {
        base = 1400.0 + (accuracy - 78.0) * 50.0;
    } else if (accuracy >= 65.0) {
        base = 1000.0 + (accuracy - 65.0) * 30.0;
    } else if (accuracy >= 50.0) {
        base = 650.0 + (accuracy - 50.0) * 23.0;
    } else {
        base = std::max(100.0, 200.0 + accuracy * 9.0);
    }

    base += stats.brilliant_count * 60.0;
    base += stats.great_count * 30.0;
    base -= stats.blunder_count * 45.0;
    base -= stats.miss_count * 25.0;
    base -= stats.mistake_count * 15.0;

    if (acpl <= 25.0) base += 50.0;
    else if (acpl >= 100.0) base -= 50.0;

    int rounded = static_cast<int>(std::round(base / 50.0)) * 50;
    return std::clamp(rounded, 100, 3200);
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

    std::vector<MoveEvalRecord> white_records;
    std::vector<MoveEvalRecord> black_records;
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

        // Chess.com Brilliant (!!) Rules:
        // 1. Must be the engine's top move
        // 2. Not a book move, not a forced move
        // 3. Player cannot already have an overwhelming material lead (material_lead <= 2)
        // 4. Must be move 7+ (ply >= 13)
        // 5. The move must remain sound and maintain an advantage (player_win_after >= 0.50, win_loss <= 0.015)
        // 6. Must NOT be an immediate checkmate delivery
        // 7. MUST BE A GENUINE MATERIAL SACRIFICE
        if (is_best_move && !is_book && !is_forced && i >= 13 &&
            material_lead <= 2 && player_win_after >= 0.50 && win_loss <= 0.015) {

            try {
                chess::Move move_played = chess::uci::uciToMove(board_before, ply.uci);
                chess::Piece moved_piece = board_before.at(move_played.from());
                chess::Square to_sq = move_played.to();
                int moved_val = piece_val(moved_piece.type());
                chess::Piece cap_piece = board_before.at(to_sq);
                int cap_val = (cap_piece != chess::Piece::NONE) ? piece_val(cap_piece.type()) : 0;
                int net_sac = moved_val - cap_val;

                chess::Board b_after(next_fen);

                if (b_after.isGameOver().first == chess::GameResultReason::NONE) {
                    bool opp_recaptures = false;
                    if (!next_eval.best_move.empty() && next_eval.best_move != "(none)") {
                        try {
                            chess::Move opp_best = chess::uci::uciToMove(b_after, next_eval.best_move);
                            if (opp_best.to() == to_sq) {
                                opp_recaptures = true;
                            }
                        } catch (...) {}
                    }
                    if (!opp_recaptures && i + 1 < game.plies.size()) {
                        try {
                            chess::Move opp_actual = chess::uci::uciToMove(b_after, game.plies[i + 1].uci);
                            if (opp_actual.to() == to_sq) {
                                opp_recaptures = true;
                            }
                        } catch (...) {}
                    }

                    // Moved piece itself is sacrificed:
                    // Moved piece is worth >= 3, and net material given up is >= 2
                    if (moved_val >= 3 && net_sac >= 2 && b_after.isAttacked(to_sq, opp_c)) {
                        // If we captured something (like a pawn or minor piece in an exchange sac):
                        // it is ONLY a sacrifice if the opponent actually recaptures on to_sq!
                        if (cap_piece != chess::Piece::NONE) {
                            if (opp_recaptures) {
                                is_brilliant = true;
                            }
                        } else {
                            // Moved to an empty square with piece en prise (worth >= 3, sacrificed):
                            // Either the opponent captures it, OR it is left completely hanging and undefended
                            if (opp_recaptures || !b_after.isAttacked(to_sq, player_c)) {
                                is_brilliant = true;
                            }
                        }
                    }

                    // Check deflection / quiet sacrifice:
                    // A different major/minor piece (>= 3) of player's is left hanging and undefended,
                    // and opponent's best move captures that hanging piece!
                    if (!is_brilliant && !opp_recaptures && 
                        !next_eval.best_move.empty() && next_eval.best_move != "(none)") {
                        chess::Move opp_best = chess::uci::uciToMove(b_after, next_eval.best_move);
                        chess::Piece target = b_after.at(opp_best.to());
                        if (target != chess::Piece::NONE && target.color() == player_c && piece_val(target.type()) >= 3) {
                            if (!b_after.isAttacked(opp_best.to(), player_c)) {
                                bool was_hanging_before = !board_before.isAttacked(opp_best.to(), player_c) &&
                                                          board_before.isAttacked(opp_best.to(), opp_c);
                                if (!was_hanging_before) {
                                    is_brilliant = true;
                                }
                            }
                        }
                    }
                }
            } catch (...) {}
        }

        // 4. Check Great Move (!) (Chess.com: "The only good move in the position")
        bool is_great = false;
        if (is_best_move && !is_book && !is_forced && !is_brilliant && 
            i >= 12 && material_lead <= 6) {

            try {
                chess::Move move_played = chess::uci::uciToMove(board_before, ply.uci);
                chess::Square to_sq = move_played.to();
                chess::Piece cap_piece = board_before.at(to_sq);
                chess::Board b_after(next_fen);

                // Exclude moves delivering checkmate (checkmate is Best, not Great)
                bool is_mate_delivery = (b_after.isGameOver().first != chess::GameResultReason::NONE);

                // Exclude recaptures:
                // If opponent just moved to this square or just captured our piece and we capture back
                bool is_recapture = false;
                if (i > 0) {
                    chess::Board prev_board(game.plies[i - 1].fen_before);
                    chess::Move prev_opp_move = chess::uci::uciToMove(prev_board, game.plies[i - 1].uci);
                    if (to_sq == prev_opp_move.to()) {
                        is_recapture = true;
                    }
                }

                // Exclude king running away when in check (standard evasion)
                bool is_check_evasion = board_before.inCheck();

                // Capturing a hanging piece for free (e.g. taking an undefended Queen or pawn) is Best, NOT Great
                bool is_free_capture = (cap_piece != chess::Piece::NONE && !b_after.isAttacked(to_sq, opp_c));

                if (!is_mate_delivery && !is_recapture && !is_free_capture && !is_check_evasion && prev_eval.has_second_best) {
                    double score2_p = prev_eval.second_score.win_probability(white_to_move);
                    double player_win_2 = white_to_move ? score2_p : (1.0 - score2_p);
                    double win_gap = player_win_before - player_win_2;

                    // Centipawn gap between move #1 and move #2
                    int cp2_val = white_to_move ? to_cp(prev_eval.second_score.to_white_perspective(true))
                                                : -to_cp(prev_eval.second_score.to_white_perspective(false));
                    int cp_gap = cp_before - cp2_val;

                    // Great move conditions:
                    // Candidate #2 drops win probability by >= 14% OR drops >= 140 centipawns
                    if (win_gap >= 0.14 || cp_gap >= 140) {
                        // Scenario 1: Only move that saves/holds a tough position
                        if (player_win_before <= 0.55 && player_win_after >= 0.40 && player_win_2 <= 0.25) {
                            is_great = true;
                        }
                        // Scenario 2: Only winning move in a sharp/contested position
                        else if (player_win_before >= 0.45 && player_win_after >= 0.60 && 
                                 (win_gap >= 0.18 || cp_gap >= 180 || player_win_2 <= 0.48)) {
                            is_great = true;
                        }
                    }
                }
            } catch (...) {}
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
        } else if (is_best_move || win_loss <= 0.008) {
            classification = MoveClassification::Best;
        } else if (win_loss <= 0.028) {
            classification = MoveClassification::Excellent;
        } else if (win_loss <= 0.065) {
            classification = MoveClassification::Good;
        } else if (win_loss <= 0.140) {
            classification = MoveClassification::Inaccuracy;
        } else if (win_loss <= 0.240) {
            classification = MoveClassification::Mistake;
        } else {
            // Missed Win vs Blunder:
            // On Chess.com, if player was winning (win% >= 0.75) and dropped below 0.60,
            // it is classified as a Miss (Missed Win).
            if (player_win_before >= 0.75 && player_win_after <= 0.60) {
                classification = MoveClassification::Miss;
            } else {
                classification = MoveClassification::Blunder;
            }
        }

        // Chess.com Decisive Advantage Rule:
        // When a player is strongly winning (>= 0.75 win prob) both before and after (>= 0.65),
        // an alternative winning line or mate is NEVER marked as Mistake, Blunder, or Miss.
        if ((classification == MoveClassification::Mistake || 
             classification == MoveClassification::Blunder || 
             classification == MoveClassification::Miss) &&
            player_win_before >= 0.75 && player_win_after >= 0.65) {
            classification = MoveClassification::Good;
        }

        // Accuracy score for this single move [0.0, 100.0] via calibrated CAPS2 scoring:
        // Consistent with move classification so blunders score low and good moves score high
        double delta_win_pct = std::max(0.0, win_loss * 100.0);
        double move_accuracy = 100.0;
        if (classification == MoveClassification::Book || 
            classification == MoveClassification::Forced ||
            classification == MoveClassification::Brilliant || 
            classification == MoveClassification::Great || 
            classification == MoveClassification::Best) {
            move_accuracy = 100.0;
        } else if (classification == MoveClassification::Excellent) {
            double raw = 100.0 - (delta_win_pct * 2.0);
            move_accuracy = std::clamp(raw, 90.0, 98.0);
        } else if (classification == MoveClassification::Good) {
            double raw = 88.0 - ((delta_win_pct - 2.5) * 2.2);
            move_accuracy = std::clamp(raw, 72.0, 88.0);
        } else if (classification == MoveClassification::Inaccuracy) {
            double raw = 68.0 - ((delta_win_pct - 6.0) * 1.6);
            move_accuracy = std::clamp(raw, 45.0, 68.0);
        } else if (classification == MoveClassification::Mistake) {
            double raw = 40.0 - ((delta_win_pct - 13.5) * 1.2);
            move_accuracy = std::clamp(raw, 18.0, 40.0);
        } else if (classification == MoveClassification::Miss) {
            double raw = 18.0 - ((delta_win_pct - 23.0) * 0.5);
            move_accuracy = std::clamp(raw, 5.0, 18.0);
        } else { // Blunder
            double raw = 10.0 * std::exp(-0.06 * (delta_win_pct - 23.0));
            move_accuracy = std::clamp(raw, 0.0, 10.0);
        }

        // Determine phase of the game
        int move_number = static_cast<int>(i / 2) + 1;
        int non_pawn_mat = 0;
        int queens = 0;
        for (int sq = 0; sq < 64; ++sq) {
            chess::Piece p = board_before.at(chess::Square(sq));
            if (p != chess::Piece::NONE) {
                if (p.type() == chess::PieceType::QUEEN) queens++;
                if (p.type() != chess::PieceType::PAWN && p.type() != chess::PieceType::KING) {
                    non_pawn_mat += piece_val(p.type());
                }
            }
        }

        int current_phase = 1; // Middlegame
        if (move_number <= 10 && still_in_book) {
            current_phase = 0; // Opening
        } else if (move_number <= 12 && non_pawn_mat >= 38) {
            current_phase = 0; // Opening
        } else if (non_pawn_mat <= 20 || (queens == 0 && non_pawn_mat <= 26)) {
            current_phase = 2; // Endgame
        } else {
            current_phase = 1; // Middlegame
        }

        double move_weight = 1.0;
        if (is_book || is_forced) {
            move_weight = 0.15;
        } else {
            bool is_decided = (player_win_before >= 0.94 && material_lead >= 6) ||
                              (player_win_before <= 0.06 && material_lead <= -6);
            if (is_decided && classification != MoveClassification::Blunder && 
                classification != MoveClassification::Miss) {
                move_weight = 0.25;
            } else if (classification == MoveClassification::Blunder) {
                move_weight = 1.30;
            } else if (classification == MoveClassification::Miss) {
                move_weight = 1.20;
            } else if (classification == MoveClassification::Mistake) {
                move_weight = 1.10;
            } else {
                move_weight = 1.00;
            }
        }

        MoveEvalRecord record{move_accuracy, move_weight, current_phase};
        if (white_to_move) {
            white_records.push_back(record);
            white_cpls.push_back(cpl);
            update_stats(review.white_stats, classification);
        } else {
            black_records.push_back(record);
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
            prog.current_accuracy_white = compute_phase_accuracy(white_records, -1);
            prog.current_accuracy_black = compute_phase_accuracy(black_records, -1);

            progress_cb(prog);
        }

        prev_eval = std::move(next_eval);
    }

    auto avg_int = [](const std::vector<int>& v) {
        if (v.empty()) return 0.0;
        return static_cast<double>(std::accumulate(v.begin(), v.end(), 0)) / v.size();
    };

    review.white_stats.accuracy = compute_phase_accuracy(white_records, -1);
    review.white_stats.opening_accuracy = compute_phase_accuracy(white_records, 0);
    review.white_stats.middlegame_accuracy = compute_phase_accuracy(white_records, 1);
    review.white_stats.endgame_accuracy = compute_phase_accuracy(white_records, 2);
    review.white_stats.acpl = avg_int(white_cpls);

    review.black_stats.accuracy = compute_phase_accuracy(black_records, -1);
    review.black_stats.opening_accuracy = compute_phase_accuracy(black_records, 0);
    review.black_stats.middlegame_accuracy = compute_phase_accuracy(black_records, 1);
    review.black_stats.endgame_accuracy = compute_phase_accuracy(black_records, 2);
    review.black_stats.acpl = avg_int(black_cpls);

    int white_actual_elo = parse_elo_header(review.headers, "WhiteElo");
    int black_actual_elo = parse_elo_header(review.headers, "BlackElo");

    review.white_stats.estimated_rating = calculate_estimated_rating(
        review.white_stats.accuracy, review.white_stats.acpl, review.white_stats, white_actual_elo);
    review.black_stats.estimated_rating = calculate_estimated_rating(
        review.black_stats.accuracy, review.black_stats.acpl, review.black_stats, black_actual_elo);

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
    os << " Played Like: ~" << std::left << std::setw(25) << review.white_stats.estimated_rating
       << " Played Like: ~" << review.black_stats.estimated_rating << "\n";
    os << " ACPL: " << std::setprecision(1) << std::setw(32) << review.white_stats.acpl
       << " ACPL: " << review.black_stats.acpl << "\n";
    os << "--------------------------------------------------------------------------------\n";
    os << " Phase Accuracy                            White              Black\n";
    os << "--------------------------------------------------------------------------------\n";

    auto print_phase = [&](const std::string& name, double w, double b) {
        std::ostringstream w_oss, b_oss;
        if (w > 0.0) w_oss << std::fixed << std::setprecision(1) << w << "%";
        else w_oss << "-";
        if (b > 0.0) b_oss << std::fixed << std::setprecision(1) << b << "%";
        else b_oss << "-";

        os << " " << std::left << std::setw(42) << name
           << std::setw(19) << w_oss.str()
           << b_oss.str() << "\n";
    };
    print_phase("Opening", review.white_stats.opening_accuracy, review.black_stats.opening_accuracy);
    print_phase("Middlegame", review.white_stats.middlegame_accuracy, review.black_stats.middlegame_accuracy);
    if (review.white_stats.endgame_accuracy > 0.0 || review.black_stats.endgame_accuracy > 0.0) {
        print_phase("Endgame", review.white_stats.endgame_accuracy, review.black_stats.endgame_accuracy);
    }
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
