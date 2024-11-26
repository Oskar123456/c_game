#include "incl.h"
#include "../raylib/raylib.h"
#include "../raylib/rlgl.h"

static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, bool SHOW_LETTER_BOUNDRY, Color tint);
static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint);
void DrawAxes(Vector3 pos, float len, Font font);
void DrawGridPos(int slices, float spacing, Vector3 pos);

