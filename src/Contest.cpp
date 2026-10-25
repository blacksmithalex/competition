#include "Contest.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {

bool isBlank(const std::string& text) {
    return std::all_of(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

}  // namespace

Contest::Contest(int prizePlaces) : prizePlaces_(prizePlaces) {
    if (prizePlaces < MinPrizePlaces || prizePlaces > MaxPrizePlaces) {
        throw std::invalid_argument("Количество призовых мест должно быть от 1 до 10.");
    }
}

bool Contest::registerParticipant(int id, const std::string& name, std::string& error) {
    if (state_ != State::Registration) {
        error = "Регистрация участников уже завершена.";
        return false;
    }
    if (participants_.size() >= MaxParticipants) {
        error = "Достигнуто максимальное количество участников: 1000.";
        return false;
    }
    if (id <= 0) {
        error = "Идентификатор должен быть положительным целым числом.";
        return false;
    }
    if (name.empty() || isBlank(name)) {
        error = "Имя участника не может быть пустым.";
        return false;
    }
    if (name.find('\t') != std::string::npos || name.find('\n') != std::string::npos ||
        name.find('\r') != std::string::npos) {
        error = "Имя не должно содержать символы табуляции или перевода строки.";
        return false;
    }
    const auto duplicate = std::find_if(participants_.begin(), participants_.end(),
                                        [id](const Participant& participant) {
                                            return participant.id == id;
                                        });
    if (duplicate != participants_.end()) {
        error = "Участник с таким идентификатором уже зарегистрирован.";
        return false;
    }

    participants_.push_back({id, name, std::nullopt});
    return true;
}

bool Contest::finishRegistration(std::string& error) {
    if (state_ != State::Registration) {
        error = "Регистрация уже завершена.";
        return false;
    }
    if (participants_.size() < static_cast<std::size_t>(prizePlaces_)) {
        error = "Участников не может быть меньше, чем призовых мест.";
        return false;
    }
    state_ = State::Scoring;
    return true;
}

bool Contest::registerScore(int participantId, int score, std::string& error) {
    if (state_ != State::Scoring) {
        error = "Баллы можно вводить только после окончания регистрации.";
        return false;
    }
    if (score < 0 || score > 100) {
        error = "Балл должен находиться в диапазоне от 0 до 100.";
        return false;
    }

    const auto participant = std::find_if(participants_.begin(), participants_.end(),
                                          [participantId](const Participant& item) {
                                              return item.id == participantId;
                                          });
    if (participant == participants_.end()) {
        error = "Участник с указанным идентификатором не найден.";
        return false;
    }
    if (participant->score.has_value()) {
        error = "Балл этого участника уже зарегистрирован.";
        return false;
    }

    participant->score = score;
    return true;
}

bool Contest::finishContest(std::string& error) {
    if (state_ != State::Scoring) {
        error = "Сейчас конкурс нельзя завершить.";
        return false;
    }
    const auto missingScore = std::find_if(participants_.begin(), participants_.end(),
                                           [](const Participant& participant) {
                                               return !participant.score.has_value();
                                           });
    if (missingScore != participants_.end()) {
        error = "Не введён балл участника с идентификатором " +
                std::to_string(missingScore->id) + ".";
        return false;
    }
    state_ = State::Finished;
    return true;
}

int Contest::prizePlaces() const {
    return prizePlaces_;
}

Contest::State Contest::state() const {
    return state_;
}

const std::vector<Participant>& Contest::participants() const {
    return participants_;
}

std::vector<Participant> Contest::standings() const {
    auto result = participants_;
    std::sort(result.begin(), result.end(), [](const Participant& left, const Participant& right) {
        if (left.score != right.score) {
            return left.score.value_or(-1) > right.score.value_or(-1);
        }
        if (left.name != right.name) {
            return left.name < right.name;
        }
        return left.id < right.id;
    });
    return result;
}

std::vector<Participant> Contest::winners() const {
    if (state_ != State::Finished) {
        return {};
    }

    const auto sorted = standings();
    const int cutoff = sorted.at(static_cast<std::size_t>(prizePlaces_ - 1)).score.value();
    std::vector<Participant> result;
    std::copy_if(sorted.begin(), sorted.end(), std::back_inserter(result),
                 [cutoff](const Participant& participant) {
                     return participant.score.value() >= cutoff;
                 });
    return result;
}

