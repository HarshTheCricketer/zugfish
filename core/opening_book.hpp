#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace zugfish {

struct OpeningInfo {
    std::string eco;
    std::string name;
};

class OpeningBook {
public:
    static OpeningBook& instance();

    // Check if the next move continues a known opening book line
    bool is_book_move(const std::vector<std::string>& uci_history, const std::string& next_uci) const;

    // Identify the deepest opening variation matched by the moves played so far
    OpeningInfo identify_opening(const std::vector<std::string>& uci_history) const;

private:
    struct Node {
        std::string uci;
        OpeningInfo opening;
        std::unordered_map<std::string, std::unique_ptr<Node>> children;
    };

    OpeningBook();
    void add_line(const std::string& eco, const std::string& name, const std::vector<std::string>& uci_moves);
    void load_openings();

    std::unique_ptr<Node> root_;
};

} // namespace zugfish
