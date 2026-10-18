#include "online_importer.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace zugfish {

static size_t curl_write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total);
    return total;
}

static bool http_get(const std::string& url,
                     std::string& response_body,
                     std::string& err_out,
                     const std::string& accept_header = "") {
    CURL* curl = curl_easy_init();
    if (!curl) {
        err_out = "Failed to initialize CURL";
        return false;
    }

    response_body.clear();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "zugfish-chess-reviewer/1.0 (Linux; contact: github.com)");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = nullptr;
    if (!accept_header.empty()) {
        std::string h = "Accept: " + accept_header;
        headers = curl_slist_append(headers, h.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (headers) {
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        err_out = curl_easy_strerror(res);
        return false;
    }
    if (http_code != 200) {
        if (http_code == 404) {
            err_out = "Player not found (404)";
        } else if (http_code == 429) {
            err_out = "Rate limit reached. Please wait a moment.";
        } else {
            err_out = "Server returned HTTP status " + std::to_string(http_code);
        }
        return false;
    }

    return true;
}

static std::string trim_spaces(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string format_unix_timestamp(int64_t timestamp_sec) {
    if (timestamp_sec <= 0) return "";
    std::time_t t = static_cast<std::time_t>(timestamp_sec);
    std::tm tm_buf{};
#if defined(_WIN32)
    gmtime_s(&tm_buf, &t);
#else
    gmtime_r(&t, &tm_buf);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y.%m.%d", &tm_buf);
    return std::string(buf);
}

static std::vector<OnlineGameSummary> fetch_chesscom(const std::string& username, int max_games, std::string* error_msg) {
    std::vector<OnlineGameSummary> result;
    std::string err;

    std::string clean_user = trim_spaces(username);
    std::string archives_url = "https://api.chess.com/pub/player/" + clean_user + "/games/archives";
    std::string archives_resp;

    if (!http_get(archives_url, archives_resp, err)) {
        if (error_msg) *error_msg = "Chess.com: " + err;
        return result;
    }

    nlohmann::json archives_json;
    try {
        archives_json = nlohmann::json::parse(archives_resp);
    } catch (...) {
        if (error_msg) *error_msg = "Chess.com: Failed to parse archives JSON";
        return result;
    }

    if (!archives_json.contains("archives") || !archives_json["archives"].is_array() || archives_json["archives"].empty()) {
        if (error_msg) *error_msg = "Chess.com: No game archives found for user " + clean_user;
        return result;
    }

    const auto& archives = archives_json["archives"];
    // Iterate from newest archive backwards until we have max_games
    for (auto it = archives.rbegin(); it != archives.rend() && static_cast<int>(result.size()) < max_games; ++it) {
        std::string archive_url = it->get<std::string>();
        std::string games_resp;
        std::string archive_err;

        if (!http_get(archive_url, games_resp, archive_err)) {
            continue;
        }

        try {
            auto month_json = nlohmann::json::parse(games_resp);
            if (month_json.contains("games") && month_json["games"].is_array()) {
                const auto& games_arr = month_json["games"];
                // Newest games in the month are at the end of the array
                for (auto g_it = games_arr.rbegin(); g_it != games_arr.rend() && static_cast<int>(result.size()) < max_games; ++g_it) {
                    const auto& g = *g_it;

                    OnlineGameSummary sum;
                    sum.platform = "Chess.com";
                    sum.id = g.value("url", "");
                    sum.pgn = g.value("pgn", "");
                    sum.speed = g.value("time_class", "standard");

                    int64_t end_t = g.value("end_time", 0LL);
                    sum.date = format_unix_timestamp(end_t);

                    if (g.contains("white") && g["white"].is_object()) {
                        sum.white_name = g["white"].value("username", "White");
                        sum.white_rating = g["white"].value("rating", 0);
                    }
                    if (g.contains("black") && g["black"].is_object()) {
                        sum.black_name = g["black"].value("username", "Black");
                        sum.black_rating = g["black"].value("rating", 0);
                    }

                    // Result interpretation
                    std::string w_res = g["white"].value("result", "");
                    std::string b_res = g["black"].value("result", "");
                    if (w_res == "win") {
                        sum.result = "1-0";
                    } else if (b_res == "win") {
                        sum.result = "0-1";
                    } else if (w_res == "agreed" || w_res == "repetition" || w_res == "stalemate" || w_res == "insufficient") {
                        sum.result = "1/2-1/2";
                    } else {
                        sum.result = "*";
                    }

                    if (!sum.pgn.empty()) {
                        result.push_back(std::move(sum));
                    }
                }
            }
        } catch (...) {
            continue;
        }
    }

    if (result.empty() && error_msg && error_msg->empty()) {
        *error_msg = "Chess.com: No games found for " + clean_user;
    }

    return result;
}

static std::vector<OnlineGameSummary> fetch_lichess(const std::string& username, int max_games, std::string* error_msg) {
    std::vector<OnlineGameSummary> result;
    std::string err;

    std::string clean_user = trim_spaces(username);
    std::string url = "https://lichess.org/api/games/user/" + clean_user +
                      "?max=" + std::to_string(max_games) +
                      "&pgnInJson=true&clocks=false&evals=false&opening=true";
    std::string resp;

    if (!http_get(url, resp, err, "application/x-ndjson")) {
        if (error_msg) *error_msg = "Lichess: " + err;
        return result;
    }

    std::istringstream stream(resp);
    std::string line;

    while (std::getline(stream, line) && static_cast<int>(result.size()) < max_games) {
        std::string trimmed_line = trim_spaces(line);
        if (trimmed_line.empty()) continue;

        try {
            auto g = nlohmann::json::parse(trimmed_line);

            OnlineGameSummary sum;
            sum.platform = "Lichess";
            sum.id = g.value("id", "");
            sum.pgn = g.value("pgn", "");
            sum.speed = g.value("speed", "standard");

            int64_t created_at_ms = g.value("createdAt", 0LL);
            sum.date = format_unix_timestamp(created_at_ms / 1000);

            if (g.contains("players") && g["players"].is_object()) {
                const auto& players = g["players"];
                if (players.contains("white") && players["white"].is_object()) {
                    const auto& w = players["white"];
                    if (w.contains("user") && w["user"].is_object()) {
                        sum.white_name = w["user"].value("name", "Anonymous");
                    } else {
                        sum.white_name = "AI";
                    }
                    sum.white_rating = w.value("rating", 0);
                }
                if (players.contains("black") && players["black"].is_object()) {
                    const auto& b = players["black"];
                    if (b.contains("user") && b["user"].is_object()) {
                        sum.black_name = b["user"].value("name", "Anonymous");
                    } else {
                        sum.black_name = "AI";
                    }
                    sum.black_rating = b.value("rating", 0);
                }
            }

            std::string winner = g.value("winner", "");
            std::string status = g.value("status", "");
            if (winner == "white") {
                sum.result = "1-0";
            } else if (winner == "black") {
                sum.result = "0-1";
            } else if (status == "draw" || status == "stalemate") {
                sum.result = "1/2-1/2";
            } else {
                sum.result = "*";
            }

            if (!sum.pgn.empty()) {
                result.push_back(std::move(sum));
            }
        } catch (...) {
            continue;
        }
    }

    if (result.empty() && error_msg && error_msg->empty()) {
        *error_msg = "Lichess: No games found for " + clean_user;
    }

    return result;
}

std::vector<OnlineGameSummary> OnlineImporter::fetch_recent_games(
    OnlinePlatform platform,
    const std::string& username,
    int max_games,
    std::string* error_msg
) {
    if (error_msg) error_msg->clear();

    if (platform == OnlinePlatform::ChessCom) {
        return fetch_chesscom(username, max_games, error_msg);
    } else {
        return fetch_lichess(username, max_games, error_msg);
    }
}

} // namespace zugfish
