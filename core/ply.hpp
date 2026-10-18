#pragma once

#include <string>

namespace zugfish {

struct Ply
{
    std::string san;
    std::string uci;
    std::string fen_before;
};

} // namespace zugfish