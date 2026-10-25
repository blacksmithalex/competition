#pragma once

#include "Participant.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Storage {

bool loadParticipants(const std::filesystem::path& path,
                      std::vector<Participant>& participants,
                      std::string& error);

bool saveParticipants(const std::filesystem::path& path,
                      const std::vector<Participant>& participants,
                      std::string& error);

bool saveResults(const std::filesystem::path& path,
                 const std::vector<Participant>& standings,
                 int prizePlaces,
                 std::string& error);

}  // namespace Storage

