#pragma once

#include "Participant.h"

#include <cstddef>
#include <string>
#include <vector>

class Contest {
public:
    enum class State {
        Registration,
        Scoring,
        Finished
    };

    static constexpr std::size_t MaxParticipants = 1000;
    static constexpr int MinPrizePlaces = 1;
    static constexpr int MaxPrizePlaces = 10;

    explicit Contest(int prizePlaces);

    bool registerParticipant(int id, const std::string& name, std::string& error);
    bool finishRegistration(std::string& error);
    bool registerScore(int participantId, int score, std::string& error);
    bool finishContest(std::string& error);

    [[nodiscard]] int prizePlaces() const;
    [[nodiscard]] State state() const;
    [[nodiscard]] const std::vector<Participant>& participants() const;
    [[nodiscard]] std::vector<Participant> standings() const;
    [[nodiscard]] std::vector<Participant> winners() const;

private:
    int prizePlaces_;
    State state_{State::Registration};
    std::vector<Participant> participants_;
};

