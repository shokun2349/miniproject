#include "BattleSystem.h"
#include <algorithm>
#include <random>

// ฟังก์ชันสุ่มตัวเลข (ใช้แค่ในไฟล์นี้)
namespace {
    int getRandom(int min, int max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(min, max);
        return distr(gen);
    }
}

void BattleSystem::addLog(std::string log) {
    battleLogs.push_back(log);
    if (battleLogs.size() > 5) battleLogs.erase(battleLogs.begin());
}

void BattleSystem::sortTurnOrderBySpeed() {
    std::sort(fighters.begin(), fighters.end(), [](const Character& a, const Character& b) {
        return a.speed > b.speed;
    });
}

bool BattleSystem::isGameOver() {
    int playerAlive = 0, enemyAlive = 0;
    for (auto& f : fighters) {
        if (f.isAlive()) {
            if (f.isPlayer) playerAlive++;
            else enemyAlive++;
        }
    }
    return (playerAlive == 0 || enemyAlive == 0); 
}


Character* BattleSystem::getTarget(bool isPlayer) {
    for (auto& f : fighters) {
        if (f.isAlive() && f.isPlayer != isPlayer) return &f;
    }
    return nullptr;
}

Character* BattleSystem::getAlly(bool isPlayer) {
    Character* target = nullptr;
    for (auto& f : fighters) {
        if (f.isAlive() && f.isPlayer == isPlayer) {
            if (target == nullptr || f.hp < target->hp) target = &f;
        }
    }
    return target;
}

void BattleSystem::executeAttack(Character& attacker, Character& target) {
    if (target.speed > attacker.speed) {
        int missChance = (target.speed - attacker.speed) * 2;
        if (getRandom(1, 100) <= missChance + 10) { 
            addLog(attacker.name + " attacked " + target.name + " but MISSED!");
            return;
        }
    }

    int damage = (attacker.atk + (attacker.speed / 5)) - target.def;
    if (damage <= 0) damage = 1;

    target.takeDamage(damage);
    addLog(attacker.name + " dealt " + std::to_string(damage) + " dmg to " + target.name);
}

void BattleSystem::executeUltimate(Character& attacker, Character& target) {
    attacker.currentTurn = 0; // รีเซ็ตเกจ

    if (attacker.type == SUPPORT) {
        Character* ally = getAlly(attacker.isPlayer);
        int healAmount = attacker.magic * 2;
        if (ally) {
            ally->heal(healAmount);
            addLog(attacker.name + " cast Ultimate! Healed " + ally->name + " for " + std::to_string(healAmount));
        }
    } 
    else if (attacker.type == MAGE) {
        int damage = (attacker.magic * 2) - target.def;
        if (damage <= 0) damage = 1;
        target.takeDamage(damage);
        addLog(attacker.name + " cast METEOR! Dealt " + std::to_string(damage) + " magic dmg to " + target.name);
    }
    else {
        int damage = (attacker.atk * 3) - target.def;
        if (damage <= 0) damage = 1;
        target.takeDamage(damage);
        addLog(attacker.name + " cast ULTIMATE STRIKE! Dealt " + std::to_string(damage) + " dmg to " + target.name);
    }
}

void BattleSystem::nextTurn() {
    do {
        currentTurnIndex = (currentTurnIndex + 1) % fighters.size();
    } while (!fighters[currentTurnIndex].isAlive() && !isGameOver());
    fighters[currentTurnIndex].currentTurn++; 
}