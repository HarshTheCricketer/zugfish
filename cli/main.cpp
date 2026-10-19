#include "zugfish.hpp"
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
    std::string pgn_path = "";
    std::string chesscom_user = "";
    std::string lichess_user = "";
    int depth = 12;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Zugfish CLI - Chess Game Reviewer & Stockfish Analysis\n"
                      << "Usage: zugfish-cli [options] [game.pgn] [depth]\n\n"
                      << "Options:\n"
                      << "  -h, --help                 Show this help message\n"
                      << "  --depth <N>                Engine analysis depth (default: 12)\n"
                      << "  --chesscom <username>      Import recent games from Chess.com API\n"
                      << "  --lichess <username>       Import recent games from Lichess.org API\n\n"
                      << "Examples:\n"
                      << "  zugfish-cli game.pgn\n"
                      << "  zugfish-cli game.pgn 14\n"
                      << "  zugfish-cli --chesscom hikaru --depth 12\n"
                      << "  zugfish-cli --lichess magnuscarlsen\n";
            return 0;
        } else if (arg == "--chesscom" && i + 1 < argc) {
            chesscom_user = argv[++i];
        } else if (arg == "--lichess" && i + 1 < argc) {
            lichess_user = argv[++i];
        } else if (arg == "--depth" && i + 1 < argc) {
            depth = std::stoi(argv[++i]);
        } else if (pgn_path.empty() && arg[0] != '-') {
            pgn_path = arg;
        } else if (!pgn_path.empty() && arg[0] != '-' && std::isdigit(arg[0])) {
            depth = std::stoi(arg);
        }
    }

    zugfish::Game game;
    bool loaded = false;

    if (!chesscom_user.empty()) {
        std::cout << "Connecting to Chess.com API for player: " << chesscom_user << "...\n";
        std::string err;
        auto games = zugfish::OnlineImporter::fetch_recent_games(zugfish::OnlinePlatform::ChessCom, chesscom_user, 5, &err);
        if (games.empty()) {
            std::cerr << "Error fetching from Chess.com: " << err << "\n";
            return 1;
        }
        std::cout << "Fetched " << games.size() << " recent games from Chess.com:\n";
        for (size_t g_idx = 0; g_idx < games.size(); ++g_idx) {
            const auto& g = games[g_idx];
            std::cout << " [" << (g_idx + 1) << "] " << g.date << " (" << g.speed << "): " 
                      << g.white_name << " (" << g.white_rating << ") vs " 
                      << g.black_name << " (" << g.black_rating << ") [" << g.result << "]\n";
        }
        std::cout << "Loading most recent game [1]...\n";
        auto res = zugfish::parse_pgn(games[0].pgn);
        if (res.success) {
            game = std::move(res.game);
            loaded = true;
        }
    } else if (!lichess_user.empty()) {
        std::cout << "Connecting to Lichess.org API for player: " << lichess_user << "...\n";
        std::string err;
        auto games = zugfish::OnlineImporter::fetch_recent_games(zugfish::OnlinePlatform::Lichess, lichess_user, 5, &err);
        if (games.empty()) {
            std::cerr << "Error fetching from Lichess: " << err << "\n";
            return 1;
        }
        std::cout << "Fetched " << games.size() << " recent games from Lichess:\n";
        for (size_t g_idx = 0; g_idx < games.size(); ++g_idx) {
            const auto& g = games[g_idx];
            std::cout << " [" << (g_idx + 1) << "] " << g.date << " (" << g.speed << "): " 
                      << g.white_name << " (" << g.white_rating << ") vs " 
                      << g.black_name << " (" << g.black_rating << ") [" << g.result << "]\n";
        }
        std::cout << "Loading most recent game [1]...\n";
        auto res = zugfish::parse_pgn(games[0].pgn);
        if (res.success) {
            game = std::move(res.game);
            loaded = true;
        }
    } else if (!pgn_path.empty()) {
        auto res = zugfish::parse_pgn_file(pgn_path);
        if (res.success) {
            game = std::move(res.game);
            loaded = true;
        } else {
            std::cerr << "Notice: Could not load " << pgn_path << " (" << res.error << "). Falling back to Opera Game.\n";
        }
    }

    if (!loaded) {
        std::string fallback = R"(
[Event "Classic Opera Game"]
[White "Paul Morphy"]
[Black "Duke of Brunswick"]
[Result "1-0"]

1. e4 e5 2. Nf3 d6 3. d4 Bg4 4. dxe5 Bxf3 5. Qxf3 dxe5 6. Bc4 Nf6 7. Qb3 Qe7 8. Nc3 c6 9. Bg5 b5 10. Nxb5 cxb5 11. Bxb5+ Nbd7 12. O-O-O Rd8 13. Rxd7 Rxd7 14. Rd1 Qe6 15. Bxd7+ Nxd7 16. Qb8+ Nxb8 17. Rd8# 1-0
)";
        auto res_fallback = zugfish::parse_pgn(fallback);
        if (res_fallback.success) {
            game = std::move(res_fallback.game);
            loaded = true;
        } else {
            std::cerr << "Fatal: Failed to load fallback game.\n";
            return 1;
        }
    }

    std::cout << "Successfully parsed game: " 
              << game.headers["White"] << " vs. " << game.headers["Black"] 
              << " (" << game.plies.size() << " plies)\n\n";

    std::cout << "Starting Game Review at Depth " << depth << "...\n";
    zugfish::GameReviewer reviewer("/usr/sbin/stockfish");

    // Decoupled progress callback (works with CLI, TUI, or GUI)
    auto on_progress = [](const zugfish::ReviewProgress& prog) {
        int pct = static_cast<int>((prog.current_ply * 100) / prog.total_plies);
        std::ostringstream ss;
        ss << "[" << std::setw(3) << pct << "%] Ply " 
           << std::setw(3) << prog.current_ply << "/" << prog.total_plies
           << ": " << std::left << std::setw(8) << prog.move_san
           << " " << std::setw(18) << ("(" + zugfish::to_symbol(prog.last_classification) + " " + zugfish::to_string(prog.last_classification) + ")")
           << " | W: " << std::fixed << std::setprecision(1) << std::setw(5) << prog.current_accuracy_white << "%"
           << " | B: " << std::setw(5) << prog.current_accuracy_black << "%";
        std::cout << "\r" << std::left << std::setw(79) << ss.str() << std::flush;
    };

    auto review = reviewer.review(game, depth, on_progress);
    std::cout << "\n";

    // Print the formatted game review
    zugfish::GameReviewer::print_review(review);

    return 0;
}