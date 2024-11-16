/*****************************************************
Create Date:        2024-11-12
Author:             Oskar Bahner Hansen
Email:              cph-oh82@cphbusiness.dk
Description:        exercise in games programming
License:            none
*****************************************************/

#include "../include/world.h"
#include "../include/stb/stb_perlin.h"
#include "../include/raylib/rlgl.h"

#define MAX_LIGHTS  4         // Max dynamic lights supported by shader

// Light data
typedef struct {
    int type;
    bool enabled;
    Vector3 position;
    Vector3 target;
    Color color;
    float attenuation;

    // Shader locations
    int enabledLoc;
    int typeLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int attenuationLoc;
} Light;

// Light type
typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT
} LightType;

Light CreateLight(int type, Vector3 position, Vector3 target, Color color, Shader shader);
void UpdateLightValues(Shader shader, Light light);

static Model _cube_models[CUBE_TYPE_NUM];
static Texture2D _cube_textures[CUBE_TYPE_NUM];
static Shader _cube_shader;
static int lightsCount = 0;

static float world_scale = 1;
static int world_seed    = 1;

static worldChunk** WORLD_CHUNKS;
static u32          WORLD_CHUNKS_NUM;
static u32          WORLD_CHUNKS_CAP;

static worldChunkMap* chunk_map;

sds worldGetInfo()
{
    sds inf = sdscatprintf(sdsempty(),
            "worldinf: %ld chunks",
            hmlen(chunk_map));
    return inf;
}

void worldChunkAdd(worldChunk* chunk)
{
    c_log_info(LOG_TAG, "worldChunkAdd %d %d %p", chunk->x, chunk->z, chunk);
    worldChunkCoord coords = {chunk->x, chunk->z};
    hmput(chunk_map, coords, chunk);
}

worldChunk* worldGenChunk(int x_off, int z_off)
{
    c_log_info(LOG_TAG, "worldGenChunk: %d %d", x_off, z_off);
    worldChunk* chunk = malloc(sizeof(worldChunk));
    chunk->x = x_off;
    chunk->z = z_off;

    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            float nz = (float)(i + z_off * CHUNK_SIZE)*(world_scale/(float)CHUNK_SIZE);
            float nx = (float)(j + x_off * CHUNK_SIZE)*(world_scale/(float)CHUNK_SIZE);

            // Calculate a better perlin noise using fbm (fractal brownian motion)
            // Typical values to start playing with:
            //   lacunarity = ~2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
            //   gain       =  0.5   -- relative weighting applied to each successive octave
            //   octaves    =  6     -- number of "octaves" of noise3() to sum
            float p = stb_perlin_fbm_noise3(nx, nz, 1.0f, 2.0f, 0.5f, 6);

            // Clamp between -1.0f and 1.0f
            if (p < -1.0f) p = -1.0f;
            if (p > 1.0f) p = 1.0f;

            // We need to normalize the data from [-1..1] to [0..1]
            float np = (p + 1.0f)/2.0f;

            u8 type = np * CUBE_TYPE_NUM;
            float h = (np * WORLD_ELEVATION_LEVELS) * 0.6f;
            chunk->cubes[i][j] = (worldCube) { .type = type, .height = h };
        }
    }

    return chunk;
}

void worldMapToCubeCoords(Vector3 pos, int* x, int* z)
{
    *x = (int)floor(pos.x);
    *z = (int)floor(pos.z);
}

void worldMapToChunkCoords(Vector3 pos, int* x, int* z)
{
    *x = (int)floor(pos.x) / CHUNK_SIZE - ((pos.x < 0) ? 1 : 0);
    *z = (int)floor(pos.z) / CHUNK_SIZE - ((pos.z < 0) ? 1 : 0);
}

worldChunkCoord worldFindMe(Vector3 pos)
{
    worldChunk* chunk = NULL;
    worldChunkCoord coords;
    worldMapToChunkCoords(pos, &coords.x, &coords.z);
    return coords;
}

void worldRender(Vector3 player_pos)
{
    worldChunkCoord coords;
    worldMapToChunkCoords(player_pos, &coords.x, &coords.z);
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            worldChunkCoord coords_adj = {coords.x + j, coords.z + i};
            if (hmget(chunk_map, coords_adj) == NULL) {
                worldChunkAdd(worldGenChunk(coords_adj.x, coords_adj.z));
            }
            //c_log_info(LOG_TAG, "before crash");
            worldRenderChunk(hmget(chunk_map, coords_adj));
            //c_log_info(LOG_TAG, "after crash");
        }
    }
}

void worldRenderChunk(worldChunk *chunk)
{
    if (chunk == NULL)
        c_log_error(LOG_TAG, "worldRenderChunk NULL pointer");
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            worldCube* cube = &chunk->cubes[i][j];
            Vector3 pos = { chunk->x * CHUNK_SIZE + j, chunk->cubes[i][j].height, chunk->z * CHUNK_SIZE + i};
            DrawModel(_cube_models[0], pos, 1, WHITE);
        }
    }
}

