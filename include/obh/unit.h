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
    Vector3 position, direction;
    float movement_speed, sprint_factor, fall_velocity;
    bool sprinting, falling;
    Model* model;
};

struct UnitCamera {
    struct Unit* following;
    Camera camera;
    float off_x, off_y, off_z;
    float speed;
};

typedef struct Unit Unit;
typedef struct UnitCamera UnitCamera;

void unitMove(Unit* unit);
void unitTurnLeft(Unit* unit, float deg);
void unitUpdate(Unit* unit);
void unitStop(Unit* unit);
BoundingBox unitBoundingBox(Unit* unit);
void unitUpdateThirdPersonCamera(UnitCamera* cam);
void unitPollInputs(Unit* unit);
void unitCamPollInputs(UnitCamera* unit_cam);
Vector3 unitGetMovementVec(Unit* unit);

