// A *stub* of the zugfish core API, reconstructed from how the old ImGui front end used it.
// Used only to compile-check src/backend/ZugfishBackend.cpp without the real core:
//   g++ -std=c++20 -fsyntax-only -DZUGFISH_WITH_CORE -Itools/zugfish_stub -I<qt includes> ...
#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace zugfish {
struct Ply { std::string san, uci, fen_before; };
struct Game { std::map<std::string, std::string> headers; std::vector<Ply> plies; };
struct ParseResult { bool success = false; Game game; std::string error; };
inline ParseResult parse_pgn(const std::string &) { return {}; }

enum class MoveClassification { Brilliant, Great, Best, Excellent, Good, Book, Inaccuracy, Mistake, Blunder, Miss, Forced };
struct Score {
    int value = 0; bool is_mate = false;
    Score to_white_perspective(bool) const { return *this; }
    double win_probability(bool) const { return 0.5; }
};
struct Eval { Score score; std::string best_move; };
struct AnalyzedPly {
    Ply ply; Eval before_eval, after_eval; Score score_white_perspective;
    double win_percent_white = 50, win_percent_loss = 0; int centipawn_loss = 0;
    MoveClassification classification{}; std::string best_was_san, next_best_san;
};
struct GameReview { std::vector<AnalyzedPly> plies; Eval initial_eval; std::string initial_best_san; };
struct ReviewProgress { size_t current_ply = 0; double current_accuracy_white = 0, current_accuracy_black = 0; };
class GameReviewer {
public:
    explicit GameReviewer(const std::string &) {}
    GameReview review(const Game &, int, std::function<void(const ReviewProgress &)>, std::function<bool()>) { return {}; }
};
enum class OnlinePlatform { ChessCom, Lichess };
struct OnlineGameSummary { std::string date, white_name, black_name, speed, result, pgn; int white_rating = 0, black_rating = 0; };
struct OnlineImporter {
    static std::vector<OnlineGameSummary> fetch_recent_games(OnlinePlatform, const std::string &, int, std::string *) { return {}; }
};
} // namespace zugfish
