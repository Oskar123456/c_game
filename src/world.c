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

worldChunk* worldGenChunk(int x_off, int z_off, float scale, int seed)
{
    worldChunk* chunk = malloc(sizeof(worldChunk));
    chunk->x = x_off / CHUNK_SIZE;
    chunk->z = z_off / CHUNK_SIZE;
    Image image = GenImagePerlinNoise(CHUNK_SIZE, CHUNK_SIZE,
            x_off * CHUNK_SIZE, x_off * CHUNK_SIZE, 1);

    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            float nz = (float)(i + z_off)*(scale/(float)CHUNK_SIZE);
            float nx = (float)(j + x_off)*(scale/(float)CHUNK_SIZE);

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
            u8 h = np * WORLD_ELEVATION_LEVELS;
            chunk->cubes[i][j] = (worldCube) { .type = type, .height = h };
        }
    }

    return chunk;
}

void worldRenderChunk(worldChunk *chunk)
{
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            worldCube* cube = &chunk->cubes[i][j];
            Vector3 pos = { chunk->x * CHUNK_SIZE + j, chunk->cubes[i][j].height * 0.6f, chunk->z * CHUNK_SIZE + i};
            DrawModel(_cube_models[cube->type], pos, 1, WHITE);
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
        0.51f, 0.99f,
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







