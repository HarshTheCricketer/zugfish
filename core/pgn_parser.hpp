#pragma once

#include "game.hpp"
#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>

namespace zugfish {

struct ParseResult {
    bool success = false;
    Game game;
    std::string error;
};

// Parses a single game from an input stream.
ParseResult parse_pgn(std::istream& stream);

// Parses a single game from a PGN string.
ParseResult parse_pgn(std::string_view pgn_text);

// Parses a single game from a PGN file.
ParseResult parse_pgn_file(const std::filesystem::path& file_path);

} // namespace zugfish
