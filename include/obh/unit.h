/*****************************************************
Create Date:        2024-11-25
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "./incl.h"
#include "../raylib/raylib.h"
#include "../raylib/raymath.h"

struct Unit {
    Vector3 pos, dir;
    float speed, speed_sprint, speed_fall, speed_fall_acc;
    Model* model;
    bool sprinting, falling;
};

typedef struct Unit Unit;

void unitMove(Unit* unit, Camera* cam);
void unitTurnLeft(Unit* unit, float deg);
void unitUpdate(Unit* unit);
void unitStop(Unit* unit);
