#pragma once
#include <vector>
#include "character.h"
#include "BattleSystem.h"

// สถานะของเกมในปัจจุบัน
enum GameState { MAIN_MENU, PARTY_SELECTION, PLAYING, ANSWERING_MATH, STAGE_CLEAR, GAME_OVER, GAME_WON };

class GameManager {
public:
    std::vector<Character> playerRoster; // เก็บข้อมูลทีมผู้เล่นข้ามด่าน
    BattleSystem currentBattle;          // ระบบต่อสู้ของด่านปัจจุบัน
    int currentStage;
    GameState state;

    // ตัวแปรสำหรับระบบคณิตศาสตร์
    std::string mathQuestion;
    int mathAnswer;
    std::string playerInputString;
    bool isQueuedUlt; // จำไว้ว่าผู้เล่นกดโจมตีปกติ หรือ ท่าไม้ตาย

    GameManager();
    // ฟังก์ชันใหม่สำหรับจัดทีมตามที่ผู้เล่นเลือก
    void setPlayerTeam(const std::vector<ClassType>& selectedClasses);
    
    void initPlayerTeam();
    void startStage(int stage);
    void updateBattleState(); // เช็คผลแพ้ชนะหลังจบเทิร์น
    void proceedToNextStage();

    // ฟังก์ชันใหม่สำหรับคณิตศาสตร์
    void triggerMathQuestion(bool isUlt);
    void submitMathAnswer();
};