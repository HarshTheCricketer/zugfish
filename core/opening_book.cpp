#include "opening_book.hpp"

namespace zugfish {

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
}

bool OpeningBook::is_book_move(const std::vector<std::string>& uci_history, const std::string& next_uci) const {
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

OpeningInfo OpeningBook::identify_opening(const std::vector<std::string>& uci_history) const {
    const Node* current = root_.get();
    OpeningInfo best_match = {"A00", "Custom Opening"};
    for (const auto& uci : uci_history) {
        auto it = current->children.find(uci);
        if (it == current->children.end()) {
            break;
        }
        current = it->second.get();
        if (!current->opening.eco.empty()) {
            best_match = current->opening;
        }
    }
    return best_match;
}

void OpeningBook::load_openings() {
    add_line("C20", "King's Pawn Game: Open Game", {"e2e4", "e7e5"});
    add_line("C40", "King's Knight Opening", {"e2e4", "e7e5", "g1f3"});
    add_line("C42", "Petrov's Defense", {"e2e4", "e7e5", "g1f3", "g8f6"});
    add_line("C42", "Petrov's Defense: Classical Attack", {"e2e4", "e7e5", "g1f3", "g8f6", "f3e5", "d7d6", "e5f3", "f6e4", "d2d4", "d6d5", "f1d3"});
    add_line("C42", "Petrov's Defense: Steinitz Attack", {"e2e4", "e7e5", "g1f3", "g8f6", "d2d4", "e5d4", "e4e5", "f6e4", "d1d4", "d7d5", "e5d6", "e4d6"});
    add_line("C41", "Philidor Defense", {"e2e4", "e7e5", "g1f3", "d7d6"});
    add_line("C41", "Philidor Defense: Exchange Variation", {"e2e4", "e7e5", "g1f3", "d7d6", "d2d4", "e5d4", "f3d4", "g8f6", "b1c3", "f8e7"});
    add_line("C41", "Philidor Defense: Morphy Variation", {"e2e4", "e7e5", "g1f3", "d7d6", "d2d4", "c8g4"});
    add_line("C41", "Philidor Defense: Hanham Variation", {"e2e4", "e7e5", "g1f3", "d7d6", "d2d4", "b8d7", "f1c4", "c7c6", "e1g1", "f8e7"});
    add_line("C44", "Scotch Game", {"e2e4", "e7e5", "g1f3", "b8c6", "d2d4"});
    add_line("C45", "Scotch Game: Classical Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "d2d4", "e5d4", "f3d4", "f8c5", "c1e3", "d8f6", "c2c3", "g8e7"});
    add_line("C45", "Scotch Game: Schmidt Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "d2d4", "e5d4", "f3d4", "g8f6", "b1c3", "f8b4", "d4c6", "b7c6", "f1d3", "d7d5"});
    add_line("C45", "Scotch Game: Mieses Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "d2d4", "e5d4", "f3d4", "g8f6", "d4c6", "b7c6", "e4e5", "d8e7", "d1e2", "f6d5", "c2c4", "c8a6"});
    add_line("C50", "Italian Game", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4"});
    add_line("C50", "Italian Game: Hungarian Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8e7", "d2d4", "d7d6"});
    add_line("C53", "Italian Game: Giuoco Piano", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "c2c3", "g8f6", "d2d4", "e5d4", "c3d4", "c5b4", "c1d2", "b4d2", "b1d2", "d7d5"});
    add_line("C54", "Italian Game: Giuoco Pianissimo", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "c2c3", "g8f6", "d2d3", "d7d6", "e1g1", "e8g8", "b1d2", "a7a6", "a2a4", "c5a7", "f1e1"});
    add_line("C54", "Italian Game: Giuoco Pianissimo Main Line", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "d2d3", "g8f6", "c2c3", "d7d6", "e1g1", "a7a6", "a2a4", "c5a7", "h2h3", "e8g8", "f1e1", "c8e6"});
    add_line("C51", "Evans Gambit", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5", "b2b4", "c5b4", "c2c3", "b4a5", "d2d4", "e5d4", "e1g1"});
    add_line("C55", "Two Knights Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6"});
    add_line("C55", "Two Knights Defense: Modern Line", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "d2d3", "f8e7", "e1g1", "e8g8", "f1e1", "d7d6", "c2c3"});
    add_line("C57", "Two Knights Defense: Fried Liver Attack", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "f3g5", "d7d5", "e4d5", "f6d5", "g5f7", "e8f7", "d1f3", "f7e6", "b1c3", "c6b4"});
    add_line("C58", "Two Knights Defense: Polerio Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "f3g5", "d7d5", "e4d5", "c6a5", "c4b5", "c7c6", "d5c6", "b7c6", "f1e2", "h7h6", "g5f3", "e5e4", "f3e5", "f8d6"});
    add_line("C60", "Ruy Lopez", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5"});
    add_line("C65", "Ruy Lopez: Berlin Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "g8f6"});
    add_line("C67", "Ruy Lopez: Berlin Defense, Open", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "g8f6", "e1g1", "f6e4", "d2d4", "e4d6", "b5c6", "d7c6", "d4e5", "d6f5", "d1d8", "e8d8"});
    add_line("C68", "Ruy Lopez: Exchange Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "b5c6", "d7c6", "e1g1", "f7f6", "d2d4", "e5d4", "f3d4", "c6c5", "d4e2", "d8d1", "f1d1", "c8d7"});
    add_line("C70", "Ruy Lopez: Morphy Defense", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4"});
    add_line("C78", "Ruy Lopez: Closed", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f8e7"});
    add_line("C84", "Ruy Lopez: Closed, Centre Attack", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f8e7", "f1e1", "b7b5", "a4b3", "d7d6", "c2c3", "e8g8", "h2h3", "c6a5", "b3c2", "c7c5", "d2d4"});
    add_line("C88", "Ruy Lopez: Marshall Attack", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f8e7", "f1e1", "b7b5", "a4b3", "e8g8", "c2c3", "d7d5", "e4d5", "f6d5", "f3e5", "c6e5", "e1e5", "c7c6", "d2d4", "e7d6", "e5e1", "d8h4", "g2g3", "h4h3"});
    add_line("C80", "Ruy Lopez: Open Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "f1a4", "g8f6", "e1g1", "f6e4", "d2d4", "b7b5", "a4b3", "d7d5", "d4e5", "c8e6"});
    add_line("C47", "Four Knights Game", {"e2e4", "e7e5", "g1f3", "b8c6", "b1c3", "g8f6"});
    add_line("C48", "Four Knights Game: Spanish Variation", {"e2e4", "e7e5", "g1f3", "b8c6", "b1c3", "g8f6", "f1b5", "f8b4", "e1g1", "e8g8", "d2d3", "d7d6"});
    add_line("C25", "Vienna Game", {"e2e4", "e7e5", "b1c3"});
    add_line("C26", "Vienna Game: Falkbeer Variation", {"e2e4", "e7e5", "b1c3", "g8f6", "f1c4", "f6e4", "d1h5", "e4d6", "c4b3", "b8c6", "c3b5", "g7g6", "h5f3", "f7f5", "f3d5", "d8e7", "b5c7", "e8d8", "c7a8", "b7b6"});
    add_line("C30", "King's Gambit", {"e2e4", "e7e5", "f2f4"});
    add_line("C33", "King's Gambit Accepted", {"e2e4", "e7e5", "f2f4", "e5f4", "g1f3", "g7g5", "h2h4", "g5g4", "f3e5", "g8f6", "d2d4", "d7d6", "e5d3", "f6e4", "c1f4"});
    add_line("C31", "King's Gambit Declined: Falkbeer Countergambit", {"e2e4", "e7e5", "f2f4", "d7d5", "e4d5", "e5f4", "g1f3", "g8f6"});
    add_line("C23", "Bishop's Opening", {"e2e4", "e7e5", "f1c4", "g8f6", "d2d3", "c7c6", "g1f3", "d7d5", "c4b3", "f8b4", "c2c3", "b4d6"});
    add_line("B20", "Sicilian Defense", {"e2e4", "c7c5"});
    add_line("B22", "Sicilian Defense: Alapin Variation", {"e2e4", "c7c5", "c2c3"});
    add_line("B22", "Sicilian Defense: Alapin, 2...d5", {"e2e4", "c7c5", "c2c3", "d7d5", "e4d5", "d8d5", "d2d4", "g8f6", "g1f3", "e7e6", "f1e2", "b8c6", "e1g1", "c5d4", "c3d4", "f8e7", "b1c3", "d5d6"});
    add_line("B22", "Sicilian Defense: Alapin, 2...Nf6", {"e2e4", "c7c5", "c2c3", "g8f6", "e4e5", "f6d5", "d2d4", "c5d4", "g1f3", "b8c6", "c3d4", "d7d6", "f1c4", "e7e6"});
    add_line("B23", "Sicilian Defense: Closed", {"e2e4", "c7c5", "b1c3", "b8c6", "g2g3", "g7g6", "f1g2", "f8g7", "d2d3", "d7d6", "f2f4", "e7e6", "g1f3", "g8e7", "e1g1", "e8g8"});
    add_line("B21", "Sicilian Defense: Smith-Morra Gambit", {"e2e4", "c7c5", "d2d4", "c5d4", "c2c3", "d4c3", "b1c3", "b8c6", "g1f3", "d7d6", "f1c4", "e7e6", "e1g1", "g8f6", "d1e2", "f8e7", "f1d1", "e6e5"});
    add_line("B27", "Sicilian Defense: Open", {"e2e4", "c7c5", "g1f3"});
    add_line("B30", "Sicilian Defense: Old Sicilian", {"e2e4", "c7c5", "g1f3", "b8c6"});
    add_line("B32", "Sicilian Defense: Open, 2...Nc6 3.d4", {"e2e4", "c7c5", "g1f3", "b8c6", "d2d4", "c5d4", "f3d4", "e7e5", "d4b5", "d7d6", "c2c4", "f8e7", "b1c3", "a7a6", "b5a3"});
    add_line("B33", "Sicilian Defense: Sveshnikov Variation", {"e2e4", "c7c5", "g1f3", "b8c6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "e7e5", "d4b5", "d7d6", "c1g5", "a7a6", "b5a3", "b7b5", "c3d5", "f8e7", "g5f6", "e7f6", "c2c3", "e8g8"});
    add_line("B40", "Sicilian Defense: French Variation", {"e2e4", "c7c5", "g1f3", "e7e6"});
    add_line("B41", "Sicilian Defense: Kan Variation", {"e2e4", "c7c5", "g1f3", "e7e6", "d2d4", "c5d4", "f3d4", "a7a6", "b1c3", "d8c7", "f1d3", "g8f6", "e1g1", "f8c5", "d4b3"});
    add_line("B45", "Sicilian Defense: Taimanov Variation", {"e2e4", "c7c5", "g1f3", "e7e6", "d2d4", "c5d4", "f3d4", "b8c6", "b1c3", "d8c7", "c1e3", "a7a6", "d1d2", "g8f6", "e1c1", "f8b4", "f2f3"});
    add_line("B50", "Sicilian Defense: Modern Variations", {"e2e4", "c7c5", "g1f3", "d7d6"});
    add_line("B51", "Sicilian Defense: Moscow Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "f1b5+", "c8d7", "b5d7", "d8d7", "e1g1", "g8f6", "f1e1", "b8c6", "c2c3", "e7e6", "d2d4", "c5d4", "c3d4", "d6d5"});
    add_line("B53", "Sicilian Defense: Chekhover Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "d1d4", "b8c6", "f1b5", "c8d7", "b5c6", "d7c6", "b1c3", "g8f6"});
    add_line("B60", "Sicilian Defense: Classical Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "b8c6", "c1g5", "e7e6", "d1d2", "a7a6", "e1c1", "c8d7", "f2f4", "b7b5"});
    add_line("B70", "Sicilian Defense: Dragon Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "g7g6"});
    add_line("B75", "Sicilian Defense: Dragon, Yugoslav Attack", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "g7g6", "c1e3", "f8g7", "f2f3", "e8g8", "d1d2", "b8c6", "f1c4", "c8d7", "e1c1", "a8c8", "c4b3", "c6e5"});
    add_line("B80", "Sicilian Defense: Scheveningen Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "e7e6", "f1e2", "a7a6", "e1g1", "f8e7", "f2f4", "e8g8", "g1h1", "d8c7", "a2a4", "b8c6", "c1e3"});
    add_line("B90", "Sicilian Defense: Najdorf Variation", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "a7a6"});
    add_line("B90", "Sicilian Defense: Najdorf, English Attack", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "a7a6", "c1e3", "e7e5", "d4b3", "c8e6", "f2f3", "b8d7", "d1d2", "b7b5", "e1c1", "f8e7", "g2g4"});
    add_line("B95", "Sicilian Defense: Najdorf, 6.Bg5", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "a7a6", "c1g5", "e7e6", "f2f4", "d8b6", "d1d2", "b6b2", "a1b1", "b2a3"});
    add_line("B92", "Sicilian Defense: Najdorf, 6.Be2", {"e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "a7a6", "f1e2", "e7e5", "d4b3", "f8e7", "e1g1", "e8g8", "c1e3", "c8e6", "d1d2", "b8d7"});
    add_line("C00", "French Defense", {"e2e4", "e7e6"});
    add_line("C01", "French Defense: Exchange Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "e4d5", "e6d5", "g1f3", "g8f6", "f1d3", "f8d6", "e1g1", "e8g8", "c1g5", "c8g4", "b1d2", "b8d7", "c2c3", "c7c6", "d1c2", "d8c7"});
    add_line("C02", "French Defense: Advance Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "e4e5", "c7c5", "c2c3", "b8c6", "g1f3", "d8b6", "a2a3", "c5c4", "b1d2", "c6a5", "g2g3", "c8d7", "f1h3"});
    add_line("C03", "French Defense: Tarrasch Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "b1d2", "c7c5", "e4d5", "e6d5", "g1f3", "g8f6", "f1b5", "c8d7", "b5d7", "b8d7", "e1g1", "f8e7", "d4c5", "d7c5", "d2b3", "c5e4"});
    add_line("C07", "French Defense: Tarrasch, Open", {"e2e4", "e7e6", "d2d4", "d7d5", "b1d2", "c7c5", "g1f3", "g8f6", "e4e5", "f6d7", "c2c3", "b8c6", "f1d3", "d8b6", "e1g1", "c5d4", "c3d4", "c6d4", "f3d4", "b6d4", "d2f3"});
    add_line("C10", "French Defense: Classical", {"e2e4", "e7e6", "d2d4", "d7d5", "b1c3", "g8f6", "c1g5", "f8e7", "e4e5", "f6d7", "g5e7", "d8e7", "f2f4", "e8g8", "g1f3", "c7c5", "d1d2", "b8c6", "e1c1"});
    add_line("C11", "French Defense: Steinitz Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "b1c3", "g8f6", "e4e5", "f6d7", "f2f4", "c7c5", "g1f3", "b8c6", "c1e3", "c5d4", "f3d4", "f8c5", "d1d2", "e8g8", "e1c1"});
    add_line("C15", "French Defense: Winawer Variation", {"e2e4", "e7e6", "d2d4", "d7d5", "b1c3", "f8b4", "e4e5", "c7c5", "a2a3", "b4c3", "b2c3", "g8e7", "d1g4", "e8g8", "g1f3", "b8c6", "f1d3", "f7f5", "e5f6", "f8f6"});
    add_line("B10", "Caro-Kann Defense", {"e2e4", "c7c6"});
    add_line("B12", "Caro-Kann Defense: Advance Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "e4e5", "c8f5", "g1f3", "e7e6", "f1e2", "c6c5", "c1e3", "d8b6", "b1c3", "b8c6", "e1g1", "b6b2", "d1e1", "c5d4", "f3d4", "c6d4", "e3d4"});
    add_line("B12", "Caro-Kann Defense: Advance, Short Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "e4e5", "c8f5", "g1f3", "e7e6", "f1e2", "b8d7", "e1g1", "h7h6", "b1d2", "g8e7", "d2b3", "g7g5"});
    add_line("B13", "Caro-Kann Defense: Exchange Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "e4d5", "c6d5", "f1d3", "b8c6", "c2c3", "g8f6", "c1f4", "c8g4", "d1b3", "d8d7", "b1d2", "e7e6", "g1f3", "f8d6", "f4d6", "d7d6", "e1g1", "e8g8"});
    add_line("B14", "Caro-Kann Defense: Panov Attack", {"e2e4", "c7c6", "d2d4", "d7d5", "e4d5", "c6d5", "c2c4", "g8f6", "b1c3", "e7e6", "g1f3", "f8b4", "c4d5", "f6d5", "c1d2", "b8c6", "f1d3", "e8g8", "e1g1"});
    add_line("B15", "Caro-Kann Defense: Classical Variation", {"e2e4", "c7c6", "d2d4", "d7d5", "b1c3", "d5e4", "c3e4", "c8f5", "e4g3", "f5g6", "h2h4", "h7h6", "g1f3", "b8d7", "h4h5", "g6h7", "f1d3", "h7d3", "d1d3", "e7e6", "c1d2", "g8f6", "e1c1", "f8e7"});
    add_line("B01", "Scandinavian Defense", {"e2e4", "d7d5"});
    add_line("B01", "Scandinavian Defense: Main Line", {"e2e4", "d7d5", "e4d5", "d8d5", "b1c3", "d5a5", "d2d4", "g8f6", "g1f3", "c7c6", "f1c4", "c8f5", "c1d2", "e7e6", "c3d5", "a5d8", "d5f6", "g7f6"});
    add_line("B01", "Scandinavian Defense: Gubinsky-Melts", {"e2e4", "d7d5", "e4d5", "d8d5", "b1c3", "d5d6", "d2d4", "g8f6", "g1f3", "c7c6", "g2g3", "c8f5", "f1g2", "e7e6", "e1g1", "f8e7"});
    add_line("B02", "Alekhine's Defense", {"e2e4", "g8f6", "e4e5", "f6d5", "d2d4", "d7d6", "g1f3", "g7g6", "f1c4", "d5b6", "c4b3", "f8g7", "e1g1", "e8g8"});
    add_line("B07", "Pirc Defense", {"e2e4", "d7d6", "d2d4", "g8f6", "b1c3", "g7g6", "g1f3", "f8g7", "f1e2", "e8g8", "e1g1", "c7c6", "f1e1"});
    add_line("B06", "Modern Defense", {"e2e4", "g7g6", "d2d4", "f8g7", "c2c3", "d7d6", "f2f4", "g8f6", "f1d3", "e8g8", "g1f3"});
    add_line("A40", "Queen's Pawn Game", {"d2d4"});
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
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "g8f6", "e2e3", "e7e6", "g1f3", "f8d6", "f4g3", "e8g8", "b1d2", "c7c5", "c2c3", "b8c6", "f1d3", "b7b6", "d1e2", "c8b7", "e3e4", "d6e7", "e4e5", "f6h5"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "c7c5", "e2e3", "b8c6", "c2c3", "g8f6", "b1d2", "e7e6", "g1f3", "f8d6", "f4g3", "e8g8", "f1d3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "c1f4", "c7c5", "e2e3", "g8f6", "g1f3", "b8c6", "c2c3", "d8b6", "d1b3", "c5c4", "b3c2", "c8f5", "c2c1", "e7e6", "b1d2"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "g8f6", "c1f4", "d7d5", "e2e3", "c7c5", "c2c3", "b8c6", "b1d2", "e7e6", "g1f3", "f8d6", "f4g3", "e8g8", "f1d3"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "g8f6", "c1f4", "g7g6", "e2e3", "f8g7", "g1f3", "e8g8", "f1e2", "d7d6", "h2h3", "c7c5", "c2c3", "b7b6", "e1g1", "c8b7", "b1d2", "b8d7"});
    add_line("D00", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "g1f3", "g8f6", "c1f4", "e7e6", "e2e3", "f8d6", "f4g3", "e8g8", "f1d3", "c7c5", "c2c3", "b8c6", "b1d2", "b7b6"});
    add_line("D02", "Queen's Pawn Game: London System", {"d2d4", "d7d5", "g1f3", "g8f6", "c1f4", "c7c5", "e2e3", "b8c6", "c2c3", "e7e6", "b1d2", "f8d6", "f4g3", "e8g8", "f1d3"});
    add_line("D05", "Colle System", {"d2d4", "d7d5", "g1f3", "g8f6", "e2e3", "e7e6", "f1d3", "c7c5", "c2c3", "b8c6", "b1d2", "f8d6", "e1g1", "e8g8", "d4c5", "d6c5", "e3e4", "d8c7"});
    add_line("A46", "Torre Attack", {"d2d4", "g8f6", "g1f3", "e7e6", "c1g5", "c7c5", "e2e3", "b7b6", "c2c3", "c8b7", "b1d2", "f8e7", "f1d3", "e8g8", "e1g1", "d7d6"});
    add_line("A45", "Trompowsky Attack", {"d2d4", "g8f6", "c1g5", "d7d5", "e2e3", "c7c5", "g5f6", "g7f6", "d4c5", "e7e6", "c2c4", "d5c4", "d1d8", "e8d8", "f1c4", "f8c5"});
    add_line("D06", "Queen's Gambit", {"d2d4", "d7d5", "c2c4"});
    add_line("D20", "Queen's Gambit Accepted", {"d2d4", "d7d5", "c2c4", "d5c4", "g1f3", "g8f6", "e2e3", "e7e6", "f1c4", "c7c5", "e1g1", "a7a6", "d1e2", "b7b5", "c4b3", "c8b7", "f1d1", "b8d7", "b1c3", "d8b8"});
    add_line("D30", "Queen's Gambit Declined", {"d2d4", "d7d5", "c2c4", "e7e6"});
    add_line("D35", "Queen's Gambit Declined: Exchange Variation", {"d2d4", "d7d5", "c2c4", "e7e6", "b1c3", "g8f6", "c4d5", "e6d5", "c1g5", "c7c6", "e2e3", "f8e7", "f1d3", "b8d7", "g1e2", "e8g8", "e1g1", "f8e8", "d1c2", "d7f8"});
    add_line("D37", "Queen's Gambit Declined: Classical Variation", {"d2d4", "d7d5", "c2c4", "e7e6", "b1c3", "g8f6", "g1f3", "f8e7", "c1g5", "e8g8", "e2e3", "h7h6", "g5h4", "b7b6", "c4d5", "f6d5", "h4e7", "d8e7", "c3d5", "e6d5", "a1c1", "c8e6"});
    add_line("D38", "Queen's Gambit Declined: Ragozin Defense", {"d2d4", "d7d5", "c2c4", "e7e6", "b1c3", "g8f6", "g1f3", "f8b4", "c4d5", "e6d5", "c1g5", "h7h6", "g5f6", "d8f6", "e2e3", "e8g8", "f1e2", "c7c6"});
    add_line("D58", "Queen's Gambit Declined: Tartakower Defense", {"d2d4", "d7d5", "c2c4", "e7e6", "b1c3", "g8f6", "c1g5", "f8e7", "e2e3", "h7h6", "g5h4", "e8g8", "g1f3", "b7b6", "f1e2", "c8b7", "h4f6", "e7f6", "c4d5", "e6d5", "b2b4", "c7c6", "e1g1"});
    add_line("D10", "Slav Defense", {"d2d4", "d7d5", "c2c4", "c7c6"});
    add_line("D11", "Slav Defense: Modern Line", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "e2e3", "c8f5", "b1c3", "e7e6", "f3h4", "f5g6", "h4g6", "h7g6"});
    add_line("D15", "Slav Defense: Three Knights", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "b1c3", "d5c4", "a2a4", "c8f5", "e2e3", "e7e6", "f1c4", "f8b4", "e1g1", "e8g8", "d1e2", "b8d7", "e3e4", "f5g6"});
    add_line("D43", "Semi-Slav Defense", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "b1c3", "e7e6"});
    add_line("D45", "Semi-Slav Defense: Main Line", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "b1c3", "e7e6", "e2e3", "b8d7", "d1c2", "f8d6", "f1d3", "e8g8", "e1g1", "d5c4", "d3c4", "b7b5", "c4d3", "c8b7"});
    add_line("D47", "Semi-Slav Defense: Meran Variation", {"d2d4", "d7d5", "c2c4", "c7c6", "g1f3", "g8f6", "b1c3", "e7e6", "e2e3", "b8d7", "f1d3", "d5c4", "d3c4", "b7b5", "c4d3", "c8b7", "e1g1", "a7a6", "e3e4", "c6c5", "d4d5"});
    add_line("E00", "Catalan Opening", {"d2d4", "g8f6", "c2c4", "e7e6", "g2g3", "d7d5", "f1g2", "f8e7", "g1f3", "e8g8", "e1g1", "d5c4", "d1c2", "a7a6", "a2a4", "c8d7", "c2c4", "d7c6", "c1f4"});
    add_line("E60", "King's Indian Defense", {"d2d4", "g8f6", "c2c4", "g7g6"});
    add_line("E61", "King's Indian Defense: Main Line", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "f8g7", "e2e4", "d7d6", "g1f3", "e8g8", "f1e2", "e7e5", "e1g1", "b8c6", "d4d5", "c6e7", "b2b4", "a7a5", "c1a3"});
    add_line("E70", "King's Indian Defense: Classical System", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "f8g7", "e2e4", "d7d6", "g1f3", "e8g8", "f1e2", "e7e5", "e1g1", "b8c6", "d4d5", "c6e7", "f3e1", "f6d7", "c1e3", "f7f5", "f2f3", "f5f4", "e3f2", "g6g5"});
    add_line("E81", "King's Indian Defense: Sämisch Variation", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "f8g7", "e2e4", "d7d6", "f2f3", "e8g8", "c1e3", "c7c5", "g1e2", "b8c6", "d1d2", "d8a5", "e1c1", "a7a6", "c1b1", "b7b5"});
    add_line("D70", "Grünfeld Defense", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "d7d5"});
    add_line("D85", "Grünfeld Defense: Exchange Variation", {"d2d4", "g8f6", "c2c4", "g7g6", "b1c3", "d7d5", "c4d5", "f6d5", "e2e4", "d5c3", "b2c3", "f8g7", "g1f3", "c7c5", "a1b1", "e8g8", "f1e2", "b8c6", "d4d5", "c6e5", "f3e5", "g7e5", "d1d2", "e7e6", "f2f4", "e5c7"});
    add_line("E20", "Nimzo-Indian Defense", {"d2d4", "g8f6", "c2c4", "e7e6", "b1c3", "f8b4"});
    add_line("E32", "Nimzo-Indian Defense: Classical Variation", {"d2d4", "g8f6", "c2c4", "e7e6", "b1c3", "f8b4", "d1c2", "e8g8", "a2a3", "b4c3", "c2c3", "b7b6", "c1g5", "c8b7", "f2f3", "d7d5", "e2e3", "b8d7", "c4d5", "e6d5"});
    add_line("E40", "Nimzo-Indian Defense: Rubinstein System", {"d2d4", "g8f6", "c2c4", "e7e6", "b1c3", "f8b4", "e2e3", "e8g8", "f1d3", "d7d5", "g1f3", "c7c5", "e1g1", "b8c6", "a2a3", "b4c3", "b2c3", "d5c4", "d3c4", "d8c7"});
    add_line("E12", "Queen's Indian Defense", {"d2d4", "g8f6", "c2c4", "e7e6", "g1f3", "b7b6", "g2g3", "c8a6", "b2b3", "f8b4", "c1d2", "b4e7", "f1g2", "c7c6", "d2c3", "d7d5", "f3e5", "f6d7", "e5d7", "b8d7", "b1d2", "e8g8", "e1g1"});
    add_line("A60", "Benoni Defense", {"d2d4", "g8f6", "c2c4", "c7c5", "d4d5", "e7e6", "b1c3", "e6d5", "c4d5", "d7d6", "e2e4", "g7g6", "f2f4", "f8g7", "f1b5", "f6d7", "a2a4", "e8g8", "g1f3", "b8a6", "e1g1", "a6b4"});
    add_line("A57", "Benko Gambit", {"d2d4", "g8f6", "c2c4", "c7c5", "d4d5", "b7b5", "c4b5", "a7a6", "b5a6", "g7g6", "b1c3", "c8a6", "g1f3", "f8g7", "g2g3", "d7d6", "f1g2", "b8d7", "a1b1", "e8g8", "e1g1", "d8a5"});
    add_line("A80", "Dutch Defense", {"d2d4", "f7f5"});
    add_line("A87", "Dutch Defense: Leningrad Variation", {"d2d4", "f7f5", "c2c4", "g8f6", "g2g3", "g7g6", "f1g2", "f8g7", "g1f3", "e8g8", "e1g1", "d7d6", "b1c3", "d8e8", "d4d5", "b8a6", "c1e3", "c8d7", "e3d4", "c7c5", "d5c6", "b7c6"});
    add_line("A10", "English Opening", {"c2c4"});
    add_line("A20", "English Opening: King's English Variation", {"c2c4", "e7e5"});
    add_line("A28", "English Opening: Four Knights", {"c2c4", "e7e5", "b1c3", "g8f6", "g1f3", "b8c6", "g2g3", "f8b4", "f1g2", "e8g8", "e1g1", "f8e8", "d2d3", "b4c3", "b2c3", "e5e4", "f3d4", "h7h6", "c1f4", "d7d6"});
    add_line("A30", "English Opening: Symmetrical Variation", {"c2c4", "c7c5", "g1f3", "g8f6", "b1c3", "b8c6", "d2d4", "c5d4", "f3d4", "e7e6", "g2g3", "d8b6", "d4b3", "c6e5", "e2e4", "f8b4", "d1e2", "d7d6", "c1d2", "e8g8", "f2f4", "e5c6"});
    add_line("A04", "Réti Opening", {"g1f3"});
    add_line("A09", "Réti Opening: King's Indian Attack", {"g1f3", "d7d5", "g2g3", "g8f6", "f1g2", "e7e6", "e1g1", "f8e7", "d2d3", "e8g8", "b1d2", "c7c5", "e2e4", "b8c6", "f1e1"});
}

} // namespace zugfish
