#include "opening_book.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace zugfish {

namespace fs = std::filesystem;

OpeningBook& OpeningBook::instance() {
    static OpeningBook book;
    return book;
}

OpeningBook::OpeningBook()
    : root_(std::make_unique<Node>()) {
    root_->opening = {"A00", "Undetermined Opening"};
    load_openings();
}

void OpeningBook::add_line(const std::string& eco, const std::string& name, const std::vector<std::string>& uci_moves) {
    if (uci_moves.empty()) return;
    Node* current = root_.get();
    for (const auto& uci : uci_moves) {
        auto it = current->children.find(uci);
        if (it == current->children.end()) {
            auto new_node = std::make_unique<Node>();
            new_node->uci = uci;
            new_node->opening = {eco, name};
            Node* raw_ptr = new_node.get();
            current->children[uci] = std::move(new_node);
            current = raw_ptr;
        } else {
            current = it->second.get();
            // Update with more specific variation name
            current->opening = {eco, name};
        }
    }
    eco_count_++;
}

bool OpeningBook::is_book_move(const chess::Board& board, const std::vector<std::string>& uci_history, const std::string& next_uci) const {
    // 1. Check grandmaster tournament Polyglot book
    if (polyglot_.is_loaded()) {
        if (polyglot_.is_book_move(board, next_uci)) {
            return true;
        }
    }

    // 2. Check exhaustive ECO trie
    const Node* current = root_.get();
    for (const auto& uci : uci_history) {
        auto it = current->children.find(uci);
        if (it == current->children.end()) {
            return false;
        }
        current = it->second.get();
    }
    return current->children.find(next_uci) != current->children.end();
}

bool OpeningBook::is_book_move(const std::vector<std::string>& uci_history, const std::string& next_uci) const {
    // Reconstruct board if board was not passed
    chess::Board board;
    for (const auto& u : uci_history) {
        try {
            chess::Move m = chess::uci::uciToMove(board, u);
            board.makeMove(m);
        } catch (...) {
            break;
        }
    }
    return is_book_move(board, uci_history, next_uci);
}

OpeningInfo OpeningBook::identify_opening(const std::vector<std::string>& uci_history) const {
    const Node* current = root_.get();
    OpeningInfo best_match = {"A00", "Custom Opening"};
    for (const auto& uci : uci_history) {
        auto it = current->children.find(uci);
        if (it == current->children.end()) {
            break;
        }
        current = it->second.get();
        if (!current->opening.eco.empty() && current->opening.eco != "A00") {
            best_match = current->opening;
        } else if (best_match.eco == "A00" && !current->opening.name.empty()) {
            best_match = current->opening;
        }
    }
    return best_match;
}

static std::string find_file(const std::string& filename) {
    const std::vector<std::string> candidates = {
        "data/" + filename,
        "../data/" + filename,
        "../../data/" + filename,
        "/home/karmveer/.coding/zugfish/data/" + filename,
        "/usr/local/share/zugfish/" + filename,
        "/usr/share/zugfish/" + filename
    };
    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return path;
        }
    }
    return "";
}

void OpeningBook::load_openings() {
    // 1. Try to load Polyglot master opening book
    std::string bin_path = find_file("gm2001.bin");
    if (!bin_path.empty()) {
        polyglot_.load(bin_path);
    }

    // 2. Try to load exhaustive ECO database (3,865 lines)
    std::string tsv_path = find_file("openings_uci.tsv");
    bool tsv_loaded = false;

    if (!tsv_path.empty()) {
        std::ifstream file(tsv_path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                std::istringstream iss(line);
                std::string eco, name, uci_str;
                if (!std::getline(iss, eco, '\t')) continue;
                if (!std::getline(iss, name, '\t')) continue;
                if (!std::getline(iss, uci_str, '\t')) continue;

                std::vector<std::string> ucis;
                std::istringstream uci_iss(uci_str);
                std::string u;
                while (uci_iss >> u) {
                    ucis.push_back(u);
                }

                if (!ucis.empty()) {
                    add_line(eco, name, ucis);
                }
            }
            tsv_loaded = (eco_count_ > 0);
        }
    }

    // 3. If external database could not be loaded, use embedded fallback
    if (!tsv_loaded) {
        load_embedded_fallback();
    }
}

