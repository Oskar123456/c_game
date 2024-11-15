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


static float CAMERA_MOVE_SPEED = 45.0f;

#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f

struct Pixel { u8 r, g, b, a; };
struct Unit {
    union {
        Vector3 pos;
        struct { float x, y, z; };
    };

    Model *models;
    int    models_n, model_current;
    int    direction, moving;
    float movement_speed;
    float movement_speed_sprint;
    float direction_angle;
};

void UpdateCameraCustom(Camera *camera, int mode);

static int screenWidth = 800;
static int screenHeight = 600;
static Camera camera;
static bool fullscreen;
static bool paused;
struct Unit player_unit;

void unitMove(struct Unit* unit, float x, float y, float z)
{
    unit->x += x;
    unit->y += y;
    unit->z += z;
    camera.position.x += x;
    camera.position.y += y;
    camera.position.z += z;
    camera.target = unit->pos;
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
        //if (paused)
           //DisableCursor();
        //else
           //EnableCursor();
    }
    if (IsKeyPressed(KEY_F1))
        SetExitKey(KEY_F1);

    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
        screenWidth = 1920;
        screenHeight = 1080;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT))
        CAMERA_MOVE_SPEED = 145.0f;
    else
        CAMERA_MOVE_SPEED = 45.0f;

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))
        player_unit.moving = 1;
    else
        player_unit.moving = 0;

    if (IsKeyDown(KEY_UP)) {
        player_unit.direction_angle = 0;
        if (!IsKeyDown(KEY_LEFT_SHIFT))
            unitMove(&player_unit, 0, 0, player_unit.movement_speed);
        else
            unitMove(&player_unit, 0, 0, player_unit.movement_speed_sprint);
    }
    if (IsKeyDown(KEY_DOWN)) {
        player_unit.direction_angle = 180;
        if (!IsKeyDown(KEY_LEFT_SHIFT))
            unitMove(&player_unit, 0, 0, -player_unit.movement_speed);
        else
            unitMove(&player_unit, 0, 0, -player_unit.movement_speed_sprint);
    }
    if (IsKeyDown(KEY_LEFT)) {
        player_unit.direction_angle = 90;
        player_unit.direction = 0;
        if (!IsKeyDown(KEY_LEFT_SHIFT))
            unitMove(&player_unit, player_unit.movement_speed, 0, 0);
        else
            unitMove(&player_unit, player_unit.movement_speed_sprint, 0, 0);
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player_unit.direction_angle = 270;
        player_unit.direction = 1;
        if (!IsKeyDown(KEY_LEFT_SHIFT))
            unitMove(&player_unit, -player_unit.movement_speed, 0, 0);
        else
            unitMove(&player_unit, -player_unit.movement_speed_sprint, 0, 0);
    }
}


