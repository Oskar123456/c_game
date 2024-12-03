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

#define DARK_GRASS (Color) { 0x22, 0x2B, 0x29, 0xFF }
#define LIGHT_GRASS (Color) { 0x6C, 0xAD, 0x18, 0xFF }

static int scr_w = 800, scr_h = 600;

static UnitCamera unit_cam;
static Unit player_unit;

#define CAMERA_ROTATION_SPEED                           0.03f
#define CAMERA_PAN_SPEED                                0.2f
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_ORBITAL_SPEED                            0.5f

static float CAMERA_OFF_X = 50.0f;
static float CAMERA_OFF_Y = 50.0f;
static float CAMERA_OFF_Z = -50.0f;

enum CUBETYPE {
    CUBETYPE_GRASS,
    CUBETYPE_NUM,
};

#define CHUNKSIZE 32
#define HEIGHTLEVELS 40

typedef struct iVec2 {
    int x, y;
} iVec2;

struct Cube {
    int x, y, z;
    enum CUBETYPE type;
};

struct WorldChunk {
    iVec2 coord;
    struct Cube cubes[CHUNKSIZE][CHUNKSIZE];
};

struct WorldMap {
    union {
        iVec2 coord;
        iVec2 key;
    };
    union {
        struct WorldChunk chunk;
        struct WorldChunk value;
    };
};

static struct WorldMap *world_map;

struct WorldChunk genWorldChunk(int x, int z)
{
    struct WorldChunk wc = { .coord = { .x = x, .y = z } };

    Image perlin_img = GenImagePerlinNoise(CHUNKSIZE, CHUNKSIZE, x * CHUNKSIZE, z * CHUNKSIZE, 0.6);
    for (int i = 0; i < CHUNKSIZE; i += 1) {
        for (int j = 0; j < CHUNKSIZE; j += 1) {
            Color* perlin_colors = perlin_img.data;
            wc.cubes[i][j] = (struct Cube) {.x = j,
                    .y = perlin_colors[(CHUNKSIZE - i - 1) * CHUNKSIZE + j].r / HEIGHTLEVELS,
                    .z = CHUNKSIZE - i - 1 };
        }
    }

    return wc;
}

iVec2 getChunkCoords(Vector3 position)
{
    int chunk_x = floor(floor(position.x) / CHUNKSIZE);
    int chunk_z = floor(floor(position.z) / CHUNKSIZE);
    return (iVec2) { chunk_x, chunk_z };
}

void genWorldAround(Vector3 position)
{
    iVec2 chunk_pos = getChunkCoords(position);
    int chunk_x = chunk_pos.x;
    int chunk_z = chunk_pos.y;

    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            iVec2 chunk_pos_inner = { chunk_x + j, chunk_z + i };
            if (hmgeti(world_map, chunk_pos_inner) >= 0)
                continue;
            struct WorldChunk wc = genWorldChunk(chunk_pos_inner.x, chunk_pos_inner.y);
            hmput(world_map, chunk_pos_inner, wc);
        }
    }
}

void pollKeys()
{
}

void pollWindowEvents()
{
}

