#include "Gamemanager.h"
#include <random>
GameManager::GameManager() {
    currentStage = 1;
    state = MAIN_MENU;
    initPlayerTeam();
    
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

//  ฟังก์ชันนี้จะถูกเรียกเมื่อผู้เล่นเลือกโจมตีหรือใช้ท่าไม้ตาย มันจะสุ่มคำถามคณิตศาสตร์และรอให้ผู้เล่นตอบ
void GameManager::triggerMathQuestion(bool isUlt) {
    isQueuedUlt = isUlt;
    playerInputString = ""; // ล้างคำตอบเก่า
    state = ANSWERING_MATH; // เปลี่ยนสถานะให้เกมหยุดรอ

    // สุ่มตัวเลขและเครื่องหมาย
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
        if (a < b) std::swap(a, b); // สลับไม่ให้คำตอบติดลบ เพื่อให้พิมพ์ง่ายขึ้น
        mathQuestion = std::to_string(a) + " - " + std::to_string(b) + " = ?";
        mathAnswer = a - b;
    } else {
        a = (a % 10) + 1; // เลขคูณเอาแค่ 1-10
        b = (b % 10) + 1;
        mathQuestion = std::to_string(a) + " * " + std::to_string(b) + " = ?";
        mathAnswer = a * b;
    }
}

void GameManager::submitMathAnswer() {
    int playerAns = -9999;
    try {
        if (!playerInputString.empty()) {
            playerAns = std::stoi(playerInputString); // แปลงข้อความที่พิมพ์เป็นตัวเลข
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
        // ถ้าตอบผิด ให้โจมตีพลาด
        currentBattle.addLog("Wrong Answer! " + attacker.name + " got confused and missed!");
    }

    // จบเทิร์นและกลับสู่สถานะต่อสู้
    currentBattle.nextTurn();
    updateBattleState();
    
    if (state == ANSWERING_MATH) {
        state = PLAYING;
    }
}