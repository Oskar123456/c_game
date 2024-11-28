/*****************************************************
Create Date:        2024-11-25
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "../include/obh/unit.h"

Vector3 unitGetMovementVec(Unit* unit)
{
    Vector3 dir = Vector3Scale(unit->direction, unit->movement_speed);
    if (unit->sprinting)
        return Vector3Scale(dir, unit->sprint_factor);
    else
        return Vector3Scale(dir, unit->sprint_factor);
}

void unitMove(Unit* unit)
{
    Vector3 dir = unitGetMovementVec(unit);
    if (unit->sprinting)
        dir = Vector3Scale(dir, unit->sprint_factor);
    unit->position = Vector3Add(unit->position, dir);
}

void unitTurnLeft(Unit* unit, float deg)
{
    unit->direction = Vector3RotateByAxisAngle(unit->direction, (Vector3) {0, 1, 0}, deg);
}

void unitUpdate(Unit* unit)
{
    if (unit->falling) {
        unit->fall_velocity += 0.014;
        unit->direction.y -= unit->fall_velocity;
    }
    unitMove(unit);
    unit->direction.x = 0;
    unit->direction.z = 0;
}

void unitStop(Unit* unit)
{
    unit->falling = false;
    unit->direction.y = 0;
    unit->fall_velocity = 0;
}

BoundingBox unitBoundingBox(Unit* unit)
{
    BoundingBox bb = GetModelBoundingBox(*unit->model);
    bb.min = Vector3Add(bb.min, unit->position);
    bb.max = Vector3Add(bb.max, unit->position);
    return bb;
}

void unitUpdateThirdPersonCamera(UnitCamera* cam)
{
    cam->camera.target = cam->following->position;
    cam->camera.position = (Vector3) {
        cam->camera.target.x + cam->off_x,
        cam->camera.target.y + cam->off_y,
        cam->camera.target.z + cam->off_z,
    };
}

void unitPollInputs(Unit* unit)
{
    if (IsKeyDown(KEY_LEFT)) {
        unit->direction.x += unit->movement_speed;
        unit->direction.z += unit->movement_speed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        unit->direction.x += -unit->movement_speed;
        unit->direction.z += -unit->movement_speed;
    }
    if (IsKeyDown(KEY_UP)) {
        unit->direction.x += -unit->movement_speed;
        unit->direction.z += unit->movement_speed;
    }
    if (IsKeyDown(KEY_DOWN)) {
        unit->direction.x += unit->movement_speed;
        unit->direction.z += -unit->movement_speed;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT))
        unit->sprinting = true;
    else
        unit->sprinting = false;

    if (IsKeyPressed(KEY_SPACE) && !unit->falling) {
        unit->falling = true;
        unit->direction.y = 0.8;
    }

}

void unitCamPollInputs(UnitCamera* unit_cam)
{
    if (IsKeyDown(KEY_S)) {
        unit_cam->off_x += unit_cam->speed;
        unit_cam->off_y += unit_cam->speed;
        unit_cam->off_z -= unit_cam->speed;
    }

    if (IsKeyDown(KEY_W)) {
        unit_cam->off_x -= unit_cam->speed;
        unit_cam->off_y -= unit_cam->speed;
        unit_cam->off_z += unit_cam->speed;
    }

    if (IsKeyDown(KEY_A)) {
        unit_cam->off_x -= unit_cam->speed;
        unit_cam->off_y += unit_cam->speed;
    }

    if (IsKeyDown(KEY_D)) {
        unit_cam->off_x += unit_cam->speed;
        unit_cam->off_y -= unit_cam->speed;
    }

    if (IsKeyPressed(KEY_R)) {
        unit_cam->off_x = 50;
        unit_cam->off_y = 50;
        unit_cam->off_z = -50;
    }
}
