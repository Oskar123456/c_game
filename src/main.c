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
#include "../include/obh/debug.h"

#include "../include/glad/glad.h"

#include "../include/raylib/raylib.h"
#include "../include/raylib/raygui.h"
#include "../include/raylib/raymath.h"
#include "../include/raylib/rcamera.h"
#include "../include/raylib/rlgl.h"

#define STB_DS_IMPLEMENTATION
#include "../include/stb/stb_ds.h"

static int scr_w = 800, scr_h = 600;

static Camera camera;
static float CAMERA_MOVE_SPEED = 45.0f;

#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f

static float CAMERA_OFF_X = 50.0f;
static float CAMERA_OFF_Y = 50.0f;
static float CAMERA_OFF_Z = -50.0f;

Unit player_unit = {
    .speed = 0.04,
    .speed_sprint = 0.12,
    .pos = (Vector3) {0, 0.5, 0},
    .dir = (Vector3) {1, 0, 1},
};

static Vector3 unit_horizontal, unit_vertical;

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

    if (IsKeyPressed(KEY_SPACE) && !player_unit.falling) {
        player_unit.falling = true;
        player_unit.speed_fall = -1;
    }

    /* camera */

    if (IsKeyDown(KEY_S)) {
        CAMERA_OFF_X += 0.4;
        CAMERA_OFF_Y += 0.4;
        CAMERA_OFF_Z -= 0.4;
    }

    if (IsKeyDown(KEY_W)) {
        CAMERA_OFF_X -= 0.4;
        CAMERA_OFF_Y -= 0.4;
        CAMERA_OFF_Z += 0.4;
    }

    if (IsKeyDown(KEY_A)) {
        CAMERA_OFF_X -= 0.4;
        CAMERA_OFF_Y += 0.4;
    }

    if (IsKeyDown(KEY_D)) {
        CAMERA_OFF_X += 0.4;
        CAMERA_OFF_Y -= 0.4;
    }

    if (IsKeyPressed(KEY_R)) {
        CAMERA_OFF_X = 50;
        CAMERA_OFF_Y = 50;
        CAMERA_OFF_Z = -50;
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

BoundingBox GetBoundingBoxModelWithPos(Model* model, Vector3 pos)
{
    BoundingBox bb = GetModelBoundingBox(*model);
    bb.min = Vector3Add(bb.min, pos);
    bb.max = Vector3Add(bb.max, pos);
    return bb;
}

void DrawModelWithCollisionCheck(Model* to_draw, Vector3 to_draw_pos,
        Model* to_check, Vector3 to_check_pos)
{
    BoundingBox to_check_bb = GetBoundingBoxModelWithPos(to_check, to_check_pos);
    BoundingBox to_draw_bb = GetBoundingBoxModelWithPos(to_draw, to_draw_pos);

    bool collision = CheckCollisionBoxes(to_check_bb, to_draw_bb);
    if (collision)
        DrawModel(*to_draw, to_draw_pos, 1, RED);
    else
        DrawModel(*to_draw, to_draw_pos, 1, WHITE);
}

bool CollisionTest(Unit* unit, BoundingBox* bbs, int bbs_len)
{
    if (!unit->falling)
        return false;
    BoundingBox unit_bb = GetBoundingBoxModelWithPos(unit->model, unit->pos);
    for (int i = 0; i < bbs_len; ++i) {
        if (CheckCollisionBoxes(unit_bb, bbs[i])) {
            unitStop(unit);
            unit->pos.y = bbs[i].max.y + 0.51;
            printf("collision between BB (min: ");
            arr_f_print((float*)&bbs[i].min, 3);
            printf(" | max: ");
            arr_f_print((float*)&bbs[i].max, 3);
            printf(")\n and unit (pos: ");
            arr_f_print((float*)&unit->pos, 3);
            printf(")\n");
            return true;
        }
    }
    return false;
}

void UpdateCameraThirdPerson(Camera* camera, Unit* unit)
{
    camera->target = unit->pos;
    camera->position = (Vector3) {
        camera->target.x + CAMERA_OFF_X,
        camera->target.y + CAMERA_OFF_Y,
        camera->target.z + CAMERA_OFF_Z,
    };
}

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

    Font font = LoadFont("resources/fonts/overpass-regular.otf");

    Texture2D scarfy = LoadTexture("resources/images/scarfy.png");

    int n_cubes = 75;
    Vector3 cube_pos[n_cubes];
    for (int i = 0; i < n_cubes; ++i) {
        cube_pos[i] = (Vector3) { rand() % 70, 0.5, rand() % 70 };
    }

    BoundingBox cube_bb[n_cubes];
    for (int i = 0; i < n_cubes; ++i) {
        cube_bb[i] = GetBoundingBoxModelWithPos(&mo, cube_pos[i]);
    }

    Vector3 base_plane_pos = { 0, 0, 0 };
    Mesh base_plane = GenMeshPlane(100, 100, 1, 1);
    BoundingBox base_plane_bb = GetMeshBoundingBox(base_plane);
    base_plane_bb.min.y = -1.5;
    base_plane_bb.max.y = 0.01;
    printf("%f %f %f --> %f %f %f \n", base_plane_bb.min.x,base_plane_bb.min.y,base_plane_bb.min.z,
            base_plane_bb.max.x,base_plane_bb.max.y,base_plane_bb.max.z);

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
        unitUpdate(&player_unit);

        CollisionTest(&player_unit, cube_bb, n_cubes);
        CollisionTest(&player_unit, &base_plane_bb, 1);

        UpdateCamera(&camera, CAMERA_FREE);
        UpdateCameraThirdPerson(&camera, &player_unit);
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLUE);

            BeginMode3D(camera);

                DrawGridPos(100, 1, Vector3Zero());
                DrawCubeTexture(scarfy, player_unit.pos, 1, 1, 1, WHITE);
                DrawCubeWires(player_unit.pos, 1, 1, 1, GREEN);
                for (int i = 0; i < n_cubes; ++i) {
                    Color col;
                    col = RED;

                    DrawModelWithCollisionCheck(&mo, cube_pos[i], player_unit.model, player_unit.pos);
                }

                DrawAxes(GetBoundingBoxModelWithPos(player_unit.model, player_unit.pos).min, 2, font);

            EndMode3D();

            sprintf(camera_info_str, "%.1f %.1f %.1f --> %.1f %.1f %.1f",
                    camera.position.x, camera.position.y, camera.position.z,
                    camera.target.x, camera.target.y, camera.target.z);
            sprintf(player_info_str, "%.2f %.2f %.2f",
                    player_unit.pos.x, player_unit.pos.y, player_unit.pos.z);

            DrawTextEx(font, camera_info_str, (Vector2) { 10, 30 }, 18, 1, YELLOW);
            DrawTextEx(font, player_info_str, (Vector2) { 10, 50 }, 18, 1, YELLOW);

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

