#include "Gamemanager.h"

GameManager::GameManager() {
    currentStage = 1;
    state = PLAYING;
    initPlayerTeam();
    startStage(currentStage);
}

void GameManager::initPlayerTeam() {
    playerRoster.push_back(Character("Knight(Player)", KNIGHT, 120, 25, 0, 15, 10, true));
    playerRoster.push_back(Character("Mage(Player)", MAGE, 80, 10, 40, 5, 12, true));
    playerRoster.push_back(Character("Support(Player)", SUPPORT, 90, 10, 25, 8, 15, true));
}

void GameManager::startStage(int stage) {
    currentBattle = BattleSystem(); // รีเซ็ตระบบต่อสู้ใหม่หมด
    currentBattle.addLog("--- STAGE " + std::to_string(stage) + " START! ---");

    // ดึงเฉพาะตัวละครผู้เล่นที่ยังมีชีวิตอยู่เข้าร่วมต่อสู้
    for (auto& p : playerRoster) {
        if (p.isAlive()) {
            p.currentTurn = 0; // รีเซ็ตเกจท่าไม้ตายเมื่อเริ่มด่านใหม่
            currentBattle.fighters.push_back(p);
        }
    }

    // เซ็ตศัตรูตามด่าน
    if (stage == 1) {
        currentBattle.fighters.push_back(Character("Goblin A", ARCHER, 60, 15, 0, 5, 18, false));
        currentBattle.fighters.push_back(Character("Orc Boss", TANK, 200, 20, 0, 20, 8, false));
    } else if (stage == 2) {
        currentBattle.addLog("Warning: The Demon King approaches!");
        // มอนสเตอร์ด่าน 2 เก่งขึ้น อึดขึ้น ไวขึ้น
        currentBattle.fighters.push_back(Character("Dark Elf", MAGE, 100, 20, 35, 10, 22, false)); 
        currentBattle.fighters.push_back(Character("Demon King", SPECIAL, 300, 30, 20, 25, 15, false));
    }

    currentBattle.sortTurnOrderBySpeed(); // เรียงคิวใหม่
    state = PLAYING;
}

void GameManager::updateBattleState() {
    if (state != PLAYING) return;

    if (currentBattle.isGameOver()) {
        int playerAlive = 0;

        // อัปเดตเลือดของตัวละครที่รอดชีวิตกลับไปยัง Roster หลัก
        for (auto& f : currentBattle.fighters) {
            if (f.isPlayer) {
                if (f.isAlive()) playerAlive++;
                
                // หาตัวละครใน Roster แล้วอัปเดต HP ล่าสุด
                for (auto& p : playerRoster) {
                    if (p.name == f.name) {
                        p.hp = f.hp; 
                    }
                }
            }
        }

        // ตัดสินผลลัพธ์
        if (playerAlive == 0) {
            state = GAME_OVER;
        } else {
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