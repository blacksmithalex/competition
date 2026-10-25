#pragma once

#include <optional>
#include <string>

struct Participant {
    int id{};
    std::string name;
    std::optional<int> score;
};

