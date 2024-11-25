/*****************************************************
Create Date:        2024-11-25
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "../include/obh/unit.h"

void unitMove(Unit* unit, Camera* cam)
{
    if (unit->sprinting) {
        unit->pos = Vector3Add(unit->pos, Vector3Scale(unit->dir, unit->speed_sprint));
        cam->position = Vector3Add(cam->position, Vector3Scale(unit->dir, unit->speed_sprint));
    }
    else {
        unit->pos = Vector3Add(unit->pos, Vector3Scale(unit->dir, unit->speed));
        cam->position = Vector3Add(cam->position, Vector3Scale(unit->dir, unit->speed));
    }
}

void unitTurnLeft(Unit* unit, float deg)
{
    unit->dir = Vector3RotateByAxisAngle(unit->dir, (Vector3) {0, 1, 0}, deg);
}