#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include "Gamemanager.h" 

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

    // --- สร้างปุ่ม Play ---
    sf::RectangleShape playButton({200.0f, 60.0f});
    playButton.setPosition({300.0f, 250.0f});
    playButton.setFillColor(sf::Color(50, 150, 50)); // สีเขียว

    // --- สร้างปุ่ม Quit ---
    sf::RectangleShape quitButton({200.0f, 60.0f});
    quitButton.setPosition({300.0f, 350.0f});
    quitButton.setFillColor(sf::Color(150, 50, 50)); // สีแดง

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // --- เช็คการคลิกเมาส์ สำหรับหน้า Main Menu ---
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    // ดึงตำแหน่งเมาส์แปลงเป็น float เพื่อไปเช็คการชนกับกล่อง
                    sf::Vector2f mousePos(static_cast<float>(mouseReleased->position.x), 
                                          static_cast<float>(mouseReleased->position.y));

                    if (game.state == MAIN_MENU) {
                        // ถ้าคลิกโดนปุ่ม Play
                        if (playButton.getGlobalBounds().contains(mousePos)) {
                            game.startStage(1); // เริ่มด่าน 1 และเปลี่ยนสถานะเป็น PLAYING
                        }
                        // ถ้าคลิกโดนปุ่ม Quit
                        else if (quitButton.getGlobalBounds().contains(mousePos)) {
                            window.close();
                        }
                    }
                }
            }

            // รับตัวหนังสือตอนพิมพ์คำตอบคณิตศาสตร์
            if (game.state == ANSWERING_MATH) {
                if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                    char32_t c = textEntered->unicode;
                    if ((c >= '0' && c <= '9') || c == '-') {
                        game.playerInputString += static_cast<char>(c);
                    }
                }
            }

            // รับปุ่มกดจากคีย์บอร์ด
            if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                
                if (game.state == PLAYING && !game.currentBattle.isGameOver()) {
                    Character& currentFighter = game.currentBattle.fighters[game.currentBattle.currentTurnIndex];
                    
                    if (currentFighter.isPlayer && currentFighter.isAlive()) {
                        Character* target = game.currentBattle.getTarget(true);
                        if (!target) continue;

                        if (keyReleased->code == sf::Keyboard::Key::Num1) {
                            game.triggerMathQuestion(false); // เรียกโจทย์คณิต
                        } 
                        else if (keyReleased->code == sf::Keyboard::Key::Num2) {
                            if (currentFighter.canUseUlt()) {
                                game.triggerMathQuestion(true); // เรียกโจทย์คณิต
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

        // ระบบ Auto ศัตรู
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

        // เปลี่ยนเทิร์น
        if (turnActionCompleted) {
            game.currentBattle.nextTurn();
            game.updateBattleState(); 
            turnActionCompleted = false;
        }

        // --- เริ่มเคลียร์หน้าจอและวาดภาพ ---
        window.clear(sf::Color(30, 30, 30));

        // ถ้าระบบอยู่ในหน้า MAIN_MENU ให้วาดแค่เมนู
        if (game.state == MAIN_MENU) {
            // ชื่อเกม
            sf::Text titleText(font);
            titleText.setString("EPIC MATH RPG");
            titleText.setCharacterSize(50);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition({210.0f, 100.0f});
            window.draw(titleText);

            // วาดกล่องปุ่ม
            window.draw(playButton);
            window.draw(quitButton);

            // ข้อความบนปุ่ม
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
        // ถ้าไม่อยู่ในเมนูหลัก ก็วาดฉากต่อสู้ตามปกติ
        else {
            sf::Text statText(font);
            statText.setCharacterSize(16);
            float playerYPos = 50.0f;
            float enemyYPos = 50.0f;
            
            for (size_t i = 0; i < game.currentBattle.fighters.size(); ++i) {
                auto& f = game.currentBattle.fighters[i];
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
            
            for (const auto& log : game.currentBattle.battleLogs) {
                logText.setString(log);
                logText.setPosition({50.0f, logY});
                window.draw(logText);
                logY += 25.0f;
            }

            if (game.state == ANSWERING_MATH) {
                sf::RectangleShape mathBox({400.0f, 150.0f});
                mathBox.setFillColor(sf::Color(20, 50, 100, 230)); 
                mathBox.setPosition({200.0f, 150.0f});
                mathBox.setOutlineThickness(3.0f);
                mathBox.setOutlineColor(sf::Color::White);
                window.draw(mathBox);

                sf::Text mathUI(font);
                mathUI.setCharacterSize(24);
                mathUI.setFillColor(sf::Color::White);
                mathUI.setString("Solve to Attack!\n\n" + game.mathQuestion + "\n\nAnswer: " + game.playerInputString + "_");
                mathUI.setPosition({220.0f, 170.0f});
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
                guideText.setString("Stage Clear! Press 'Enter' to proceed to Stage 2.");
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