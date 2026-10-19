#include "polyglot.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace zugfish {

static inline uint64_t read_u64_be(const uint8_t* p) {
    return (static_cast<uint64_t>(p[0]) << 56) |
           (static_cast<uint64_t>(p[1]) << 48) |
           (static_cast<uint64_t>(p[2]) << 40) |
           (static_cast<uint64_t>(p[3]) << 32) |
           (static_cast<uint64_t>(p[4]) << 24) |
           (static_cast<uint64_t>(p[5]) << 16) |
           (static_cast<uint64_t>(p[6]) << 8)  |
           (static_cast<uint64_t>(p[7]));
}

static inline uint16_t read_u16_be(const uint8_t* p) {
    return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
}

static inline uint32_t read_u32_be(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           (static_cast<uint32_t>(p[3]));
}

PolyglotBook::PolyglotBook() = default;

PolyglotBook::PolyglotBook(const std::string& filepath) {
    load(filepath);
}

bool PolyglotBook::load(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    std::streamsize size = file.tellg();
    if (size <= 0 || size % 16 != 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    entries_raw_.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(entries_raw_.data()), size)) {
        entries_raw_.clear();
        return false;
    }
    return true;
}

std::string PolyglotBook::decode_move(uint16_t move_bits) {
    int to_sq   = move_bits & 0x3f;
    int from_sq = (move_bits >> 6) & 0x3f;
    int prom    = (move_bits >> 12) & 0x7;

    char f1 = 'a' + (from_sq % 8);
    char r1 = '1' + (from_sq / 8);
    char f2 = 'a' + (to_sq % 8);
    char r2 = '1' + (to_sq / 8);

    std::string uci;
    uci += f1;
    uci += r1;
    uci += f2;
    uci += r2;

    if (prom == 1) uci += 'n';
    else if (prom == 2) uci += 'b';
    else if (prom == 3) uci += 'r';
    else if (prom == 4) uci += 'q';

    // Polyglot castling encoding converts "king captures friendly rook" to standard UCI
    if (uci == "e1h1") uci = "e1g1";
    else if (uci == "e1a1") uci = "e1c1";
    else if (uci == "e8h8") uci = "e8g8";
    else if (uci == "e8a8") uci = "e8c8";

    return uci;
}

std::vector<PolyglotEntry> PolyglotBook::get_entries(const chess::Board& board) const {
    std::vector<PolyglotEntry> results;
    if (entries_raw_.empty()) return results;

    uint64_t target_key = board.hash();
    size_t num_entries = entries_raw_.size() / 16;

    size_t low = 0;
    size_t high = num_entries;
    size_t first_idx = num_entries;

    // Binary search for first occurrence of target_key
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        uint64_t mid_key = read_u64_be(&entries_raw_[mid * 16]);
        if (mid_key >= target_key) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    if (low < num_entries && read_u64_be(&entries_raw_[low * 16]) == target_key) {
        first_idx = low;
    }

    if (first_idx == num_entries) {
        return results;
    }

    for (size_t i = first_idx; i < num_entries; ++i) {
        const uint8_t* ptr = &entries_raw_[i * 16];
        uint64_t key = read_u64_be(ptr);
        if (key != target_key) break;

        uint16_t move_bits = read_u16_be(ptr + 8);
        uint16_t weight = read_u16_be(ptr + 10);
        uint32_t learn = read_u32_be(ptr + 12);

        PolyglotEntry entry;
        entry.uci = decode_move(move_bits);
        entry.weight = weight;
        entry.learn = learn;
        results.push_back(entry);
    }

    std::sort(results.begin(), results.end(), [](const PolyglotEntry& a, const PolyglotEntry& b) {
        return a.weight > b.weight;
    });

    return results;
}

bool PolyglotBook::is_book_move(const chess::Board& board, const std::string& uci) const {
    auto entries = get_entries(board);
    for (const auto& entry : entries) {
        if (entry.uci == uci) {
            return true;
        }
    }
    return false;
}

} // namespace zugfish
