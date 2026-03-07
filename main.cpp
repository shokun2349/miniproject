#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <algorithm>
#include "Gamemanager.h" 

struct CharacterSlot {
    ClassType type;
    sf::RectangleShape shape;
    sf::Text label;
    bool isSelected = false;

    CharacterSlot(const sf::Font& font) : label(font) {}
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Turn-Based RPG Mechanics");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("ARIAL.TTF")) {
        std::cout << "Error loading font! Please put ARIAL.TTF in the same directory." << std::endl;
        return -1;
    }

    GameManager game;
    bool turnActionCompleted = false;
    sf::Clock enemyTimer;
    bool isEnemyWaiting = false;

    // --- UI สำหรับหน้า Main Menu ---
    sf::RectangleShape playButton({200.0f, 60.0f});
    playButton.setPosition({300.0f, 250.0f});
    playButton.setFillColor(sf::Color(50, 150, 50)); 

    sf::RectangleShape quitButton({200.0f, 60.0f});
    quitButton.setPosition({300.0f, 350.0f});
    quitButton.setFillColor(sf::Color(150, 50, 50)); 

    // --- UI สำหรับหน้า Party Selection ---
    std::vector<std::string> classNames = {"KNIGHT", "ARCHER", "MAGE", "TANK", "SUPPORT", "SPECIAL"};
    std::vector<CharacterSlot> charButtons;
    std::vector<ClassType> selectedParty;

    for (int i = 0; i < 6; ++i) {
        CharacterSlot slot(font);
        slot.type = static_cast<ClassType>(i);
        slot.shape.setSize({150.0f, 50.0f});
        float xPos = (i % 2 == 0) ? 100.0f : 300.0f;
        float yPos = 150.0f + ((i / 2) * 80.0f);
        slot.shape.setPosition({xPos, yPos});
        slot.shape.setFillColor(sf::Color(50, 50, 150));
        
        slot.label.setFont(font);
        slot.label.setString(classNames[i]);
        slot.label.setCharacterSize(20);
        slot.label.setPosition({xPos + 10.0f, yPos + 10.0f});
        
        charButtons.push_back(slot);
    }

    sf::RectangleShape confirmButton({200.0f, 60.0f});
    confirmButton.setPosition({500.0f, 450.0f});
    confirmButton.setFillColor(sf::Color(100, 100, 100));

    sf::Text confirmText(font);
    confirmText.setString("CONFIRM");
    confirmText.setCharacterSize(24);
    confirmText.setFillColor(sf::Color::White);
    confirmText.setPosition({540.0f, 465.0f});

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // --- จัดการการคลิกเมาส์ ---
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos(static_cast<float>(mouseReleased->position.x), 
                                          static_cast<float>(mouseReleased->position.y));

                    if (game.state == MAIN_MENU) {
                        if (playButton.getGlobalBounds().contains(mousePos)) {
                            game.state = PARTY_SELECTION; 
                        }
                        else if (quitButton.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    }
                    else if (game.state == PARTY_SELECTION) {
                        for (auto& btn : charButtons) {
                            if (btn.shape.getGlobalBounds().contains(mousePos)) {
                                auto it = std::find(selectedParty.begin(), selectedParty.end(), btn.type);
                                if (it != selectedParty.end()) {
                                    selectedParty.erase(it);
                                    btn.isSelected = false;
                                    btn.shape.setFillColor(sf::Color(50, 50, 150));
                                } 
                                else if (selectedParty.size() < 3) {
                                    selectedParty.push_back(btn.type);
                                    btn.isSelected = true;
                                    btn.shape.setFillColor(sf::Color(50, 150, 50));
                                }
                            }
                        }

                        if (selectedParty.size() == 3) confirmButton.setFillColor(sf::Color(200, 150, 50));
                        else confirmButton.setFillColor(sf::Color(100, 100, 100));

                        if (selectedParty.size() == 3 && confirmButton.getGlobalBounds().contains(mousePos)) {
                            game.setPlayerTeam(selectedParty);
                            game.startStage(1); 
                        }
                    }
                }
            }

            // --- จัดการคีย์บอร์ด ---
            if (game.state == ANSWERING_MATH) {
                if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                    char32_t c = textEntered->unicode;
                    if ((c >= '0' && c <= '9') || c == '-') {
                        game.playerInputString += static_cast<char>(c);
                    }
                }
            }

            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                if (game.state == PLAYING && !game.currentBattle.isGameOver()) {
                    Character& currentFighter = game.currentBattle.fighters[game.currentBattle.currentTurnIndex];
                    if (currentFighter.isPlayer && currentFighter.isAlive()) {
                        Character* target = game.currentBattle.getTarget(true);
                        if (!target) continue;

                        if (keyReleased->code == sf::Keyboard::Key::Num1) {
                            game.triggerMathQuestion(false); 
                        } 
                        else if (keyReleased->code == sf::Keyboard::Key::Num2) {
                            if (currentFighter.canUseUlt()) {
                                game.triggerMathQuestion(true); 
                            } else {
                                game.currentBattle.addLog("Ultimate not ready!");
                            }
                        }
                    }
                }
                else if (game.state == ANSWERING_MATH) {
                    if (keyReleased->code == sf::Keyboard::Key::Enter) {
                        game.submitMathAnswer(); 
                        turnActionCompleted = true; 
                    } 
                    else if (keyReleased->code == sf::Keyboard::Key::Backspace) {
                        if (!game.playerInputString.empty()) {
                            game.playerInputString.pop_back(); 
                        }
                    }
                }
                else if (game.state == STAGE_CLEAR) {
                    if (keyReleased->code == sf::Keyboard::Key::Enter) {
                        game.proceedToNextStage();
                    }
                }
            }
        }

        // --- ระบบ Auto ศัตรู ---
        if (game.state == PLAYING && !game.currentBattle.isGameOver()) {
            Character& currentFighter = game.currentBattle.fighters[game.currentBattle.currentTurnIndex];
            if (!currentFighter.isPlayer && currentFighter.isAlive()) {
                if (!isEnemyWaiting) {
                    isEnemyWaiting = true;
                    enemyTimer.restart(); 
                }
                if (isEnemyWaiting && enemyTimer.getElapsedTime().asSeconds() >= 1.0f) {
                    Character* target = game.currentBattle.getTarget(false);
                    if (target) {
                        if (currentFighter.canUseUlt()) game.currentBattle.executeUltimate(currentFighter, *target);
                        else game.currentBattle.executeAttack(currentFighter, *target);
                        turnActionCompleted = true;
                    }
                    isEnemyWaiting = false; 
                }
            }
        }

        if (turnActionCompleted) {
            game.currentBattle.nextTurn();
            game.updateBattleState(); 
            turnActionCompleted = false;
        }

        // ==========================================
        // --- ส่วนวาดกราฟิกลงจอ (DRAWING SECTION) ---
        // ==========================================
        window.clear(sf::Color(30, 30, 30));

        // 1. วาดหน้า Main Menu
        if (game.state == MAIN_MENU) {
            sf::Text titleText(font);
            titleText.setString("EPIC MATH RPG");
            titleText.setCharacterSize(50);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition({210.0f, 100.0f});
            window.draw(titleText);

            window.draw(playButton);
            window.draw(quitButton);

            sf::Text playText(font);
            playText.setString("PLAY");
            playText.setCharacterSize(24);
            playText.setFillColor(sf::Color::White);
            playText.setPosition({365.0f, 265.0f});
            window.draw(playText);

            sf::Text quitText(font);
            quitText.setString("QUIT");
            quitText.setCharacterSize(24);
            quitText.setFillColor(sf::Color::White);
            quitText.setPosition({365.0f, 365.0f});
            window.draw(quitText);
        }
        // 2. วาดหน้า เลือกตัวละคร
        else if (game.state == PARTY_SELECTION) {
            sf::Text title(font);
            title.setString("Select Your Party (Choose 3)");
            title.setCharacterSize(30);
            title.setFillColor(sf::Color::White);
            title.setPosition({200.0f, 50.0f});
            window.draw(title);

            for (auto& btn : charButtons) {
                window.draw(btn.shape);
                window.draw(btn.label);
            }

            sf::Text statusText(font);
            statusText.setCharacterSize(22);
            statusText.setPosition({500.0f, 150.0f});
            statusText.setFillColor(sf::Color::White);
            std::string partyStr = "Current Party:\n";
            for (auto p : selectedParty) {
                partyStr += "- " + classNames[static_cast<int>(p)] + "\n";
            }
            statusText.setString(partyStr);
            window.draw(statusText);

            window.draw(confirmButton);
            window.draw(confirmText);
        }
        // 3. วาดหน้าฉากต่อสู้
        else {
            // ==========================================
            // แสดงเลขด่าน (Stage) ด้านบนตรงกลาง
            // ==========================================
            sf::Text stageText(font);
            stageText.setCharacterSize(28);
            stageText.setFillColor(sf::Color::White);
            stageText.setStyle(sf::Text::Bold);
            stageText.setString("--- STAGE " + std::to_string(game.currentStage) + " ---");
            stageText.setPosition({320.0f, 10.0f}); 
            window.draw(stageText);

            sf::Text statText(font);
            statText.setCharacterSize(16);
            
            // ขยับ Y ลงมาเพื่อไม่ให้ข้อความไปทับกับคำว่า STAGE ด้านบน
            float playerYPos = 70.0f;
            float enemyYPos = 70.0f;
            
            for (size_t i = 0; i < game.currentBattle.fighters.size(); ++i) {
                auto& f = game.currentBattle.fighters[i];
                
                float xPos = f.isPlayer ? 50.0f : 450.0f;
                float yPos = f.isPlayer ? playerYPos : enemyYPos;

                std::string info = f.name + " (HP: " + std::to_string(f.hp) + "/" + std::to_string(f.maxHp) + ") ";
                if (f.isAlive()) info += "Ult Charge: " + std::to_string(std::min(f.currentTurn, f.ultCooldown)) + "/" + std::to_string(f.ultCooldown);
                else info += " [DEAD]";

                if (i == game.currentBattle.currentTurnIndex && f.isAlive() && game.state == PLAYING) {
                    info = ">>> " + info + " <<<";
                    statText.setFillColor(sf::Color::Yellow);
                } else {
                    statText.setFillColor(f.isPlayer ? sf::Color::Cyan : sf::Color::Red);
                }

                statText.setString(info);
                statText.setPosition({xPos, yPos});
                window.draw(statText);

                // ==========================================
                // วาดหลอดเลือด (Health Bar)
                // ==========================================
                if (f.isAlive()) {
                    float barWidth = 200.0f; 
                    float barHeight = 12.0f; 
                    
                    sf::RectangleShape barBack(sf::Vector2f(barWidth, barHeight));
                    barBack.setFillColor(sf::Color(50, 50, 50));
                    barBack.setPosition({xPos, yPos + 25.0f}); 
                    window.draw(barBack);

                    float hpPercentage = static_cast<float>(f.hp) / f.maxHp;
                    if (hpPercentage < 0.0f) hpPercentage = 0.0f; 
                    
                    sf::RectangleShape barFront(sf::Vector2f(barWidth * hpPercentage, barHeight));
                    
                    if (hpPercentage > 0.5f) barFront.setFillColor(sf::Color::Green);
                    else if (hpPercentage > 0.2f) barFront.setFillColor(sf::Color::Yellow);
                    else barFront.setFillColor(sf::Color::Red);

                    barFront.setPosition({xPos, yPos + 25.0f});
                    window.draw(barFront);
                }
                
                // อัปเดตตำแหน่ง Y สำหรับตัวละครตัวถัดไป
                if (f.isPlayer) playerYPos += 60.0f; 
                else enemyYPos += 60.0f;  
            }

            sf::Text logText(font);
            logText.setCharacterSize(18);
            logText.setFillColor(sf::Color::White);
            float logY = 350.0f;
            for (const auto& log : game.currentBattle.battleLogs) {
                logText.setString(log);
                logText.setPosition({50.0f, logY});
                window.draw(logText);
                logY += 25.0f;
            }

            // ==========================================
            // กล่องโจทย์คณิตศาสตร์
            // ==========================================
            if (game.state == ANSWERING_MATH) {
                sf::RectangleShape mathBox({550.0f, 250.0f});
                mathBox.setFillColor(sf::Color(20, 50, 100, 230)); 
                mathBox.setPosition({125.0f, 120.0f}); 
                mathBox.setOutlineThickness(3.0f);
                mathBox.setOutlineColor(sf::Color::White);
                window.draw(mathBox);

                sf::Text mathUI(font);
                mathUI.setCharacterSize(24);
                mathUI.setFillColor(sf::Color::White);
                mathUI.setString("Solve to Attack!\n\n" + game.mathQuestion + "\n\nAnswer: " + game.playerInputString + "_");
                mathUI.setPosition({150.0f, 150.0f}); 
                window.draw(mathUI);
            }

            sf::Text guideText(font);
            guideText.setCharacterSize(20);
            guideText.setPosition({50.0f, 520.0f});

            if (game.state == PLAYING && !game.currentBattle.isGameOver() && game.currentBattle.fighters[game.currentBattle.currentTurnIndex].isPlayer) {
                guideText.setString("Player Turn: Press '1' to Attack, '2' to Ultimate");
                guideText.setFillColor(sf::Color::Green);
                window.draw(guideText);
            } else if (game.state == STAGE_CLEAR) {
                guideText.setString("Stage Clear! Press 'Enter' to proceed.");
                guideText.setFillColor(sf::Color::Yellow);
                window.draw(guideText);
            } else if (game.state == GAME_OVER) {
                guideText.setString("GAME OVER! Your party was wiped out.");
                guideText.setFillColor(sf::Color::Red);
                window.draw(guideText);
            } else if (game.state == GAME_WON) {
                guideText.setString("VICTORY! You have defeated the Demon King!");
                guideText.setFillColor(sf::Color::Yellow);
                window.draw(guideText);
            }
        }

        window.display();
    }

    return 0;
}