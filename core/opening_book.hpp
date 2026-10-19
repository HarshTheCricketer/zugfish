#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <chess.hpp>
#include "polyglot.hpp"

namespace zugfish {

struct OpeningInfo {
    std::string eco;
    std::string name;
};

class OpeningBook {
public:
    static OpeningBook& instance();

    // Check if the next move continues a known opening book line
    bool is_book_move(const chess::Board& board, const std::vector<std::string>& uci_history, const std::string& next_uci) const;

    // Backward-compatible overload for uci_history only
    bool is_book_move(const std::vector<std::string>& uci_history, const std::string& next_uci) const;

    // Identify the deepest opening variation matched by the moves played so far
    OpeningInfo identify_opening(const std::vector<std::string>& uci_history) const;

    // Check if polyglot tournament database is active
    bool has_polyglot() const { return polyglot_.is_loaded(); }
    size_t polyglot_size() const { return polyglot_.size(); }
    size_t eco_count() const { return eco_count_; }

private:
    struct Node {
        std::string uci;
        OpeningInfo opening;
        std::unordered_map<std::string, std::unique_ptr<Node>> children;
    };

    OpeningBook();
    void add_line(const std::string& eco, const std::string& name, const std::vector<std::string>& uci_moves);
    void load_openings();
    void load_embedded_fallback();

    std::unique_ptr<Node> root_;
    PolyglotBook polyglot_;
    size_t eco_count_ = 0;
};

} // namespace zugfish
