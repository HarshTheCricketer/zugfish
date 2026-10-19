#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chess.hpp>

namespace zugfish {

struct PolyglotEntry {
    std::string uci;
    uint16_t weight = 0;
    uint32_t learn = 0;
};

class PolyglotBook {
public:
    PolyglotBook();
    explicit PolyglotBook(const std::string& filepath);

    bool load(const std::string& filepath);
    bool is_loaded() const { return !entries_raw_.empty(); }
    size_t size() const { return entries_raw_.size() / 16; }

    // Check if the given move (UCI string) is a book move for the board position
    bool is_book_move(const chess::Board& board, const std::string& uci) const;

    // Retrieve all book moves for the current board position, sorted by weight descending
    std::vector<PolyglotEntry> get_entries(const chess::Board& board) const;

    // Decode a 16-bit Polyglot move into a UCI move string (e.g. 0x31c -> "e2e4")
    static std::string decode_move(uint16_t move_bits);

private:
    std::vector<uint8_t> entries_raw_;
};

} // namespace zugfish
