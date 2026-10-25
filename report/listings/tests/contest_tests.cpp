#include "Contest.h"

#include <cassert>
#include <iostream>
#include <string>

namespace {

void add(Contest& contest, int id, const std::string& name) {
    std::string error;
    assert(contest.registerParticipant(id, name, error));
}

void score(Contest& contest, int id, int value) {
    std::string error;
    assert(contest.registerScore(id, value, error));
}

void testTieExpandsWinners() {
    Contest contest(2);
    add(contest, 1, "Анна");
    add(contest, 2, "Борис");
    add(contest, 3, "Вера");
    add(contest, 4, "Глеб");

    std::string error;
    assert(contest.finishRegistration(error));
    score(contest, 1, 100);
    score(contest, 2, 90);
    score(contest, 3, 90);
    score(contest, 4, 70);
    assert(contest.finishContest(error));

    const auto winners = contest.winners();
    assert(winners.size() == 3);
    assert(winners[0].id == 1);
    assert(winners[1].score == 90);
    assert(winners[2].score == 90);
}

void testValidation() {
    Contest contest(1);
    std::string error;
    assert(!contest.registerParticipant(0, "Анна", error));
    assert(contest.registerParticipant(1, "Анна", error));
    assert(!contest.registerParticipant(1, "Другое имя", error));
    assert(contest.finishRegistration(error));
    assert(!contest.registerScore(1, 101, error));
    assert(contest.registerScore(1, 50, error));
    assert(!contest.registerScore(1, 60, error));
    assert(contest.finishContest(error));
}

void testCannotFinishWithoutAllScores() {
    Contest contest(1);
    add(contest, 1, "Анна");
    add(contest, 2, "Борис");
    std::string error;
    assert(contest.finishRegistration(error));
    score(contest, 1, 100);
    assert(!contest.finishContest(error));
}

}  // namespace

int main() {
    testTieExpandsWinners();
    testValidation();
    testCannotFinishWithoutAllScores();
    std::cout << "Все тесты пройдены.\n";
    return 0;
}

