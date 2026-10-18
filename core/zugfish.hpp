#pragma once

#include "ply.hpp"
#include "game.hpp"
#include "pgn_parser.hpp"
#include "uci_engine.hpp"
#include "reviewer.hpp"
#include "online_importer.hpp"

#include <iomanip>
#include <iostream>
#include <string>

namespace zugfish {

inline void print_game(const Game& game, std::ostream& os = std::cout) {
    auto get_header = [&](const std::string& key) -> std::string {
        auto it = game.headers.find(key);
        return it != game.headers.end() ? it->second : "?";
    };

    os << "\n======================================================================\n";
    os << " " << get_header("White") << " vs. " << get_header("Black");
    if (game.headers.count("Result")) {
        os << " (" << get_header("Result") << ")";
    }
    os << "\n";

    if (game.headers.count("Event") || game.headers.count("Date") || game.headers.count("Site")) {
        os << " Event: " << get_header("Event")
           << " | Site: " << get_header("Site")
           << " | Date: " << get_header("Date") << "\n";
    }
    os << "======================================================================\n";
    os << " " << std::left
       << std::setw(6)  << "#"
       << std::setw(14) << "White"
       << std::setw(12) << "UCI"
       << std::setw(14) << "Black"
       << std::setw(12) << "UCI" << "\n";
    os << "----------------------------------------------------------------------\n";

    for (size_t i = 0; i < game.plies.size(); i += 2) {
        int move_num = static_cast<int>(i / 2) + 1;
        const auto& white = game.plies[i];

        os << " " << std::left
           << std::setw(6)  << (std::to_string(move_num) + ".")
           << std::setw(14) << white.san
           << std::setw(12) << white.uci;

        if (i + 1 < game.plies.size()) {
            const auto& black = game.plies[i + 1];
            os << std::setw(14) << black.san
               << std::setw(12) << black.uci;
        }
        os << "\n";
    }

    os << "----------------------------------------------------------------------\n";
    os << " Move Text: ";
    for (size_t i = 0; i < game.plies.size(); ++i) {
        if (i % 2 == 0) {
            os << (i / 2 + 1) << ". ";
        }
        os << game.plies[i].san << " ";
    }
    if (game.headers.count("Result")) {
        os << game.headers.at("Result");
    }
    os << "\n";
    os << " Total plies: " << game.plies.size() << "\n";
    os << "======================================================================\n\n";
}

} // namespace zugfish