void worldInit()
{
    Image grass_img = LoadImage("resources/images/minecraft_grass_crop.png");
    Image dirt_img  = LoadImage("resources/images/minecraft_grass_crop.png");
    Image snow_img  = LoadImage("resources/images/minecraft_grass_crop.png");

    _cube_textures[CUBE_TYPE_GRASS] = LoadTextureFromImage(grass_img);
    _cube_textures[CUBE_TYPE_DIRT]  = LoadTextureFromImage(dirt_img);
    _cube_textures[CUBE_TYPE_SNOW]  = LoadTextureFromImage(snow_img);

    _cube_models[CUBE_TYPE_GRASS] = LoadModelFromMesh(worldGenMeshCube(1, 1, 1));
    _cube_models[CUBE_TYPE_DIRT]  = LoadModelFromMesh(worldGenMeshCube(1, 1, 1));
    _cube_models[CUBE_TYPE_SNOW]  = LoadModelFromMesh(worldGenMeshCube(1, 1, 1));

    _cube_models[CUBE_TYPE_GRASS].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _cube_textures[CUBE_TYPE_GRASS];
    _cube_models[CUBE_TYPE_DIRT].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _cube_textures[CUBE_TYPE_DIRT];
    _cube_models[CUBE_TYPE_SNOW].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = _cube_textures[CUBE_TYPE_SNOW];

    UnloadImage(grass_img);
    UnloadImage(snow_img);
    UnloadImage(dirt_img);

    _cube_shader = LoadShader("resources/shaders/basic.vs", "resources/shaders/basic.fs");

    _cube_shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(_cube_shader, "matModel");
    _cube_shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(_cube_shader, "viewPos");

    int ambientLoc = GetShaderLocation(_cube_shader, "ambient");
    SetShaderValue(_cube_shader, ambientLoc, (float[4]){ 0.8f, 0.8f, 0.8f, 1.0f }, SHADER_UNIFORM_VEC4);
    // Using just 1 point lights
    CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0, 100, 0 },
            Vector3Zero(), (Color) { 0xFD, 0xFB, 0xD3, 0xFF }, _cube_shader);
    //CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0, 100, 0 },
    //        Vector3Zero(), WHITE, _cube_shader);

    _cube_models[CUBE_TYPE_GRASS].materials[0].shader = _cube_shader;
    _cube_models[CUBE_TYPE_DIRT].materials[0].shader = _cube_shader;
    _cube_models[CUBE_TYPE_SNOW].materials[0].shader = _cube_shader;
}

Light CreateLight(int type, Vector3 position, Vector3 target, Color color, Shader shader)
{
    Light light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = true;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color = color;

        // NOTE: Lighting shader naming must be the provided ones
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
        light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));

        UpdateLightValues(shader, light);

        lightsCount++;
    }

    return light;
}

worldChunk* worldFindChunk(Vector3 pos)
{
    int x = (int)floor(pos.x) / CHUNK_SIZE - ((pos.x < 0) ? 1 : 0);
    int z = (int)floor(pos.z) / CHUNK_SIZE - ((pos.z < 0) ? 1 : 0);
    //c_log_info(LOG_TAG, "%f %f -> %d %d", pos.x, pos.z, x, z);
    for (int i = 0; i < WORLD_CHUNKS_NUM; ++i) {
        if (WORLD_CHUNKS[i]->x == x && WORLD_CHUNKS[i]->z == z)
            return WORLD_CHUNKS[i];
    }
    worldChunkAdd(worldGenChunk(x, z));
    return worldFindChunk(pos);
}

// Send light properties to shader
// NOTE: Light shader locations should be available
void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    float color[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255,
                       (float)light.color.b/(float)255, (float)light.color.a/(float)255 };
    SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
}

