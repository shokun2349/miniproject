#pragma once
#include <vector>
#include "character.h"
#include "BattleSystem.h"

// สถานะของเกมในปัจจุบัน
enum GameState { PLAYING, STAGE_CLEAR, GAME_OVER, GAME_WON };

class GameManager {
public:
    std::vector<Character> playerRoster; // เก็บข้อมูลทีมผู้เล่นข้ามด่าน
    BattleSystem currentBattle;          // ระบบต่อสู้ของด่านปัจจุบัน
    int currentStage;
    GameState state;

    GameManager();
    void initPlayerTeam();
    void startStage(int stage);
    void updateBattleState(); // เช็คผลแพ้ชนะหลังจบเทิร์น
    void proceedToNextStage();
};