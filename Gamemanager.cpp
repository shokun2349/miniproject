#include "Gamemanager.h"
#include <random>

GameManager::GameManager() {
    currentStage = 1;
    state = MAIN_MENU;
    // ไม่มีการ Fix ตัวละครตรงนี้แล้ว เพราะเราจะให้ผู้เล่นเลือกเอง
}

// ฟังก์ชันนี้แหละครับที่ Linker หาไม่เจอในตอนแรก!
void GameManager::setPlayerTeam(const std::vector<ClassType>& selectedClasses) {
    playerRoster.clear();
    for (ClassType type : selectedClasses) {
        if (type == KNIGHT) 
            playerRoster.push_back(Character("Knight", KNIGHT, 120, 25, 0, 15, 10, true));
        else if (type == ARCHER) 
            playerRoster.push_back(Character("Archer", ARCHER, 90, 30, 0, 10, 20, true));
        else if (type == MAGE) 
            playerRoster.push_back(Character("Mage", MAGE, 80, 10, 40, 5, 12, true));
        else if (type == TANK) 
            playerRoster.push_back(Character("Tank", TANK, 150, 15, 0, 25, 5, true));
        else if (type == SUPPORT) 
            playerRoster.push_back(Character("Support", SUPPORT, 90, 10, 25, 8, 15, true));
        else if (type == SPECIAL) 
            playerRoster.push_back(Character("Hero", SPECIAL, 100, 25, 25, 15, 18, true));
    }
}

void GameManager::startStage(int stage) {
    currentBattle = BattleSystem(); 
    currentBattle.addLog("--- STAGE " + std::to_string(stage) + " START! ---");

    for (auto& p : playerRoster) {
        if (p.isAlive()) {
            p.currentTurn = 0; 
            currentBattle.fighters.push_back(p);
        }
    }

    if (stage == 1) {
        currentBattle.fighters.push_back(Character("Goblin A", ARCHER, 60, 15, 0, 5, 18, false));
        currentBattle.fighters.push_back(Character("Orc Boss", TANK, 200, 20, 0, 20, 8, false));
    } else if (stage == 2) {
        currentBattle.addLog("Warning: The Demon King approaches!");
        currentBattle.fighters.push_back(Character("Dark Elf", MAGE, 100, 20, 35, 10, 22, false)); 
        currentBattle.fighters.push_back(Character("Demon King", SPECIAL, 300, 30, 20, 25, 15, false));
    }

    currentBattle.sortTurnOrderBySpeed(); 
    state = PLAYING;
}

void GameManager::updateBattleState() {
    if (state != PLAYING) return;

    if (currentBattle.isGameOver()) {
        int playerAlive = 0;

        for (auto& f : currentBattle.fighters) {
            if (f.isPlayer) {
                if (f.isAlive()) playerAlive++;
                for (auto& p : playerRoster) {
                    if (p.name == f.name) {
                        p.hp = f.hp; 
                    }
                }
            }
        }

        if (playerAlive == 0) {
            currentBattle.addLog("--- DEFEAT! All players are dead. ---");
            state = GAME_OVER;
        } else {
            currentBattle.addLog("--- VICTORY! All enemies are dead. ---");
            if (currentStage == 1) {
                state = STAGE_CLEAR;
            } else {
                state = GAME_WON;
            }
        }
    }
}

void GameManager::proceedToNextStage() {
    if (state == STAGE_CLEAR && currentStage == 1) {
        currentStage = 2;
        startStage(currentStage);
    }
}

void GameManager::triggerMathQuestion(bool isUlt) {
    isQueuedUlt = isUlt;
    playerInputString = ""; 
    state = ANSWERING_MATH; 

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distNum(1, 20);
    std::uniform_int_distribution<> distOp(0, 2);

    int a = distNum(gen);
    int b = distNum(gen);
    int op = distOp(gen);

    if (op == 0) {
        mathQuestion = std::to_string(a) + " + " + std::to_string(b) + " = ?";
        mathAnswer = a + b;
    } else if (op == 1) {
        if (a < b) std::swap(a, b); 
        mathQuestion = std::to_string(a) + " - " + std::to_string(b) + " = ?";
        mathAnswer = a - b;
    } else {
        a = (a % 10) + 1; 
        b = (b % 10) + 1;
        mathQuestion = std::to_string(a) + " * " + std::to_string(b) + " = ?";
        mathAnswer = a * b;
    }
}

void GameManager::submitMathAnswer() {
    int playerAns = -9999;
    try {
        if (!playerInputString.empty()) {
            playerAns = std::stoi(playerInputString); 
        }
    } catch (...) {
        playerAns = -9999;
    }

    Character& attacker = currentBattle.fighters[currentBattle.currentTurnIndex];
    Character* target = currentBattle.getTarget(true);

    if (playerAns == mathAnswer) {
        currentBattle.addLog("Math Correct! Action executed.");
        if (isQueuedUlt) currentBattle.executeUltimate(attacker, *target);
        else currentBattle.executeAttack(attacker, *target);
    } else {
        currentBattle.addLog("Wrong Answer! " + attacker.name + " got confused and missed!");
    }

    currentBattle.nextTurn();
    updateBattleState();
    
    if (state == ANSWERING_MATH) {
        state = PLAYING;
    }
}