#pragma once
#include <vector>
#include <string>
#include "character.h"

class BattleSystem {
public:
    std::vector<Character> fighters;
    std::vector<std::string> battleLogs;
    int currentTurnIndex = 0;

    void addLog(std::string log);
    void sortTurnOrderBySpeed();
    bool isGameOver();

    Character* getTarget(bool isPlayer);
    Character* getAlly(bool isPlayer);

    void executeAttack(Character& attacker, Character& target);
    void executeUltimate(Character& attacker, Character& target);
    void nextTurn();
};