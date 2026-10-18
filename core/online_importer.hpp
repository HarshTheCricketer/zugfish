#pragma once

#include <string>
#include <vector>

namespace zugfish {

enum class OnlinePlatform {
    ChessCom,
    Lichess
};

struct OnlineGameSummary {
    std::string id;
    std::string platform; // "Chess.com" or "Lichess"
    std::string white_name;
    int white_rating = 0;
    std::string black_name;
    int black_rating = 0;
    std::string result; // "1-0", "0-1", "1/2-1/2"
    std::string speed;  // "bullet", "blitz", "rapid", "daily"
    std::string date;   // "YYYY.MM.DD"
    std::string pgn;    // Full PGN text
};

class OnlineImporter {
public:
    // Fetches the most recent games for a given user from Chess.com or Lichess.
    // Returns a list of game summaries ordered with the most recent game first.
    static std::vector<OnlineGameSummary> fetch_recent_games(
        OnlinePlatform platform,
        const std::string& username,
        int max_games = 10,
        std::string* error_msg = nullptr
    );
};

} // namespace zugfish
