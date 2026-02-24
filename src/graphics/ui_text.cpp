#include "graphics/ui_text.h"
#include <GL/glew.h>
#include <GL/glut.h>

void uiDrawStrokeText(float x, float y, const char* text, float scale)
{
    // Save the caller's color before the shadow changes it
    GLfloat savedColor[4];
    glGetFloatv(GL_CURRENT_COLOR, savedColor);

    glPushAttrib(GL_LINE_BIT);

    // Shadow pass (dark drop-shadow for contrast)
    glPushMatrix();
    glTranslatef(x + 2.0f, y - 2.0f, 0.0f);
    glScalef(scale, scale, 1.0f);
    glLineWidth(2.5f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
    for (const char* c = text; *c; ++c)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();

    // Main text pass — use the caller's original color
    glColor4fv(savedColor);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glLineWidth(3.0f);
    for (const char* c = text; *c; ++c)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();

    glPopAttrib();
}

float uiStrokeTextWidth(const char* text)
{
    float w = 0.0f;
    for (const char* c = text; *c; ++c)
        w += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
    return w;
}

float uiStrokeTextWidthScaled(const char* text, float scale)
{
    return uiStrokeTextWidth(text) * scale;
}