void OpeningBook::load_embedded_fallback() {
    add_line("C20", "King's Pawn Game: Open Game", {"e2e4", "e7e5"});
    add_line("C40", "King's Knight Opening", {"e2e4", "e7e5", "g1f3"});
    add_line("C42", "Petrov's Defense", {"e2e4", "e7e5", "g1f3", "g8f6"});
    add_line("C41", "Philidor Defense", {"e2e4", "e7e5", "g1f3", "d7d6"});
    add_line("C41", "Philidor Defense: Morphy Variation", {"e2e4", "e7e5", "g1f3", "d7d6", "d2d4", "c8g4"});
    add_line("C44", "Scotch Game", {"e2e4", "e7e5", "g1f3", "b8c6", "d2d4"});
    add_line("C50", "Italian Game", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4"});
    add_line("C53", "Italian Game: Giuoco Piano", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "c2c3", "g8f6", "d2d4", "e5d4", "c3d4", "c5b4"});
    add_line("C54", "Italian Game: Giuoco Pianissimo", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "c2c3", "g8f6", "d2d3", "d7d6"});
    add_line("C51", "Evans Gambit", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "b2b4"});
    add_line("C55", "Two Knights Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6"});
    add_line("C57", "Two Knights Defense: Fried Liver Attack", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "f3g5", "d7d5", "e4d5", "f6d5", "g5f7"});
    add_line("C60", "Ruy Lopez", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5"});
    add_line("C65", "Ruy Lopez: Berlin Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "g8f6"});
    add_line("C70", "Ruy Lopez: Morphy Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4"});
    add_line("C78", "Ruy Lopez: Closed", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f8e7"});
    add_line("C88", "Ruy Lopez: Marshall Attack", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f8e7", "f1e1", "b7b5", "a4b3", "e8g8", "c2c3", "d7d5"});
    add_line("C30", "King's Gambit", {"e2e4", "e7e5", "f2f4"});
    add_line("B20", "Sicilian Defense", {"e2e4", "c7c5"});
    add_line("B22", "Sicilian Defense: Alapin Variation", {"e2e4", "c7c5", "c2c3"});
    add_line("B23", "Sicilian Defense: Closed", {"e2e4", "c7c5", "b1c3"});
    add_line("B27", "Sicilian Defense: Open", {"e2e4", "c7c5", "g1f3"});
    add_line("B33", "Sicilian Defense: Sveshnikov Variation", {"e2e4", "c7c5", "g1f3", "b8c6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "e7e5"});
    add_line("B70", "Sicilian Defense: Dragon Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "g7g6"});
    add_line("B90", "Sicilian Defense: Najdorf Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "a7a6"});
    add_line("C00", "French Defense", {"e2e4", "e7e6"});
    add_line("C02", "French Defense: Advance Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "e4e5"});
    add_line("C10", "French Defense: Classical", {"e2e4", "e7e6", "d2d4", "d7d5", "b1c3", "g8f6"});
    add_line("B10", "Caro-Kann Defense", {"e2e4", "c7c6"});
    add_line("B12", "Caro-Kann Defense: Advance Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "e4e5"});
    add_line("B15", "Caro-Kann Defense: Classical Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "b1c3", "d5e4", "c3e4", "c8f5"});
    add_line("B01", "Scandinavian Defense", {"e2e4", "d7d5"});
    add_line("D00", "Queen's Pawn Game: Closed", {"d2d4", "d7d5"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3", "f8d6"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3", "f8d6", "f4g3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3", "f8d6", "f4g3", "d6g3", "h2g3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3", "f8d6", "f4g3", "d6g3", "h2g3", "e8g8"});
    add_line("D06", "Queen's Gambit", {"d2d4", "d7d5", "c2c4"});
    add_line("D20", "Queen's Gambit Accepted", {"d2d4", "d7d5", "c2c4", "d5c4"});
    add_line("D30", "Queen's Gambit Declined", {"d2d4", "d7d5", "c2c4", "e7e6"});
    add_line("D10", "Slav Defense", {"d2d4", "d7d5", "c2c4", "c7c6"});
    add_line("D43", "Semi-Slav Defense", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "b1c3", "e7e6"});
    add_line("E60", "King's Indian Defense", {"d2d4", "g8f6", "c2c4", "g7g6"});
    add_line("D70", "Grünfeld Defense", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "d7d5"});
    add_line("E20", "Nimzo-Indian Defense", {"d2d4", "g8f6", "c2c4", "e7e6", "b1c3", "f8b4"});
    add_line("A10", "English Opening", {"c2c4"});
    add_line("A04", "Réti Opening", {"g1f3"});
}

} // namespace zugfish
