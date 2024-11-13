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
#include "../include/raylib/raylib.h"
#include "../include/raylib/raymath.h"
#include "../include/raylib/rcamera.h"
#include "../include/stb/stb_perlin.h"

#include "../include/stb/stb_image.h"
#include "../include/stb/stb_image_write.h"

static float CAMERA_MOVE_SPEED = 45.0f; // Units per second
#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f       // Radians per second

void UpdateCameraCustom(Camera *camera, int mode);

static int screenWidth = 800;
static int screenHeight = 450;
static Camera camera;
static bool fullscreen;
static bool paused;

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
        if (paused)
            DisableCursor();
        else
            EnableCursor();
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

    //float camera_speed = 1;
    //if (IsKeyDown(KEY_A)) {
    //    CameraMoveRight(&camera, -camera_speed, false);
    //}
    //if (IsKeyDown(KEY_D)) {
    //    CameraMoveRight(&camera, camera_speed, false);
    //}
    //if (IsKeyDown(KEY_W)) {
    //    CameraMoveForward(&camera, camera_speed, false);
    //}
    //if (IsKeyDown(KEY_S)) {
    //    CameraMoveForward(&camera, -camera_speed, false);
    //}
}

/*
typedef struct Image {
    void *data;             // Image raw data
    int width;              // Image base width
    int height;             // Image base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
} Image;
 */

struct Pixel { u8 r, g, b, a; };

int main(int argc, char *argv[])
{


    int exit_code = EXIT_SUCCESS;
    /* initialization */
    srand(time(NULL));
    c_log_init(stderr, LOG_LEVEL_SUCCESS);

    sds s = sdscatprintf(sdsempty(), "is in working? %s", "yes");
    c_log_success(LOG_TAG, s);

    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    DisableCursor();
    SetExitKey(0);

    int size_img = 2256;
    u8 img[size_img][size_img];
    for (int i = 0; i < size_img; ++i) {
        for (int j = 0; j < size_img; ++j) {
            float ns = 1 * stb_perlin_noise3_seed((float)j / 106.0f, (float)i / 106.0f, 10, 256, 256, 256, 256);
            ns += 0.25 * stb_perlin_noise3_seed((float)j / 16.0f, (float)i / 16.0f, 10, 256, 256, 256, 256);
            ns = pow(ns, 0.35);

            u8 fin = fabs(ns * 256.0f);
            img[i][j] = fin;
        }
    }

    struct Image image = { .data = img, size_img, size_img, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
    //ExportImage(img_struct, "raylib_export.png");

    //Image image = LoadImage("raylib_export.png");     // Load heightmap image (RAM)
    Texture2D texture = LoadTextureFromImage(image);        // Convert image to texture (VRAM)

    // Define our custom camera to look into our 3d world
    camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    Mesh mesh = GenMeshHeightmap(image, (Vector3){ 4280, 16, 4280 }); // Generate heightmap mesh (RAM and VRAM)
    Model model = LoadModelFromMesh(mesh);                  // Load model from generated mesh

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture
    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    //UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
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
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, mapPosition, 1.0f, GRAY);

                DrawGrid(20, 1.0f);

            EndMode3D();

            DrawTextureEx(texture,
                    (Vector2) { screenWidth - 120, 20 },
                    0, 100.0f / texture.width, WHITE);  // Draw a Texture2D with extended parameters
            DrawRectangleLines(screenWidth - 120, 20,
                    100.0f, 100.0f, GREEN);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
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
        if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) CameraPitch(camera, cameraRotationSpeed, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -cameraRotationSpeed, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, cameraRotationSpeed, rotateAroundTarget);
        if (IsKeyDown(KEY_Q)) CameraRoll(camera, -cameraRotationSpeed);
        if (IsKeyDown(KEY_E)) CameraRoll(camera, cameraRotationSpeed);

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
            CameraYaw(camera, -mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            CameraPitch(camera, -mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);
        }

        // Keyboard support
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -cameraMoveSpeed, moveInWorldPlane);
        if (IsKeyDown(KEY_D)) CameraMoveRight(camera, cameraMoveSpeed, moveInWorldPlane);

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

