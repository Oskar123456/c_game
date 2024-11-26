#include "../include/obh/debug.h"

static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position,
        float fontSize, bool backface, bool SHOW_LETTER_BOUNDRY, Color tint)
{
    int LETTER_BOUNDRY_SIZE = 1;
    Color LETTER_BOUNDRY_COLOR = BLACK;
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)/(float)font.baseSize*scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)/(float)font.baseSize*scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
    float height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV(
                (Vector3){ position.x + width/2, position.y, position.z + height/2},
                (Vector3){ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        rlCheckRenderBatchLimit(4 + 4*backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                // Front Face
                rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
                rlTexCoord2f(tw, th); rlVertex3f(x,         y, z);              // Top Left Of The Texture and Quad
                rlTexCoord2f(tw, ty); rlVertex3f(x,         y, z + height);     // Bottom Left Of The Texture and Quad
                rlTexCoord2f(tx, ty); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
                rlTexCoord2f(tx, th); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

                if (backface)
                {
                    // Back Face
                    rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
                    rlTexCoord2f(tw, th); rlVertex3f(x,         y, z);          // Top Right Of The Texture and Quad
                    rlTexCoord2f(tx, th); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
                    rlTexCoord2f(tx, ty); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
                    rlTexCoord2f(tw, ty); rlVertex3f(x,         y, z + height); // Bottom Right Of The Texture and Quad
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize,
        float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);

    float textOffsetY = 0.0f;
    float textOffsetX = 0.0f;

    float scale = fontSize/(float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[length - i - 1], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing/(float)font.baseSize*scale;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint,
                        (Vector3){ position.x + textOffsetX, position.y, position.z + textOffsetY },
                        fontSize, backface, false, tint);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += (float)(font.recs[index].width + fontSpacing)/(float)font.baseSize*scale;
            else
                textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing)/(float)font.baseSize*scale;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}


void DrawAxes(Vector3 pos, float len, Font font)
{
    float arrow_len = len / 8;
    Color color = YELLOW;

    rlBegin(RL_LINES);

        rlColor3f(color.r, color.g, color.b);

        /* X */
        rlVertex3f(pos.x, pos.y, pos.z);
        rlVertex3f(pos.x + len, pos.y, pos.z);

        rlVertex3f(pos.x + len, pos.y, pos.z);
        rlVertex3f(pos.x + len - arrow_len, pos.y, pos.z - arrow_len / 2);

        rlVertex3f(pos.x + len, pos.y, pos.z);
        rlVertex3f(pos.x + len - arrow_len, pos.y, pos.z + arrow_len / 2);

        /* Y */
        rlVertex3f(pos.x, pos.y, pos.z);
        rlVertex3f(pos.x, pos.y + len, pos.z);

        rlVertex3f(pos.x, pos.y + len, pos.z);
        rlVertex3f(pos.x, pos.y + len - arrow_len, pos.z - arrow_len / 2);

        rlVertex3f(pos.x, pos.y + len, pos.z);
        rlVertex3f(pos.x, pos.y + len - arrow_len, pos.z + arrow_len / 2);

        /* Z */
        rlVertex3f(pos.x, pos.y, pos.z);
        rlVertex3f(pos.x, pos.y, pos.z + len);

        rlVertex3f(pos.x, pos.y, pos.z + len);
        rlVertex3f(pos.x + arrow_len / 2, pos.y, pos.z + len - arrow_len);

        rlVertex3f(pos.x, pos.y, pos.z + len);
        rlVertex3f(pos.x - arrow_len / 2, pos.y, pos.z + len - arrow_len);

    rlEnd();

    DrawTextCodepoint3D(font, 'x', (Vector3) {pos.x + len, pos.y, pos.z}, 10, true, false, color);
    DrawTextCodepoint3D(font, 'y', (Vector3) {pos.x, pos.y + len, pos.z}, 10, true, false, color);
    DrawTextCodepoint3D(font, 'z', (Vector3) {pos.x, pos.y, pos.z + len}, 10, true, false, color);
    const char *pos_str = TextFormat("(%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    pos.z -= 0.4;
    DrawText3D(font, pos_str, pos, 10, 1, 1, true, color);

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


