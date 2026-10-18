#include "pgn_parser.hpp"
#include <chess.hpp>
#include <fstream>
#include <sstream>

namespace zugfish {

class GameBuilder : public chess::pgn::Visitor {
public:
    Game game;
    bool has_error = false;
    std::string error_message;
    bool finished = false;

    void startPgn() override {
        // If we already finished one game, skip any subsequent games
        if (finished) {
            skipPgn(true);
            return;
        }
        game = Game{};
        board_ = chess::Board();
        has_error = false;
        error_message.clear();
    }

    void header(std::string_view k, std::string_view v) override {
        if (finished || has_error) return;
        game.headers[std::string(k)] = std::string(v);
    }

    void startMoves() override {
        if (finished || has_error) return;
        // Support custom starting position if FEN header exists
        if (auto it = game.headers.find("FEN"); it != game.headers.end()) {
            board_.setFen(it->second);
        }
    }

    void move(std::string_view san, std::string_view /*comment*/) override {
        if (finished || has_error) return;
        if (san.empty()) return;

        try {
            chess::Move m = chess::uci::parseSan(board_, san);
            if (m == chess::Move::NO_MOVE) {
                has_error = true;
                error_message = "Invalid or unparseable move: '" + std::string(san) +
                                "' in position: " + board_.getFen();
                skipPgn(true);
                return;
            }

            game.plies.push_back({
                std::string(san),
                chess::uci::moveToUci(m),
                board_.getFen()
            });

            board_.makeMove(m);
        } catch (const std::exception& e) {
            has_error = true;
            error_message = "Illegal move '" + std::string(san) +
                            "' in position: " + board_.getFen() + " (" + e.what() + ")";
            skipPgn(true);
        }
    }

    void endPgn() override {
        if (!has_error && !finished) {
            finished = true;
        }
    }

private:
    chess::Board board_;
};

ParseResult parse_pgn(std::istream& stream) {
    chess::pgn::StreamParser parser(stream);
    GameBuilder builder;

    auto err = parser.readGames(builder);

    ParseResult result;
    if (builder.has_error) {
        result.success = false;
        result.error = builder.error_message;
    } else if (err.hasError()) {
        result.success = false;
        result.error = "PGN syntax error: " + err.message();
    } else if (!builder.finished) {
        result.success = false;
        result.error = "No game found in PGN";
    } else {
        result.success = true;
        result.game = std::move(builder.game);
    }

    return result;
}

ParseResult parse_pgn(std::string_view pgn_text) {
    std::istringstream stream((std::string(pgn_text)));
    return parse_pgn(stream);
}

ParseResult parse_pgn_file(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        ParseResult result;
        result.success = false;
        result.error = "Failed to open file: " + file_path.string();
        return result;
    }
    return parse_pgn(file);
}

} // namespace zugfish
