/*****************************************************
Create Date:        2024-11-12
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        noats is a notes-taking and
                    synchronization application.
License:            MIT
*****************************************************/

#include "../include/incl.h"
#include "../include/db.h"
#include "../include/c_log.h"
#include "../include/raylib/raylib.h"

/* -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-10-31.............
 * ----------------------- */

void pollKeys()
{
    if (IsKeyDown(KEY_Q))
        SetExitKey(KEY_Q);
}

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
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");
    SetExitKey(0);

    // Define our custom camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 18.0f, 21.0f, 18.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    Image image = LoadImage("resources/images/heightmap.png");     // Load heightmap image (RAM)
    Texture2D texture = LoadTextureFromImage(image);        // Convert image to texture (VRAM)

    Mesh mesh = GenMeshHeightmap(image, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
    Model model = LoadModelFromMesh(mesh);                  // Load model from generated mesh

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture
    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Events
        //----------------------------------------------------------------------------------
        pollKeys();
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, mapPosition, 1.0f, RED);

                DrawGrid(20, 1.0f);

            EndMode3D();

            DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
            DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------


    return exit_code == EXIT_FAILURE ? exit_code : EXIT_SUCCESS;
}



