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

#define CHUNK_SIZE 20
#define WORLD_ELEVATION_LEVELS 10

typedef struct worldCube worldCube;
typedef struct worldChunk worldChunk;
typedef struct worldChunkCoord worldChunkCoord;
typedef struct worldChunkMap worldChunkMap;

enum CUBE_TYPE { CUBE_TYPE_GRASS, CUBE_TYPE_DIRT, CUBE_TYPE_SNOW, CUBE_TYPE_NUM, };

struct worldCube { u8 type; float height; };
struct worldChunkCoord { int x, z; };
struct worldChunkMap { worldChunkCoord key; worldChunk* value; };

struct worldChunk {
    int x, z; /* index, not cube coords */
    worldCube cubes[CHUNK_SIZE][CHUNK_SIZE];
};

void worldInit();
void worldRender(Vector3 player_pos);
void worldRenderChunk(worldChunk *chunk);
worldChunk* worldGenChunk(int x_off, int z_off);
void worldChunkAdd(worldChunk* chunk);
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
Mesh worldGenMeshCube(float width, float height, float length);
worldChunk* worldFindChunk(Vector3 pos);
worldChunkCoord worldFindMe(Vector3 pos);
void worldRender(Vector3 player_pos);
void worldRenderChunk(worldChunk *chunk);
void worldMapToChunkCoords(Vector3 pos, int* x, int* z);
void worldMapToCubeCoords(Vector3 pos, int* x, int* z);
bool worldCollisionTestBox(BoundingBox box);
void worldDrawClosestCubes(BoundingBox box);
sds worldGetInfo();

