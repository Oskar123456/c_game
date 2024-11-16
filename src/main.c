/*****************************************************
Create Date:        2024-11-12
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "../include/incl.h"
#include "../include/db.h"
#include "../include/c_log.h"
#include "../include/util.h"
#include "../include/world.h"
#include "../include/glad.h"
#include "../include/raylib/raylib.h"
#include "../include/raylib/raymath.h"
#include "../include/raylib/rcamera.h"

#define STB_DS_IMPLEMENTATION
#include "../include/stb/stb_ds.h"


static float CAMERA_MOVE_SPEED = 45.0f;

#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f

float CAMERA_OFF_X = 50.0f;
float CAMERA_OFF_Y = 50.0f;
float CAMERA_OFF_Z = -50.0f;

typedef enum { DIR_DOWN_X, DIR_UP_X,  DIR_DOWN_Z, DIR_UP_Z, DIR_UP_Y, DIR_DOWN_Y } DIRECTION;

struct Pixel { u8 r, g, b, a; };
struct Unit {
    union {
        Vector3 pos;
        struct { float x, y, z; };
    };

    Model *models;
    int   models_n, model_current;
    int   direction, moving, in_air;
    int   direction_when_jump_x, direction_when_jump_z;
    bool  is_sprinting;
    float direction_angle;
    float fall_speed, movement_speed, movement_speed_sprint, movement_speed_actual;
    Vector3 dir_vert, dir_hori, dir_up, dir_vec;
};

void UpdateCameraCustom(Camera *camera, int mode);

static int screenWidth = 800;
static int screenHeight = 600;
static Camera camera;
static bool fullscreen;
static bool paused;
struct Unit player_unit;

void adjustCamera()
{
    camera.position = (Vector3){ CAMERA_OFF_X + player_unit.x,
                     CAMERA_OFF_Y + player_unit.y,
                     CAMERA_OFF_Z + player_unit.z };
    camera.target = player_unit.pos;
}

BoundingBox getBB(Vector3 pos)
{
    return (BoundingBox) { pos, (Vector3) {pos.x + 1, pos.y + 1, pos.z + 1} };
}

void unitMove(struct Unit* unit, DIRECTION dir)
{

    if (dir == DIR_UP_X) {
        unit->pos = Vector3Subtract(unit->pos,
                Vector3Scale(unit->dir_hori, unit->movement_speed_actual));
        if (worldCollisionTestBox(getBB(unit->pos))) {
            unit->pos = Vector3Add(unit->pos,
                    Vector3Scale(unit->dir_hori, unit->movement_speed_actual));
        }
    }
    if (dir == DIR_DOWN_X) {
        unit->pos = Vector3Add(unit->pos,
                Vector3Scale(unit->dir_hori, unit->movement_speed_actual));
        if (worldCollisionTestBox(getBB(unit->pos))) {
            unit->pos = Vector3Subtract(unit->pos,
                    Vector3Scale(unit->dir_hori, unit->movement_speed_actual));
        }
    }

    if (dir == DIR_DOWN_Y || dir == DIR_UP_Y) {
        if (unit->in_air)
            unit->fall_speed = min(unit->fall_speed + 0.06, 1.5);
        unit->pos.y -= unit->dir_up.y * unit->fall_speed;
        if (worldCollisionTestBox(getBB(unit->pos))) {
            unit->pos.y += unit->dir_up.y * unit->fall_speed;
            unit->in_air = false;
            unit->direction_when_jump_x = -1;
            unit->direction_when_jump_z = -1;
        }
        else {
            unitMove(unit, unit->direction_when_jump_x);
            unitMove(unit, unit->direction_when_jump_z);
        }
    }

    if (dir == DIR_UP_Z) {
        unit->pos = Vector3Add(unit->pos,
                Vector3Scale(unit->dir_vert, unit->movement_speed_actual));
        if (worldCollisionTestBox(getBB(unit->pos))) {
        //unit->pos = Vector3Subtract(unit->pos,
        //        Vector3Scale(unit->dir_vert, unit->movement_speed_actual));
        }
    }
    if (dir == DIR_DOWN_Z) {
        unit->pos = Vector3Subtract(unit->pos,
                Vector3Scale(unit->dir_vert, unit->movement_speed_actual));
        if (worldCollisionTestBox(getBB(unit->pos))) {
        unit->pos = Vector3Add(unit->pos,
                Vector3Scale(unit->dir_vert, unit->movement_speed_actual));
        }
    }

    adjustCamera();
}

void pollWindowEvents()
{
    if (IsWindowResized() && !IsWindowFullscreen()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
    }
}

void pollKeys()
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        paused = !paused;
//        if (paused)
//        else
    }
    if (IsKeyPressed(KEY_F1))
        SetExitKey(KEY_F1);

    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
        screenWidth = 1920;
        screenHeight = 1080;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT))
        player_unit.movement_speed_actual = player_unit.movement_speed_sprint;
    else
        player_unit.movement_speed_actual = player_unit.movement_speed;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))
        player_unit.moving = 1;
    else
        player_unit.moving = 0;

    if (!player_unit.in_air) {
        if (IsKeyDown(KEY_UP)) {
            player_unit.direction_angle = 0;
            player_unit.direction = DIR_UP_Z;
            unitMove(&player_unit, DIR_UP_Z);
        }
        if (IsKeyDown(KEY_DOWN)) {
            player_unit.direction_angle = 180;
            player_unit.direction = DIR_DOWN_Z;
            unitMove(&player_unit, DIR_DOWN_Z);
        }
        if (IsKeyDown(KEY_LEFT)) {
            player_unit.direction_angle = 90;
            player_unit.direction = DIR_DOWN_X;
            unitMove(&player_unit, DIR_DOWN_X);
        }
        if (IsKeyDown(KEY_RIGHT)) {
            player_unit.direction_angle = 270;
            player_unit.direction = DIR_UP_X;
            unitMove(&player_unit, DIR_UP_X);
        }
    }

    if (IsKeyPressed(KEY_SPACE) && !player_unit.in_air) {
        player_unit.in_air = true;
        player_unit.fall_speed = -0.5;
        if (IsKeyDown(KEY_DOWN))
            player_unit.direction_when_jump_z = DIR_DOWN_Z;
        if (IsKeyDown(KEY_UP))
            player_unit.direction_when_jump_z = DIR_UP_Z;
        if (IsKeyDown(KEY_RIGHT))
            player_unit.direction_when_jump_x = DIR_UP_X;
        if (IsKeyDown(KEY_LEFT))
            player_unit.direction_when_jump_x = DIR_DOWN_X;
        c_log_info(LOG_TAG, "jump (%d %d)",
                player_unit.direction_when_jump_x, player_unit.direction_when_jump_z);
    }
}


int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;
    /* initialization */
    srand(time(NULL));
    c_log_init(stderr, LOG_LEVEL_SUCCESS);

    sds s = sdscatprintf(sdsempty(), "is in working? %s", "yes");
    c_log_success(LOG_TAG, s);

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //DisableCursor();
    SetExitKey(0);

    /* game stuff begins */
    worldInit();

    Mesh player_mesh = GenMeshPlane(1, 1, 1, 1);

    Image scarfy_img = LoadImage("resources/images/scarfy.png");
    Image scarfy_sprites[12];
    Image scarfy_meshes[12];
    Texture2D scarfy_textures[12];
    Model scarfy_models[12];
    for (int i = 0; i < 6; ++i) {
        scarfy_sprites[i] = ImageFromImage(scarfy_img,
                (Rectangle) {
                scarfy_img.width / (float)6 * i,
                0,
                scarfy_img.width / (float)6, scarfy_img.height});
        ImageFlipVertical(&scarfy_sprites[i]);
        scarfy_textures[i] = LoadTextureFromImage(scarfy_sprites[i]);
        scarfy_models[i] = LoadModelFromMesh(player_mesh);
        scarfy_models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = scarfy_textures[i];

        Matrix rotation_y = MatrixRotateY(-M_PI / 4);
        Matrix rotation_x = MatrixRotate((Vector3){ 1, 0, 1 }, -M_PI / 4);
        scarfy_models[i].transform = MatrixMultiply(rotation_y, rotation_x);
    }

    for (int i = 6; i < 12; ++i) {
        scarfy_sprites[i] = ImageFromImage(scarfy_img,
                (Rectangle) {
                scarfy_img.width / (float)6 * i,
                0,
                scarfy_img.width / (float)6, scarfy_img.height});
        ImageFlipVertical(&scarfy_sprites[i]);
        ImageFlipHorizontal(&scarfy_sprites[i]);
        scarfy_textures[i] = LoadTextureFromImage(scarfy_sprites[i]);
        scarfy_models[i] = LoadModelFromMesh(player_mesh);
        scarfy_models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = scarfy_textures[i];

        Matrix rotation_y = MatrixRotateY(-M_PI / 4);
        Matrix rotation_x = MatrixRotate((Vector3){ 1, 0, 1 }, -M_PI / 4);
        scarfy_models[i].transform = MatrixMultiply(rotation_y, rotation_x);
    }

    player_unit = (struct Unit) {
        .models = scarfy_models,
        .models_n = 6,
        .movement_speed = 0.2,
        .movement_speed_sprint = 0.4,
        .y = 50,
        .in_air = true,
    };

    camera.target = player_unit.pos;
    camera.position = (Vector3){ CAMERA_OFF_X, CAMERA_OFF_Y + player_unit.y, CAMERA_OFF_Z };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 10.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    player_unit.dir_hori = Vector3Normalize(Vector3CrossProduct(
                Vector3Subtract(camera.position, player_unit.pos), camera.up));
    player_unit.dir_vert = Vector3Normalize(Vector3CrossProduct(
                player_unit.dir_hori, camera.up));
    player_unit.dir_up = Vector3Normalize(camera.up);

    /* game stuff ends */


    u64 frame_number = 0;
    int anim_frame_time = 10;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Events
        //----------------------------------------------------------------------------------
        pollKeys();
        pollWindowEvents();
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------
        UpdateCameraCustom(&camera, CAMERA_FREE);
        if (frame_number % anim_frame_time == 0)
            player_unit.model_current = (player_unit.model_current + 1) % player_unit.models_n;
        if (player_unit.in_air)
            unitMove(&player_unit, DIR_DOWN_Y);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLUE);

            BeginMode3D(camera);

                worldRender(player_unit.pos);

                if (player_unit.moving)
                    DrawModelEx(
                            player_unit.models[player_unit.model_current + (player_unit.direction % 2) * player_unit.models_n],
                            player_unit.pos,
                            (Vector3) { 1, 1, 1 }, 0,
                            (Vector3) { 1, 1, 1 }, WHITE);
                else
                    DrawModelEx(
                            player_unit.models[(player_unit.direction % 2) * player_unit.models_n + 2],
                            player_unit.pos,
                            (Vector3) { 1, 1, 1 }, 0,
                            (Vector3) { 1, 1, 1 },
                            WHITE);

                DrawCubeWires(player_unit.pos, 1, 1, 1, YELLOW);
                worldDrawClosestCubes(getBB(player_unit.pos));

                DrawGrid(20, 1.0f);

            EndMode3D();

            DrawFPS(10, 10);
            int x, z, cubex, cubez;;
            worldMapToChunkCoords(player_unit.pos, &x, &z);
            worldMapToCubeCoords(player_unit.pos, &cubex, &cubez);
            sds player_info = sdscatprintf(sdsempty(), "Player pos: x:%.2fy:%.2fz:%.2f (chunk %d %d, cube %d %d)",
                    player_unit.x, player_unit.y, player_unit.z, x, z, cubex, cubez);
            sds player_info_2 = sdscatprintf(sdsempty(), "falling: %d (dir: x%f y:%f z:%f)",
                    player_unit.in_air,
                    player_unit.dir_vec.x,
                    player_unit.dir_vec.y,
                    player_unit.dir_vec.z);
            sds cam_info = sdscatprintf(sdsempty(), "Camera pos: x:%.2fy:%.2fz:%.2f",
                    camera.position.x, camera.position.y, camera.position.z);
            sds world_info = worldGetInfo();
            int fontsize = 16;
            int font_y = 40;
            DrawText(player_info, 4, font_y, fontsize, BLACK);
            DrawText(player_info_2, 4, font_y + 1 * fontsize + 2, fontsize, BLACK);
            DrawText(cam_info, 4, font_y + 2 * fontsize + 2, fontsize, BLACK);
            DrawText(world_info, 4, font_y + 3 * fontsize + 2, fontsize, BLACK);
            sdsfree(player_info);
            sdsfree(player_info_2);
            sdsfree(cam_info);
            sdsfree(world_info);

        EndDrawing();
        //----------------------------------------------------------------------------------
        frame_number++;
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadTexture(texture);     // Unload texture
    //UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------


    return exit_code == EXIT_FAILURE ? exit_code : EXIT_SUCCESS;
}