// Generated cuboid mesh
Mesh worldGenMeshCube(float width, float height, float length)
{
    Mesh mesh = { 0 };

    float vertices[] = {
        -width/2, -height/2, length/2,
        width/2, -height/2, length/2,
        width/2, height/2, length/2,
        -width/2, height/2, length/2,

        -width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, -height/2, -length/2,

        -width/2, height/2, -length/2,
        -width/2, height/2, length/2,
        width/2, height/2, length/2,
        width/2, height/2, -length/2,

        -width/2, -height/2, -length/2,
        width/2, -height/2, -length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, length/2,

        width/2, -height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, height/2, length/2,
        width/2, -height/2, length/2,

        -width/2, -height/2, -length/2,
        -width/2, -height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, height/2, -length/2
    };

    float texcoords[] = {
        /* back */
        0.99f, 0.01f,
        0.01f, 0.01f,
        0.01f, 0.99f,
        0.99f, 0.99f,
        /* front */
        0.99f, 0.99f,
        0.99f, 0.01f,
        0.55f, 0.01f,
        0.55f, 0.99f,
        /* up */
        0.01f, 0.99f,
        0.01f, 0.01f,
        0.49f, 0.01f,
        0.49f, 0.99f,
        /* down */
        0.99f, 0.99f,
        0.01f, 0.99f,
        0.01f, 0.01f,
        0.99f, 0.01f,
        /* left */
        0.51f, 0.99f,
        0.51f, 0.01f,
        0.99f, 0.01f,
        0.99f, 0.99f,
        /* right */
        0.99f, 0.99f,
        0.51f, 0.99f,
        0.51f, 0.01f,
        0.99f, 0.01f,
    };

    float normals[] = {
        /* back */
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        /* front */
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        /* up */
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        /* down */
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        /* right */
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        /* left */
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    mesh.vertices = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.vertices, vertices, 24*3*sizeof(float));

    mesh.texcoords = (float *)RL_MALLOC(24*2*sizeof(float));
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));

    mesh.normals = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.normals, normals, 24*3*sizeof(float));

    mesh.indices = (unsigned short *)RL_MALLOC(36*sizeof(unsigned short));

    int k = 0;

    // Indices can be initialized right now
    for (int i = 0; i < 36; i += 6)
    {
        mesh.indices[i] = 4*k;
        mesh.indices[i + 1] = 4*k + 1;
        mesh.indices[i + 2] = 4*k + 2;
        mesh.indices[i + 3] = 4*k;
        mesh.indices[i + 4] = 4*k + 2;
        mesh.indices[i + 5] = 4*k + 3;

        k++;
    }

    mesh.vertexCount = 24;
    mesh.triangleCount = 12;

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

worldCube worldGetCube(Vector3 pos)
{
    worldChunkCoord coords_chunk;
    worldMapToChunkCoords(pos, &coords_chunk.x, &coords_chunk.z);

    if (hmget(chunk_map, coords_chunk) == NULL) {
        worldChunkAdd(worldGenChunk(coords_chunk.x, coords_chunk.z));
    }

    worldChunk* chunk = hmget(chunk_map, coords_chunk);

    worldChunkCoord coords_cube;
    worldMapToCubeCoords(pos, &coords_cube.x, &coords_cube.z);

    bool sign_x = coords_cube.x < 0;
    bool sign_z = coords_cube.z < 0;

    if (sign_x)
        coords_cube.x = CHUNK_SIZE - abs(coords_cube.x);
    else
        coords_cube.x = coords_cube.x % CHUNK_SIZE;
    if (sign_z)
        coords_cube.z = CHUNK_SIZE - abs(coords_cube.z);
    else
        coords_cube.z = coords_cube.z % CHUNK_SIZE;

    return chunk->cubes[coords_cube.z][coords_cube.x];
}

bool worldCollisionTestBox(BoundingBox box)
{
    bool retval = false;

    worldChunkCoord coords_chunk;
    worldMapToChunkCoords(box.min, &coords_chunk.x, &coords_chunk.z);
    worldChunk* chunk = hmget(chunk_map, coords_chunk);

    worldCube cube;
    worldChunkCoord coords_cube;
    worldMapToCubeCoords(box.min, &coords_cube.x, &coords_cube.z);
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            worldCube cube = worldGetCube(Vector3Add(box.min, (Vector3) { j, 0, i }));
            BoundingBox cube_box = {
                { coords_cube.x + j, cube.height, coords_cube.z + i },
                { coords_cube.x + j + 1, cube.height + 1, coords_cube.z + i + 1 }
            };
            if (CheckCollisionBoxes(box, cube_box))
                retval = true;
            c_log_info(LOG_TAG, "testing col: %f %f %f, against %f %f %f",
                    box.min.x, box.min.y, box.min.z,
                    cube_box.min.x, cube_box.min.y, cube_box.min.z);
        }
    }

    if (retval)
        c_log_info(LOG_TAG, "COLLISION: %f %f %f, against %d %f %d",
               box.min.x, box.min.y, box.min.z, coords_cube.x, cube.height, coords_cube.z);
    return retval;
}

void worldDrawClosestCubes(BoundingBox box)
{
    worldChunkCoord coords_chunk;
    worldMapToChunkCoords(box.min, &coords_chunk.x, &coords_chunk.z);
    worldChunk* chunk = hmget(chunk_map, coords_chunk);

    worldCube cube;
    worldChunkCoord coords_cube;
    worldMapToCubeCoords(box.min, &coords_cube.x, &coords_cube.z);
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            worldCube cube = worldGetCube(Vector3Add(box.min, (Vector3) { j, 0, i }));
            BoundingBox cube_box = {
                { coords_cube.x + j + 0.08, cube.height + 0.08, coords_cube.z + i - 0.08},
                { coords_cube.x + j + 1, cube.height + 1, coords_cube.z + i + 1 }
            };
            DrawCubeWires(cube_box.min, 1.08, 1.08, 1.08, WHITE);
        }
    }
}





