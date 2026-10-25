#include "Storage.h"

#include <fstream>
#include <sstream>
#include <system_error>

namespace Storage {

bool loadParticipants(const std::filesystem::path& path,
                      std::vector<Participant>& participants,
                      std::string& error) {
    participants.clear();
    if (!std::filesystem::exists(path)) {
        return true;
    }

    std::ifstream input(path);
    if (!input) {
        error = "Не удалось открыть файл участников: " + path.string();
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }
        const auto separator = line.find('\t');
        if (separator == std::string::npos) {
            error = "Неверный формат строки " + std::to_string(lineNumber) +
                    " в файле участников.";
            return false;
        }

        int id = 0;
        std::istringstream idStream(line.substr(0, separator));
        if (!(idStream >> id) || id <= 0 || !idStream.eof()) {
            error = "Неверный идентификатор в строке " + std::to_string(lineNumber) + ".";
            return false;
        }
        const std::string name = line.substr(separator + 1);
        if (name.empty()) {
            error = "Пустое имя в строке " + std::to_string(lineNumber) + ".";
            return false;
        }
        participants.push_back({id, name, std::nullopt});
    }
    return true;
}

bool saveParticipants(const std::filesystem::path& path,
                      const std::vector<Participant>& participants,
                      std::string& error) {
    const auto temporary = path.string() + ".tmp";
    std::ofstream output(temporary, std::ios::trunc);
    if (!output) {
        error = "Не удалось создать временный файл участников.";
        return false;
    }
    for (const auto& participant : participants) {
        output << participant.id << '\t' << participant.name << '\n';
    }
    output.close();
    if (!output) {
        error = "Ошибка записи файла участников.";
        return false;
    }

    std::error_code code;
    std::filesystem::rename(temporary, path, code);
    if (code) {
        std::filesystem::remove(path, code);
        code.clear();
        std::filesystem::rename(temporary, path, code);
    }
    if (code) {
        error = "Не удалось заменить файл участников: " + code.message();
        std::filesystem::remove(temporary);
        return false;
    }
    return true;
}

bool saveResults(const std::filesystem::path& path,
                 const std::vector<Participant>& standings,
                 int prizePlaces,
                 std::string& error) {
    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        error = "Не удалось создать файл результатов: " + path.string();
        return false;
    }

    const int cutoff = standings.at(static_cast<std::size_t>(prizePlaces - 1)).score.value();
    output << "ИТОГИ КОНКУРСА\n\n";
    output << "Место | ID | Имя | Балл | Статус\n";
    output << std::string(60, '-') << '\n';

    int place = 0;
    int previousScore = -1;
    for (std::size_t index = 0; index < standings.size(); ++index) {
        const int score = standings[index].score.value();
        if (score != previousScore) {
            place = static_cast<int>(index) + 1;
            previousScore = score;
        }
        output << place << " | " << standings[index].id << " | " << standings[index].name
               << " | " << score << " | " << (score >= cutoff ? "призёр" : "") << '\n';
    }
    return static_cast<bool>(output);
}

}  // namespace Storage