int main(int argc, char *argv[])
{

    dict_uint_t float_freq_map;
    dict_uint_init(float_freq_map);

    int n_exps = 4, n_mantissa = 4;
    for (int i = 0; i < n_exps; ++i) {
        printf("\n");
        for (int j = 0; j < 1 << n_mantissa; j++) {
            float val = (1 << i);
            float val_mantissa = 1;
            float exp = n_mantissa;
            for (int k = 0; k < n_mantissa; ++k) {
                val_mantissa += ((j >> k) & 1) * (1 / pow(2, exp--));
            }
            val *= val_mantissa;
            printf("%f, ", val);
            if (dict_uint_get(float_freq_map, (int) val) == NULL)
                dict_uint_set_at(float_freq_map, (int) val, 0);
            dict_uint_set_at(float_freq_map, (int) val,
                    *dict_uint_get(float_freq_map, (int) val) + 1);
        }
        printf("\n");
    }

    dict_uint_out_str(stdout, float_freq_map);
    printf("\n");


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

    camera.position = (Vector3){ 0.0f, 150.0f, -150.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 10.0f;
    camera.projection = CAMERA_PERSPECTIVE;

//    int n_meshes = 100, mesh_size = 100;
//    Model models[n_meshes];
//    Vector3 model_positions[n_meshes];
//
//    for (int i = 0; i < n_meshes; ++i) {
//        int x_coord = (i % 10) * mesh_size;
//        int z_coord = (i / 10) * mesh_size;
//        Image image = GenImagePerlinNoise(mesh_size, mesh_size, x_coord, z_coord, 1);
//        Texture2D texture = LoadTextureFromImage(image);
//
//        Mesh mesh = GenMeshPlane(mesh_size, mesh_size, 1, 1);
//        models[i] = LoadModelFromMesh(mesh);
//
//        models[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
//        model_positions[i] = (Vector3) { x_coord, 0.0f, z_coord };
//
//        UnloadImage(image);
//    }

    worldInit();
    worldChunk **chunks = malloc(sizeof(worldChunk*) * 9);

    for (int i = 0; i < 1; ++i) {
        for (int j = 0; j < 3; ++j) {
            chunks[i * 3 + j] = worldGenChunk(
                    (j % 3) * CHUNK_SIZE,
                    (i * 3) * CHUNK_SIZE,
                    1, 55);
            printf("chunk at %d %d\n",
                    (j % 3) * CHUNK_SIZE,
                    (i * 3) * CHUNK_SIZE
                    );
        }
    }


    /* game stuff begins */

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
    }

    player_unit = (struct Unit) {
        .models = scarfy_models,
        .models_n = 6,
        .movement_speed = 0.3,
        .movement_speed_sprint = 0.6,
        .y = 10,
    };
    camera.target = player_unit.pos;

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
        //printf("%f %f %f\n", camera.target.x, camera.target.y, camera.target.z);
        //printf("%f %f %f\n", camera.up.x, camera.up.x, camera.up.x);
        if (frame_number % anim_frame_time == 0)
            player_unit.model_current = (player_unit.model_current + 1) % player_unit.models_n;

        // Update model animation
        //ModelAnimation anim = modelAnimations[animIndex];
        //animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        //UpdateModelAnimation(model, anim, animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                //for (int i = 0; i < n_meshes; ++i) {
                //    DrawModel(models[i], model_positions[i], 1.0f, (Color) { 0xC2, 0xB2, 0x80, 0xFF });
                //}

                for (int i = 0; i < 1; ++i) {
                    worldRenderChunk(chunks[i]);
                }

                if (player_unit.moving)
                    DrawModelEx(
                            player_unit.models[player_unit.model_current + player_unit.direction * player_unit.models_n],
                            player_unit.pos,
                            (Vector3) { 1, 0, 0 },
                            -45.0f, (Vector3) { 1, 1, 1 },
                            WHITE);
                else
                    DrawModelEx(
                            player_unit.models[player_unit.direction * player_unit.models_n + 2],
                            player_unit.pos,
                            (Vector3) { 1, 0, 0 },
                            -45.0f, (Vector3) { 1, 1, 1 },
                            WHITE);

                //DrawModelEx(model, player_unit.pos, (Vector3) { 0, 1, 0 },
                //        player_unit.direction_angle, (Vector3) { 0.2, 0.2, 0.2 }, BLUE);

                DrawGrid(20, 1.0f);

            EndMode3D();

            //DrawTextureEx(texture,
            //        (Vector2) { screenWidth - 120, 20 },
            //        0, 100.0f / texture.width, WHITE);  // Draw a Texture2D with extended parameters
            //DrawRectangleLines(screenWidth - 120, 20,
            //        100.0f, 100.0f, GREEN);

            DrawFPS(10, 10);
            sds player_info = sdscatprintf(sdsempty(), "Player pos: x:%.2fy:%.2fz:%.2f",
                    player_unit.x, player_unit.y, player_unit.z);
            DrawText(player_info, 4, 40, 8, BLACK);
            sdsfree(player_info);

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
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, cameraMoveSpeed * 2, moveInWorldPlane);
        //if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -cameraMoveSpeed * 2, moveInWorldPlane);
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

