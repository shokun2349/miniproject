#pragma once
#include <string>

// กำหนดประเภทตัวละคร
enum ClassType { KNIGHT, ARCHER, MAGE, TANK, SUPPORT, SPECIAL };

class Character {
public:
    std::string name;
    ClassType type;
    int hp, maxHp, atk, magic, def, speed;
    bool isPlayer;

    // ระบบท่าไม้ตาย
    int ultCooldown;
    int currentTurn;

    Character(std::string n, ClassType t, int h, int a, int m, int d, int s, bool player) 
        : name(n), type(t), hp(h), maxHp(h), atk(a), magic(m), def(d), speed(s), isPlayer(player), currentTurn(0) {

        // กำหนดความไวในการใช้ท่าไม้ตาย
        if (type == SUPPORT) ultCooldown = 2;
        else if (type == MAGE || type == ARCHER) ultCooldown = 3;
        else ultCooldown = 4;
    }

    bool isAlive() const { return hp > 0; }
    bool canUseUlt() const { return currentTurn >= ultCooldown; }

    void takeDamage(int damage) {
        hp -= damage;
        if (hp < 0) hp = 0;
    }

    void heal(int amount) {
        hp += amount;
        if (hp > maxHp) hp = maxHp;
    }
};