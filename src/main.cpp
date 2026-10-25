#include "Contest.h"
#include "Storage.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <string>
#include <vector>

namespace {

const std::filesystem::path ParticipantsFile = "participants.txt";
const std::filesystem::path ResultsFile = "results.txt";

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int readInteger(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        std::cout << prompt;
        int value = 0;
        if (std::cin >> value && value >= minimum && value <= maximum) {
            clearInput();
            return value;
        }
        std::cout << "Введите целое число от " << minimum << " до " << maximum << ".\n";
        clearInput();
    }
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void printParticipants(const std::vector<Participant>& participants) {
    if (participants.empty()) {
        std::cout << "Список участников пуст.\n";
        return;
    }
    std::cout << "ID | Имя\n";
    std::cout << std::string(50, '-') << '\n';
    for (const auto& participant : participants) {
        std::cout << participant.id << " | " << participant.name;
        if (participant.score.has_value()) {
            std::cout << " — " << participant.score.value() << " балл(ов)";
        }
        std::cout << '\n';
    }
}

void printStandings(const Contest& contest) {
    const auto standings = contest.standings();
    const int cutoff = standings.at(static_cast<std::size_t>(contest.prizePlaces() - 1)).score.value();
    std::cout << "\nИТОГИ КОНКУРСА\n";
    std::cout << "Место | ID | Имя | Балл | Статус\n";
    std::cout << std::string(60, '-') << '\n';

    int place = 0;
    int previousScore = -1;
    for (std::size_t index = 0; index < standings.size(); ++index) {
        const int score = standings[index].score.value();
        if (score != previousScore) {
            place = static_cast<int>(index) + 1;
            previousScore = score;
        }
        std::cout << place << " | " << standings[index].id << " | " << standings[index].name
                  << " | " << score << " | " << (score >= cutoff ? "призёр" : "") << '\n';
    }
}

bool loadSavedParticipants(Contest& contest) {
    std::vector<Participant> saved;
    std::string error;
    if (!Storage::loadParticipants(ParticipantsFile, saved, error)) {
        std::cerr << "Ошибка: " << error << '\n';
        return false;
    }
    for (const auto& participant : saved) {
        if (!contest.registerParticipant(participant.id, participant.name, error)) {
            std::cerr << "Ошибка загрузки участника ID " << participant.id << ": " << error << '\n';
            return false;
        }
    }
    if (!saved.empty()) {
        std::cout << "Загружено участников: " << saved.size() << ".\n";
    }
    return true;
}

bool saveRegistration(const Contest& contest) {
    std::string error;
    if (!Storage::saveParticipants(ParticipantsFile, contest.participants(), error)) {
        std::cerr << "Ошибка: " << error << '\n';
        return false;
    }
    std::cout << "Регистрация сохранена в " << ParticipantsFile.string() << ".\n";
    return true;
}

void registrationMenu(Contest& contest) {
    while (contest.state() == Contest::State::Registration) {
        std::cout << "\n1. Зарегистрировать участника\n"
                     "2. Показать участников\n"
                     "3. Завершить регистрацию\n"
                     "0. Сохранить и выйти\n";
        const int choice = readInteger("Выберите действие: ", 0, 3);
        if (choice == 0) {
            saveRegistration(contest);
            return;
        }
        if (choice == 1) {
            const int id = readInteger("Идентификатор: ", 1, std::numeric_limits<int>::max());
            const std::string name = readLine("Имя участника: ");
            std::string error;
            if (contest.registerParticipant(id, name, error)) {
                std::cout << "Участник зарегистрирован.\n";
            } else {
                std::cout << "Ошибка: " << error << '\n';
            }
        } else if (choice == 2) {
            printParticipants(contest.participants());
        } else {
            std::string error;
            if (contest.finishRegistration(error)) {
                std::error_code code;
                std::filesystem::remove(ParticipantsFile, code);
                std::cout << "Регистрация завершена. Переход к вводу баллов.\n";
            } else {
                std::cout << "Ошибка: " << error << '\n';
            }
        }
    }
}

void scoringMenu(Contest& contest) {
    while (contest.state() == Contest::State::Scoring) {
        std::cout << "\n1. Зарегистрировать балл\n"
                     "2. Показать участников и баллы\n"
                     "3. Завершить конкурс\n";
        const int choice = readInteger("Выберите действие: ", 1, 3);
        if (choice == 1) {
            const int id = readInteger("Идентификатор участника: ", 1,
                                       std::numeric_limits<int>::max());
            const int score = readInteger("Балл: ", 0, 100);
            std::string error;
            if (contest.registerScore(id, score, error)) {
                std::cout << "Балл зарегистрирован.\n";
            } else {
                std::cout << "Ошибка: " << error << '\n';
            }
        } else if (choice == 2) {
            printParticipants(contest.participants());
        } else {
            std::string error;
            if (contest.finishContest(error)) {
                std::cout << "Конкурс завершён.\n";
            } else {
                std::cout << "Ошибка: " << error << '\n';
            }
        }
    }
}

}  // namespace

int main() {
    try {
        std::locale::global(std::locale(""));
    } catch (const std::exception&) {
    }

    std::cout << "КОНКУРС\n";
    const int prizePlaces = readInteger("Количество призовых мест (1–10): ", 1, 10);
    Contest contest(prizePlaces);
    if (!loadSavedParticipants(contest)) {
        return 1;
    }

    registrationMenu(contest);
    if (contest.state() == Contest::State::Registration) {
        return 0;
    }

    scoringMenu(contest);
    printStandings(contest);

    std::string error;
    if (!Storage::saveResults(ResultsFile, contest.standings(), contest.prizePlaces(), error)) {
        std::cerr << "Ошибка: " << error << '\n';
        return 1;
    }
    std::cout << "\nРезультаты сохранены в " << ResultsFile.string() << ".\n";
    return 0;
}

