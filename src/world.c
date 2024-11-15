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

static Model _cube_models[CUBE_TYPE_NUM];
static Texture2D _cube_textures[CUBE_TYPE_NUM];

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
            Vector3 pos = { chunk->x * CHUNK_SIZE + j, chunk->cubes[i][j].height, chunk->z * CHUNK_SIZE + i};
            DrawCubeTexture(_cube_textures[chunk->cubes[i][j].type],
                    pos, 1, 1, 1, WHITE);
            //printf("%f %f %f\n", pos.x, pos.y, pos.z);
        }
    }
}

void worldInit()
{
    Image grass_img = LoadImage("resources/images/minecraft_grass.png");
    Image dirt_img  = LoadImage("resources/images/minecraft_dirt_pure.jpg");
    Image snow_img  = LoadImage("resources/images/minecraft_snow_top.jpg");

    _cube_textures[CUBE_TYPE_GRASS] = LoadTextureFromImage(grass_img);
    _cube_textures[CUBE_TYPE_DIRT]  = LoadTextureFromImage(dirt_img);
    _cube_textures[CUBE_TYPE_SNOW]  = LoadTextureFromImage(snow_img);

    UnloadImage(grass_img);
    UnloadImage(snow_img);
    UnloadImage(dirt_img);
}

// NOTE: Cube position is the center position
void DrawCubeTexture(Texture2D texture, Vector3 position,
        float width, float height, float length, Color color)
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

// Draw cube with texture piece applied to all faces
void DrawCubeTextureRec(Texture2D texture, Rectangle source,
        Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float texWidth = (float)texture.width;
    float texHeight = (float)texture.height;

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture.id);

    // We calculate the normalized texture coordinates for the desired texture-source-rectangle
    // It means converting from (tex.width, tex.height) coordinates to [0.0f, 1.0f] equivalent
    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front face
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);

        // Back face
        rlNormal3f(0.0f, 0.0f, - 1.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);

        // Top face
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);

        // Bottom face
        rlNormal3f(0.0f, - 1.0f, 0.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);

        // Right face
        rlNormal3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);

        // Left face
        rlNormal3f( - 1.0f, 0.0f, 0.0f);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);

    rlEnd();

    rlSetTexture(0);
}











