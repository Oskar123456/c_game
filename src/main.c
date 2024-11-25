/*****************************************************
Create Date:        2024-11-12
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "../include/obh/incl.h"
#include "../include/obh/c_log.h"
#include "../include/obh/util.h"
#include "../include/obh/unit.h"
#include "../include/glad/glad.h"
#include "../include/raylib/raylib.h"
#include "../include/raylib/raygui.h"
#include "../include/raylib/raymath.h"
#include "../include/raylib/rcamera.h"
#include "../include/raylib/rlgl.h"

#define STB_DS_IMPLEMENTATION
#include "../include/stb/stb_ds.h"


static int scr_w = 800, scr_h = 600;

void DrawGridPos(int slices, float spacing, Vector3 pos);
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
void pollKeys();
void pollWindowEvents();

static Camera camera;
static float CAMERA_MOVE_SPEED = 45.0f;

#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f

float CAMERA_OFF_X = 50.0f;
float CAMERA_OFF_Y = 50.0f;
float CAMERA_OFF_Z = -50.0f;

Unit player_unit = {
    .speed = 0.04,
    .speed_sprint = 0.12,
    .pos = (Vector3) {0, 0.5, 0},
    .dir = (Vector3) {1, 0, 1},
};

static Vector3 unit_horizontal, unit_vertical;

int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;
    /* initialization */
    srand(time(NULL));
    c_log_init(stderr, LOG_LEVEL_SUCCESS);

    sds s = sdscatprintf(sdsempty(), "is in working? %s", "yes");
    c_log_success(LOG_TAG, s);
    sdsfree(s);

    char camera_info_str[512] = { 0 };
    char player_info_str[512] = { 0 };

    InitWindow(scr_w, scr_h, "raylib [models] example - heightmap loading and drawing");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //DisableCursor();
    SetExitKey(0);

    /* game stuff begins */
    camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
    camera.position = (Vector3) {
        camera.target.x + CAMERA_OFF_X,
        camera.target.y + CAMERA_OFF_Y,
        camera.target.z + CAMERA_OFF_Z,
    };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 5.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Mesh m = GenMeshCube(1, 1, 1);
    Model mo = LoadModelFromMesh(m);
    player_unit.model = &mo;

    Texture2D scarfy = LoadTexture("resources/images/scarfy.png");

    Vector3 cube_pos[10];
    for (int i = 0; i < 10; ++i) {
        cube_pos[i] = (Vector3) { rand() % 70, 0.5, rand() % 70 };
    }

    BoundingBox bb = GetMeshBoundingBox(m);

    bb.min = Vector3Add(bb.min, player_unit.pos);
    bb.max = Vector3Add(bb.max, player_unit.pos);

    printf("%f %f %f\n", bb.min.x, bb.min.y, bb.min.z);
    printf("%f %f %f\n", bb.max.x, bb.max.y, bb.max.z);

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
        UpdateCamera(&camera, CAMERA_FREE);
        camera.target = player_unit.pos;
        //unitMove(&player_unit);
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLUE);

            BeginMode3D(camera);

                DrawGridPos(100, 1, Vector3Zero());
                //DrawCubeTexture(scarfy, player_unit.pos, 1, 1, 1, WHITE);
                DrawCubeWires(player_unit.pos, 1, 1, 1, GREEN);
                for (int i = 0; i < 10; ++i) {
                    Color col;


                    if (CheckCollisionBoxes(bb, bb))
                        col = RED;
                    else
                        col = BLUE;

                    DrawCube(cube_pos[i], 1, 1, 1, col);
                }

                BoundingBox bb = GetMeshBoundingBox(m);

                bb.min = Vector3Add(bb.min, player_unit.pos);
                bb.max = Vector3Add(bb.max, player_unit.pos);

                //printf("%f %f %f\n", bb.min.x, bb.min.y, bb.min.z);
                //printf("%f %f %f\n", bb.max.x, bb.max.y, bb.max.z);

                DrawLine3D(bb.min, bb.max, YELLOW);

            EndMode3D();

            sprintf(camera_info_str, "%.1f %.1f %.1f --> %.1f %.1f %.1f",
                    camera.position.x, camera.position.y, camera.position.z,
                    camera.target.x, camera.target.y, camera.target.z);
            sprintf(player_info_str, "%.2f %.2f %.2f",
                    player_unit.pos.x, player_unit.pos.y, player_unit.pos.z);
            DrawText(camera_info_str, 10, 30, 14, WHITE);
            DrawText(player_info_str, 10, 60, 14, WHITE);
            DrawFPS(10, 10);

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

void DrawGridPos(int slices, float spacing, Vector3 pos)
{
    pos.x = floor(pos.x);
    pos.y = floor(pos.y);
    pos.z = floor(pos.z);
    int halfSlices = slices/2;

    rlBegin(RL_LINES);
        for (int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing + pos.x, pos.y, (float)-halfSlices*spacing + pos.z);
            rlVertex3f((float)i*spacing + pos.x, pos.y, (float)halfSlices*spacing + pos.z);

            rlVertex3f((float)-halfSlices*spacing + pos.x, pos.y, (float)i*spacing + pos.z);
            rlVertex3f((float)halfSlices*spacing + pos.x, pos.y, (float)i*spacing + pos.z);
        }
    rlEnd();
}

void pollKeys()
{
    if (IsKeyDown(KEY_LEFT)) {
        player_unit.dir.x += 1;
        player_unit.dir.z += 1;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player_unit.dir.x += -1;
        player_unit.dir.z += -1;
    }
    if (IsKeyDown(KEY_UP)) {
        player_unit.dir.x += -1;
        player_unit.dir.z += 1;
    }
    if (IsKeyDown(KEY_DOWN)) {
        player_unit.dir.x += 1;
        player_unit.dir.z += -1;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT))
        player_unit.sprinting = true;

    else
        player_unit.sprinting = false;

    /* camera */

    if (IsKeyDown(KEY_S)) {
        CameraMoveForward(&camera, -1, false);
    }

    if (IsKeyDown(KEY_W)) {
        CameraMoveForward(&camera, 1, false);
    }

    if (IsKeyPressed(KEY_R)) {
        camera.position = (Vector3) {
            camera.target.x + CAMERA_OFF_X,
            camera.target.y + CAMERA_OFF_Y,
            camera.target.z + CAMERA_OFF_Z,
        };
    }

    unitMove(&player_unit, &camera);

    player_unit.dir.x = 0;
    player_unit.dir.z = 0;

}

void pollWindowEvents()
{

}

// Draw cube textured
// NOTE: Cube position is the center position
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // Vertex data transformation can be defined with the commented lines,
    // but in this example we calculate the transformed vertex data directly when calling rlVertex3f()
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, - 1.0f);     // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, - 1.0f, 0.0f);     // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f( - 1.0f, 0.0f, 0.0f);    // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}