void UpdateCameraCustom(Camera *camera, int mode)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FREE) || (mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = false;

    // Camera speeds based on frame time
    float cameraMoveSpeed = CAMERA_MOVE_SPEED*GetFrameTime();
    float cameraRotationSpeed = CAMERA_ROTATION_SPEED*GetFrameTime();
    float cameraPanSpeed = CAMERA_PAN_SPEED*GetFrameTime();
    float cameraOrbitalSpeed = CAMERA_ORBITAL_SPEED*GetFrameTime();

    if (mode == CAMERA_CUSTOM) {}
    else if (mode == CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(GetCameraUp(camera), cameraOrbitalSpeed);
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    }
    else
    {
        // Camera rotation
        //if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        //if (IsKeyDown(KEY_UP)) CameraPitch(camera, cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        //if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -cameraRotationSpeed, rotateAroundTarget);
        //if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, cameraRotationSpeed, rotateAroundTarget);
        //if (IsKeyDown(KEY_Q)) CameraRoll(camera, -cameraRotationSpeed);
        //if (IsKeyDown(KEY_E)) CameraRoll(camera, cameraRotationSpeed);

        // Camera movement
        // Camera pan (for CAMERA_FREE)
        if ((mode == CAMERA_FREE) && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)))
        {
            const Vector2 mouseDelta = GetMouseDelta();
            if (mouseDelta.x > 0.0f) CameraMoveRight(camera, cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.x < 0.0f) CameraMoveRight(camera, -cameraPanSpeed, moveInWorldPlane);
            if (mouseDelta.y > 0.0f) CameraMoveUp(camera, -cameraPanSpeed);
            if (mouseDelta.y < 0.0f) CameraMoveUp(camera, cameraPanSpeed);
        }
        else
        {
            // Mouse support
            //CameraYaw(camera, -mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            //CameraPitch(camera, -mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);
        }

        // Keyboard support
        if (IsKeyDown(KEY_W))
            CameraMoveForward(camera, 0.5, false);
        if (IsKeyDown(KEY_S))
            CameraMoveForward(camera, -0.5, false);

        //if (IsKeyDown(KEY_D)) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);

        // Gamepad movement
        if (IsGamepadAvailable(0))
        {
            // Gamepad controller support
            CameraYaw(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)*2)*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)*2)*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);
        }

        if (mode == CAMERA_FREE)
        {
            if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, cameraMoveSpeed);
            if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -cameraMoveSpeed);
        }
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL) || (mode == CAMERA_FREE))
    {
        // Zoom target distance
        CameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT)) CameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD)) CameraMoveToTarget(camera, -2.0f);
    }
}

