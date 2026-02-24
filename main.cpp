#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include "Gamemanager.h"

int main() {
    // SFML 3.0: VideoMode ใช้ปีกกา {} สำหรับ Vector2u
    std::cout << "[1] เริ่มต้นเปิดหน้าต่าง..." << std::endl;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Turn-Based RPG Mechanics");
    window.setFramerateLimit(60);

    sf::Font font;
    // SFML 3.0: ใช้ openFromFile แทน loadFromFile สำหรับ Font
    std::cout << "[2] กำลังโหลดฟอนต์..." << std::endl;
    if (!font.openFromFile("ARIAL.TTF")) {
        std::cout << "Error loading font! Please put ARIAL.ttf in the same directory." << std::endl;
        return -1;
    }
    // สร้างระบบเกม (มันจะสร้างด่าน 1 ให้เองอัตโนมัติ)
    GameManager game;
    
    std::cout << "[3] กำลังสร้างตัวละคร..." << std::endl;
    BattleSystem battle;
    // เซ็ตทีมผู้เล่น
    battle.fighters.push_back(Character("Knight(Player)", KNIGHT, 120, 25, 0, 15, 10, true));
    battle.fighters.push_back(Character("Mage(Player)", MAGE, 80, 10, 40, 5, 12, true));
    battle.fighters.push_back(Character("Support(Player)", SUPPORT, 90, 10, 25, 8, 15, true));

    // เซ็ตทีมศัตรู
    battle.fighters.push_back(Character("Goblin A", ARCHER, 60, 15, 0, 5, 18, false));
    battle.fighters.push_back(Character("Orc Boss", TANK, 200, 20, 0, 20, 8, false));

    std::cout << "[4] กำลังจัดเรียงความเร็วและเข้าลูปเกม..." << std::endl;
    battle.sortTurnOrderBySpeed();
    battle.addLog("Battle Started!");

    bool turnActionCompleted = false;
    
    // เพิ่ม 2 ตัวแปรนี้สำหรับจับเวลาศัตรู
    sf::Clock enemyTimer;
    bool isEnemyWaiting = false;
    while (window.isOpen()) {
        std::cout << ">> [5] เริ่มเช็ค Event (คีย์บอร์ด/เมาส์)" << std::endl;
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                if (!battle.isGameOver()) {
                    Character& currentFighter = battle.fighters[battle.currentTurnIndex];
                    if (currentFighter.isPlayer && currentFighter.isAlive()) {
                        Character* target = battle.getTarget(true);
                        if (!target) continue;

                        if (keyReleased->code == sf::Keyboard::Key::Num1) {
                            battle.executeAttack(currentFighter, *target);
                            turnActionCompleted = true;
                        } 
                        else if (keyReleased->code == sf::Keyboard::Key::Num2) {
                            if (currentFighter.canUseUlt()) {
                                battle.executeUltimate(currentFighter, *target);
                                turnActionCompleted = true;
                            } else {
                                battle.addLog("Ultimate not ready!");
                            }
                        }
                    }
                }
            }
        }

        std::cout << ">> [6] เช็คเทิร์นศัตรู (Auto)" << std::endl;
        if (!battle.isGameOver()) {
            Character& currentFighter = battle.fighters[battle.currentTurnIndex];
            if (!currentFighter.isPlayer && currentFighter.isAlive()) {
                
                // ถ้าเพิ่งเข้าเทิร์นศัตรู ให้เริ่มจับเวลา
                if (!isEnemyWaiting) {
                    isEnemyWaiting = true;
                    enemyTimer.restart(); 
                }

                // รอจนกว่าจะครบ 1 วินาที ถึงจะออกท่าโจมตี
                if (isEnemyWaiting && enemyTimer.getElapsedTime().asSeconds() >= 1.0f) {
                    Character* target = battle.getTarget(false);
                    if (target) {
                        if (currentFighter.canUseUlt()) battle.executeUltimate(currentFighter, *target);
                        else battle.executeAttack(currentFighter, *target);
                        
                        turnActionCompleted = true;
                    }
                    isEnemyWaiting = false; // โจมตีเสร็จแล้ว รีเซ็ตสถานะรอ
                }
            }
        }

        std::cout << ">> [7] เปลี่ยนเทิร์น (ถ้ามีการกระทำ)" << std::endl;
        if (turnActionCompleted) {
            battle.nextTurn();
            turnActionCompleted = false;
        }

        std::cout << ">> [8] เริ่มเคลียร์หน้าจอและวาดภาพ" << std::endl;
        window.clear(sf::Color(30, 30, 30));

        sf::Text statText(font);
        statText.setCharacterSize(16);
        float playerYPos = 50.0f;
        float enemyYPos = 50.0f;
        
        for (size_t i = 0; i < battle.fighters.size(); ++i) {
            auto& f = battle.fighters[i];
            std::string info = f.name + " (HP: " + std::to_string(f.hp) + "/" + std::to_string(f.maxHp) + ") ";
            if (f.isAlive()) info += "Ult Charge: " + std::to_string(std::min(f.currentTurn, f.ultCooldown)) + "/" + std::to_string(f.ultCooldown);
            else info += " [DEAD]";

            if (i == battle.currentTurnIndex && f.isAlive()) {
                info = ">>> " + info + " <<<";
                statText.setFillColor(sf::Color::Yellow);
            } else {
                statText.setFillColor(f.isPlayer ? sf::Color::Cyan : sf::Color::Red);
            }

            statText.setString(info);
            
            if (f.isPlayer) {
                statText.setPosition({50.0f, playerYPos});
                playerYPos += 40.0f;
            } else {
                statText.setPosition({450.0f, enemyYPos});
                enemyYPos += 40.0f;
            }
            window.draw(statText);
        }

        sf::Text logText(font);
        logText.setCharacterSize(18);
        logText.setFillColor(sf::Color::White);
        float logY = 350.0f;
        
        for (const auto& log : battle.battleLogs) {
            logText.setString(log);
            logText.setPosition({50.0f, logY});
            window.draw(logText);
            logY += 25.0f;
        }

        if (!battle.isGameOver() && battle.fighters[battle.currentTurnIndex].isPlayer) {
            sf::Text guideText(font);
            guideText.setString("Player Turn: Press '1' to Attack, '2' to Ultimate");
            guideText.setCharacterSize(20);
            guideText.setFillColor(sf::Color::Green);
            guideText.setPosition({50.0f, 520.0f});
            window.draw(guideText);
        }

        std::cout << ">> [9] แสดงผลจอ (Display)" << std::endl;
        window.display();
    }

    return 0;
}
