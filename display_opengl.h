#ifndef _DISPLAY_OPENGL_H_
#define _DISPLAY_OPENGL_H_

#include <GL/glew.h>
#include <GL/freeglut.h>

static inline void drawLine(float x1, float y1, 
							float x2, float y2) 
{ 
   glBegin(GL_LINES); 
   glVertex2f(x1, y1); glVertex2f(x2, y2); 
   glEnd(); 
}

static inline void drawPoint(float size, float x, float y) 
{ 
    glPointSize(size); 
    glBegin(GL_POINTS); 
    glVertex2f(x, y); 
    glEnd(); 
    glPointSize(1.0f); 
}

static inline void drawSquare(float r, float g, float b, 
							  GLint xl, GLint yl, 
							  GLint xr, GLint yr) 
{ 
    glBegin(GL_QUADS); 
    glColor3f(r, g, b); 
    glVertex2i(xl, yl); 
    glVertex2i(xl, yr); 
    glVertex2i(xr, yr); 
    glVertex2i(xr, yl); 
    glEnd(); 
}

static inline void drawText(float x, float y, void *font,
							char* text)
{
	char *p;
	glRasterPos2f(x, y);
	for (p = text; *p; ++p) {
		glutBitmapCharacter(font, *p);
	}
}

//change the axis direction
static void applyTransforms(int lvWidth, 
                            int lvHeight,
                            float lvTransX,
                            float lvTransY, 
                            float lvZoom) {
	glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	float orthoX = lvWidth  / lvZoom;
	float orthoY = lvHeight / lvZoom;

	gluOrtho2D(orthoX, -orthoX, -orthoY, orthoY);
        glTranslated(lvTransX, lvTransY, 0.0f);
}

#endif