/**
 * Tailored for minecraft grass texture
 */
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    float tex_w = texture.width;
    float tex_h = texture.height;

    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);
        // Front Face
        rlNormal3f(0.0f, 0.0f, 1.0f);       // Normal Pointing Towards Viewer
        rlTexCoord2f(0, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
        // Back Face
        rlNormal3f(0.0f, 0.0f, - 1.0f);     // Normal Pointing Away From Viewer
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
        // Top Face
        rlNormal3f(0.0f, 1.0f, 0.0f);       // Normal Pointing Up
        rlTexCoord2f((tex_w / 4) / tex_w + 0.02, 0.02);
        rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w + 0.02, ((tex_h / 3) * 1.0f ) / tex_h - 0.02);
        rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f((tex_w / 2) / tex_w - 0.02, ((tex_h / 3) * 1.0f) / tex_h - 0.02);
        rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f((tex_w / 2) / tex_w - 0.02, 0.02);
        rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
        // Bottom Face
        rlNormal3f(0.0f, - 1.0f, 0.0f);     // Normal Pointing Down
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
        // Right face
        rlNormal3f(1.0f, 0.0f, 0.0f);       // Normal Pointing Right
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
        // Left Face
        rlNormal3f( - 1.0f, 0.0f, 0.0f);    // Normal Pointing Left
        rlTexCoord2f(0, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 2.0f) / tex_h);
        rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
        rlTexCoord2f((tex_w / 4) / tex_w, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
        rlTexCoord2f(0, ((tex_h / 3) * 1.0f) / tex_h);
        rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
    rlEnd();

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
        DrawModel(*to_draw, to_draw_pos, 1, LIGHT_GRASS);
}

bool CollisionTestSimple(Unit* unit, BoundingBox* bbs, int bbs_len)
{
    static int o = 0;
    for (int i = 0; i < bbs_len; ++i) {
        bool collision = CheckCollisionBoxes(bbs[i], unitBoundingBox(unit));
        if (!collision)
            continue;

        if (unit->falling) {
            unit->falling = false;
            unit->fall_velocity = 0;
            unit->direction.y = 0;
        }

        unit->position.y = bbs[i].max.y + 0.5;

        return true;
    }

    return false;
}

bool CollisionTest(Unit* unit, BoundingBox* bbs, int bbs_len)
{
    if (!unit->falling)
        return false;

    Vector3 unit_dir = unitGetMovementVec(unit);
    BoundingBox unit_bb = unitBoundingBox(unit);

    Ray unit_ray = { .position = unit->position, .direction = unit->direction };

    for (int i = 0; i < bbs_len; ++i) {
        bool collision = CheckCollisionBoxes(bbs[i], unit_bb);
        if (!collision)
            continue;
        RayCollision rc_data = GetRayCollisionBox(unit_ray, bbs[i]);
        if (!rc_data.hit)
            continue;

        printf("collision: unit.pos: ");
        arr_f_print((float*)&unit->position, 3);
        printf(" unit.dir: ");
        arr_f_print((float*)&unit->direction, 3);
        printf(" bb: ");
        arr_f_print((float*)&bbs[i].min, 3);
        printf(" -- ");
        arr_f_print((float*)&bbs[i].max, 3);
        printf(" at point: ");
        arr_f_print((float*)&rc_data.point, 3);
        printf("\n");

        if (rc_data.normal.y > 0) {
            unit->falling = false;
            unit->fall_velocity = 0;
            //printf(" (fall)");
        }

        printf("\n");

        unit->direction = (Vector3) { 0 };
        unit->position = rc_data.point;
        //unit->position.y = bbs[i].max.y;

        return true;
    }
    return false;
}

RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create depth texture
        // We don't need a color texture for the shadowmap
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach depth texture to FBO
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
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
    char sun_info_str[512] = { 0 };

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(scr_w, scr_h, "raylib [models] example - heightmap loading and drawing");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    //DisableCursor();
    SetExitKey(0);

    /* game stuff begins */
    player_unit.movement_speed = 0.14;
    player_unit.sprint_factor = 4;
    player_unit.position = (Vector3) {0, 20, 0};
    player_unit.falling = true;

    unit_cam.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    unit_cam.camera.fovy = 5.0f;
    unit_cam.camera.projection = CAMERA_PERSPECTIVE;
    unit_cam.following = &player_unit;
    unit_cam.speed = 0.60;
    unit_cam.off_x = CAMERA_OFF_X;
    unit_cam.off_y = CAMERA_OFF_Y;
    unit_cam.off_z = CAMERA_OFF_Z;

    Font font = LoadFont("resources/fonts/overpass-regular.otf");

    Texture2D scarfy = LoadTexture("resources/images/scarfy.png");

    Vector3 base_plane_pos = { 0, -50, 0};
    Mesh base_plane = GenMeshCube(100, 100, 100);
    Model base_plane_model = LoadModelFromMesh(base_plane);
    BoundingBox base_plane_bb = GetMeshBoundingBox(base_plane);
    base_plane_bb.min = Vector3Add(base_plane_bb.min, base_plane_pos);
    base_plane_bb.max = Vector3Add(base_plane_bb.max, base_plane_pos);
    printf("%f %f %f --> %f %f %f \n", base_plane_bb.min.x,base_plane_bb.min.y,base_plane_bb.min.z,
            base_plane_bb.max.x,base_plane_bb.max.y,base_plane_bb.max.z);

    Texture2D tex_grass = LoadTexture("resources/images/minecraft_grass.png");
    Texture2D tex_dirt = LoadTexture("resources/images/minecraft_dirt_pure.jpg");

    iVec2 origin = { 0 };
    struct WorldChunk wc = genWorldChunk(0, 0);
    hmput(world_map, origin, wc);

    /* game stuff ends */

    Shader shadowShader = LoadShader("resources/shaders/basic_shadow.vs",
                                     "resources/shaders/basic_shadow.fs");
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    Color lightColor = WHITE;
    Vector4 lightColorNormalized = ColorNormalize(lightColor);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    int ambientLoc = GetShaderLocation(shadowShader, "ambient");
    float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    SetShaderValue(shadowShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);
    int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    int shadowMapResolution = 1024;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);

    RenderTexture2D shadowMap = LoadShadowmapRenderTexture(shadowMapResolution, shadowMapResolution);
    Camera3D lightCam = (Camera3D){ 0 };
    lightCam.position = (Vector3) { -100, 200, -100 };
    lightCam.target = Vector3Zero();
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 90.0f;

    Mesh m = GenMeshCube(1, 1, 1);
    Model mo = LoadModelFromMesh(m);
    mo.materials[0].shader = shadowShader;
    player_unit.model = &mo;

    int n_cubes = 75;
    Vector3 cube_pos[n_cubes];
    for (int i = 0; i < n_cubes; ++i) {
        cube_pos[i] = (Vector3) { rand() % 70, 0.5, rand() % 70 };
    }

    BoundingBox cube_bb[n_cubes];
    for (int i = 0; i < n_cubes; ++i) {
        cube_bb[i] = GetBoundingBoxModelWithPos(&mo, cube_pos[i]);
    }


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

        unitPollInputs(&player_unit);
        unitCamPollInputs(&unit_cam);
        //----------------------------------------------------------------------------------
        // Update
        unitUpdate(&player_unit);
        unitUpdateThirdPersonCamera(&unit_cam);

        genWorldAround(player_unit.position);

        iVec2 player_chunk_pos = getChunkCoords(player_unit.position);
        struct WorldChunk player_chunk = hmget(world_map, player_chunk_pos);


        Vector3 cameraPos = unit_cam.camera.position;
        SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);

        bool player_world_collision = false;
        for (int z = 0; z < CHUNKSIZE; ++z) {
            for (int x = 0; x < CHUNKSIZE; ++x) {
                Vector3 pos = {
                    player_chunk.cubes[z][x].x + player_chunk_pos.x * CHUNKSIZE,
                    player_chunk.cubes[z][x].y,
                    player_chunk.cubes[z][x].z + player_chunk_pos.y * CHUNKSIZE,
                };
                BoundingBox bb = GetMeshBoundingBox(m);
                bb.min = Vector3Add(bb.min, pos);
                bb.max = Vector3Add(bb.max, pos);
                bool coll = CollisionTestSimple(&player_unit, &bb, 1);
                if (coll)
                    player_world_collision = true;
            }
        }
        if (!(player_world_collision || CollisionTestSimple(&player_unit, &base_plane_bb, 1)))
            player_unit.falling = true;

        unitUpdateThirdPersonCamera(&unit_cam);
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(WHITE);

            Matrix lightView;
            Matrix lightProj;
            BeginTextureMode(shadowMap);
            ClearBackground(WHITE);
            BeginMode3D(lightCam);
                lightView = rlGetMatrixModelview();
                lightProj = rlGetMatrixProjection();
                /* world render */
                for (int i = 0; i < 1; ++i) {
                    for (int z = 0; z < CHUNKSIZE; ++z) {
                        for (int x = 0; x < CHUNKSIZE; ++x) {
                            Vector3 pos = {
                                world_map[i].chunk.cubes[z][x].x + world_map[i].coord.x * CHUNKSIZE,
                                world_map[i].chunk.cubes[z][x].y,
                                world_map[i].chunk.cubes[z][x].z + world_map[i].coord.y * CHUNKSIZE,
                            };
                            DrawModel(mo, pos, 0.90, BROWN);
                            //DrawCubeTexture(tex_grass, pos, 1, 1, 1, WHITE);
                        }
                    }
                }
                DrawModel(*player_unit.model, player_unit.position, 1, BLUE);
            EndMode3D();
            EndTextureMode();
            Matrix lightViewProj = MatrixMultiply(lightView, lightProj);

            ClearBackground(BLUE);

            SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);

            rlEnableShader(shadowShader.id);
            int slot = 10;
            rlActiveTextureSlot(slot);
            rlEnableTexture(shadowMap.depth.id);
            rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

            BeginMode3D(unit_cam.camera);

                /* world render */
                for (int i = 0; i < 1; ++i) {
                    for (int z = CHUNKSIZE - 1; z >= 0; --z) {
                        for (int x = 0; x < CHUNKSIZE; ++x) {
                            Vector3 pos = {
                                world_map[i].chunk.cubes[z][x].x + world_map[i].coord.x * CHUNKSIZE,
                                world_map[i].chunk.cubes[z][x].y,
                                world_map[i].chunk.cubes[z][x].z + world_map[i].coord.y * CHUNKSIZE,
                            };
                            DrawModel(mo, pos, 0.90, BROWN);
                            //DrawCubeWires(pos, 1, 1, 1, WHITE);
                            //DrawCubeTexture(tex_grass, pos, 1, 1, 1, WHITE);
                        }
                    }
                }

                //DrawGridPos(100, 1, (Vector3) { 0, 0.5, 0 });
                DrawModel(*player_unit.model, player_unit.position, 1, BLUE);
                //DrawModel(base_plane_model, base_plane_pos, 1, DARK_GRASS);
                DrawCubeWires(player_unit.position, 1, 1, 1, GREEN);
                DrawAxes(GetBoundingBoxModelWithPos(player_unit.model, player_unit.position).min, 2, font);

            EndMode3D();

            sprintf(camera_info_str, "camera: %.1f %.1f %.1f --> %.1f %.1f %.1f",
                    unit_cam.camera.position.x, unit_cam.camera.position.y, unit_cam.camera.position.z,
                    unit_cam.camera.target.x, unit_cam.camera.target.y, unit_cam.camera.target.z);
            sprintf(player_info_str, "player: %.2f %.2f %.2f / dir: %.2f, %.2f, %.2f",
                    player_unit.position.x, player_unit.position.y, player_unit.position.z,
                    player_unit.direction.x, player_unit.direction.y, player_unit.direction.z);
            sprintf(sun_info_str, "sun: %.2f %.2f %.2f",
                    lightCam.position.x, lightCam.position.y, lightCam.position.z);

            DrawTextEx(font, camera_info_str, (Vector2) { 10, 30 }, 18, 1, YELLOW);
            DrawTextEx(font, player_info_str, (Vector2) { 10, 50 }, 18, 1, YELLOW);
            DrawTextEx(font, sun_info_str, (Vector2) { 10, 70 }, 18, 1, YELLOW);

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

