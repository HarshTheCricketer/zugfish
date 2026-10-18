#pragma once

#include "ply.hpp"
#include <map>
#include <string>
#include <vector>

namespace zugfish {

struct Game
{    
    std::map<std::string, std::string> headers;
    std::vector<Ply> plies;
};

} // namespace zugfish