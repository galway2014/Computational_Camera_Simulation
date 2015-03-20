#include "lens_view.h"
#include "lens_model.h"
extern lens_window lens_w;
extern image_window image_w;
extern lensSystem  lens_sys;
extern Ray_Tracer rt;
extern int interactive;
extern int microlens;
extern int autofocus;
extern ML ml;

static void lvText(int x, int y, void *font, int height, 
				   char *format, ...) {
    va_list args; char buffer[256];
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    glMatrixMode(GL_PROJECTION); 
	glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, lens_w.lvWidth - 1, 0, lens_w.lvHeight - 1);
    glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); glLoadIdentity();

    if ((x >= 0) && (y >= 0)) 
		glRasterPos2i(x,y);
    for (char *p = buffer; *p; ++p) 
		glutBitmapCharacter(font, *p);
   
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

static void lvClick(int button, int state, int x, int y) {
	if (state == GLUT_DOWN)	
		lens_w.lvPressed[button] = true;
	else if (state == GLUT_UP) 
		lens_w.lvPressed[button] = false;

	lens_w.lastx = x; lens_w.lasty = y; 
	glutPostRedisplay();
}


static void lvMotion(int x, int y) {
	// Here we handle translation
	float norm = 1.0f / lens_w.lvZoom;
	if (lens_w.lvPressed[GLUT_LEFT_BUTTON]) {
		lens_w.lvTransX += float(lens_w.lastx - x)*2.0f*norm; 
        lens_w.lvTransY += float(lens_w.lasty - y)*2.0f*norm;
	}
	if (lens_w.lvPressed[GLUT_MIDDLE_BUTTON]) {
		// Use this to change camera focus
	}
	// Here we handle zooming
	if (lens_w.lvPressed[GLUT_RIGHT_BUTTON]) {
	    float zoom = 3.0f * float(x - lens_w.lastx) / lens_w.lvWidth;
	    if ((lens_w.lvZoom + zoom) >= 1.0f)
			 lens_w.lvZoom += zoom;
	}

	lens_w.lastx = x; lens_w.lasty = y; 
	glutPostRedisplay();
}

static void lvResize(int w, int h) {
    lens_w.lvWidth  = w; 
    lens_w.lvHeight = h;
    glViewport(0, 0, lens_w.lvWidth, lens_w.lvHeight);
}


static void lvKey(unsigned char key, int x, int y) {
    switch (key) {
        case 27: /* Escape key */ exit(0);
        	case 'b': // toggle background
                        lens_w.bg=!lens_w.bg;
                        image_w.bg=!image_w.bg;                        
                        //cout<<lens_w.bg<<endl;
                        break;
		case 's': // toggle save mode
	                lens_w.save=true;
			break;
        case 'm': break;
        case 'd': lens_w.display=!lens_w.display;
				  break;	                   
		case ',': // decrease fstop
			lens_sys.stop_radius -= 0.2;
                        lens_sys.calEntrancePupil();
                        lens_sys.calExitPupil();
			break;
		case '.': // increase fstop
			lens_sys.stop_radius += 0.2;
                        lens_sys.calEntrancePupil();
                        lens_sys.calExitPupil();                        
			break;	
                case 'o': // decrese image plane
			if (lens_w.lviPlane.z  <  0)  lens_w.lviPlane.z += 0.1;
			break;	
		case 'p': // increase fstop
			if (lens_w.lviPlane.z  > -800)  lens_w.lviPlane.z -= 0.1;
			break;
		case 'k': // decrese image plane
			if (lens_w.lviPlane.z  <  0)  lens_w.lviPlane.z += 0.02;
			break;	
		case 'l': // increase fstop
			if (lens_w.lviPlane.z  > -800)  lens_w.lviPlane.z -= 0.02;
			break;
		case 'u': // decrese image plane
			if (lens_w.lvoPlane.z  < 10000)  lens_w.lvoPlane.z += 5;
			break;	
		case 'i': // increase fstop
			if (lens_w.lvoPlane.z  > 0)  lens_w.lvoPlane.z -= 5;
			break;
		case 'h': // decrese image plane
			if (lens_w.lvoPlane.z  < 10000)  lens_w.lvoPlane.z += 0.5;
			break;	
		case 'j': // increase fstop
			if (lens_w.lvoPlane.z  > 0)  lens_w.lvoPlane.z -= 0.5;
			break;                               
		case 't': // decrease microlens distance
			ml.z-= 0.1;
			break;                               
		case 'y': // increase microlens distance
			ml.z+= 0.1;
			break;	                        
		default: break;
	}
	
	glutSetWindow( lens_w.windows_id );
	glutPostRedisplay();  // Update screen with new rotation data
}



static  void lvDisplay(void) {
//cout<<"lvdisplay"<<endl;
glClear(GL_COLOR_BUFFER_BIT);
applyTransforms(lens_w.lvWidth,lens_w.lvHeight,lens_w.lvTransX,lens_w.lvTransY,lens_w.lvZoom);

if (lens_w.bg==true) 
    glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
else
    glColor3f(0.7, 0.7, 0.7);
glLineWidth(1);
if (lens_w.display==true){
drawLine(-lens_w.lvWidth-lens_w.lvTransX, 0, 
          lens_w.lvWidth-lens_w.lvTransX, 0);
drawLine(0, -lens_w.lvHeight-lens_w.lvTransY, 
         0,  lens_w.lvHeight-lens_w.lvTransY);
glLineWidth(1);
}
///drawLine(-20, -20, 0, 0);
lens_sys.draw();

if (lens_w.display==true){
drawLine(-lens_w.lvWidth-lens_w.lvTransX, 0, 
          lens_w.lvWidth-lens_w.lvTransX, 0);
}
rt.draw();

glMatrixMode(GL_PROJECTION); glPushMatrix();
glLoadIdentity();
gluOrtho2D(0, lens_w.lvWidth, lens_w.lvHeight, 0);
glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
if (lens_w.display==true){
drawSquare(0.25f, 0.25f, 0.25f, 0, 0, lens_w.lvWidth, BARH);
drawSquare(0.25f, 0.25f, 0.25f, 0, lens_w.lvHeight, lens_w.lvWidth, lens_w.lvHeight-BARH);
}
glLoadIdentity();
if (lens_w.display==true){
gluOrtho2D(-lens_w.lvWidth, lens_w.lvWidth, lens_w.lvHeight, -lens_w.lvHeight);
glColor3f(0, 1, 1);
drawText(-lens_w.lvWidth + 15, -lens_w.lvTransY*lens_w.lvZoom - 8, SBFONT, "+z");
drawText( lens_w.lvWidth - 50, -lens_w.lvTransY*lens_w.lvZoom - 8, SBFONT, "-z");
drawText(-lens_w.lvTransX*lens_w.lvZoom, -lens_w.lvHeight + 75, SBFONT, "+y");
drawText(-lens_w.lvTransX*lens_w.lvZoom,  lens_w.lvHeight - 65, SBFONT, "-y");
}	
if (lens_w.bg==true) 
    glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
else
    glColor3f(0.7, 0.7, 0.7);
//lvText(9, lens_w.lvHeight-16, SBFONT, 0, "fstop %1.1f (aperture %1.1fmm) | "
//		                              "object %1.1fmm | image %1.1fmm | mode: none"); 
//	                               lsystem.fstop, cur_aperture, lvoPlaneZ ,lviPlaneZ); 
if (lens_w.display==true){
lvText(9, 8, SBFONT, 0, "F %1.1fmm | F' %1.1fmm | P %1.1fmm | "
       "P' %1.1fmm | entrance pupil (%1.1fmm, %1.1fmm)| exit pupil (%1.1fmm, %1.1fmm) | "
       "F number %1.2f",
                     lens_sys.f1,lens_sys.f2,lens_sys.p1,lens_sys.p2,
                     lens_sys.pupil_en.z,  2*lens_sys.pupil_en.y,
                     lens_sys.pupil_ex.z,  2*lens_sys.pupil_ex.y,
                      2*2*lens_sys.pupil_en.y/(lens_sys.f1-lens_sys.f2));


        #define PLANE_HEIGHT  30 
        #define SQUARE_WIDTH  2
        #define OFFSET  10
        #define SBFONT GLUT_BITMAP_9_BY_15       
        
	glColor3f(1, 0, 1);
	glLineWidth(2);
	drawLine(lens_w.lviPlane.z, -PLANE_HEIGHT , lens_w.lviPlane.z, PLANE_HEIGHT);
	drawLine(lens_w.lvoPlane.z, -PLANE_HEIGHT , lens_w.lvoPlane.z, PLANE_HEIGHT);	
	drawText(lens_w.lviPlane.z, -PLANE_HEIGHT - OFFSET , SBFONT, "I");	
	drawText(lens_w.lvoPlane.z, -PLANE_HEIGHT - OFFSET , SBFONT, "O");		
	glLineWidth(1);
}
if (lens_w.bg==true) 
    glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
else
    glColor3f(0.7, 0.7, 0.7);

glLineWidth(2);

if (lens_w.display==true){
float bar=image_w.ray_h/image_w.zoom;
if (bar<1) bar=0.5f;
drawLine(lens_w.lviPlane.z-1, lens_w.lviPlane.y-bar/2, 
         lens_w.lviPlane.z-1, lens_w.lviPlane.y+bar/2);
drawText(lens_w.lviPlane.z-4, lens_w.lviPlane.y, SBFONT, "S");	
}
//To do: unable to display correct position for the first show 
glLineWidth(1);


if (microlens==1){//draw
    for (int i=-ml.w;i<=ml.w;i++)
        drawLine(ml.z, 0+i*ml.r*2-0.9*ml.r, 
                 ml.z, 0+i*ml.r*2+0.9*ml.r);
}

if (lens_w.bg==false) 
    glClearColor(BGR, BGG, BGB, 1);//BGx=Back Ground x is channel
else
    glClearColor(0.9, 0.9, 0.9, 1);

glPopMatrix();

if(lens_w.save == true) {
    lens_w.opengl_image_save();
    lens_w.save = false;
}
glutSwapBuffers();
glutPostRedisplay();
}

bool  lens_window::opengl_image_save(){
//To do: only works now for the initial resolution
glutSetWindow( lens_w.windows_id );  
int w=glutGet(GLUT_WINDOW_WIDTH);
int h=glutGet(GLUT_WINDOW_HEIGHT);
//cout<<w<<" "<<h<<endl;
cv::Mat screen = cv::Mat(h,w, CV_8UC3);
glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screen.data);
cv::cvtColor(screen, screen, cv::COLOR_RGB2BGR);
cv::flip(screen,screen,0);
cv::imwrite("screen.png", screen);
return true;
}


#define maxv(a,b) (((a) > (b))?(a):(b))
bool lens_window::opengl_window_init(){

this->lvZoom = maxv(1, this->lvZoom);    
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
glutInitWindowSize(lvWidth, lvHeight);

this->windows_id =glutCreateWindow("Lenses Ray Tracing View");
//applyTransforms(lens_w.lvWidth,lens_w.lvHeight,lens_w.lvTransX,lens_w.lvTransY,lens_w.lvZoom);
glutDisplayFunc(lvDisplay);
glutMotionFunc(lvMotion);
glutMouseFunc(lvClick);
glutKeyboardFunc(lvKey);
glutReshapeFunc(lvResize);

glutIdleFunc(NULL);

glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
glMatrixMode(GL_PROJECTION); glLoadIdentity();	
if (this->bg==false) 
    glClearColor(BGR, BGG, BGB, 1);//BGx=Back Ground x is channel
else
    glClearColor(0.9, 0.9, 0.9, 1);
return true;
}

bool lens_window::init(){

if (autofocus==1){
    lens_sys.calImagePlane();
}    
lens_sys.calExitPupil();
lens_sys.calEntrancePupil();
lens_sys.calF1F2P1P2(0,lens_sys.numLenses()-1,
                     lens_sys.f1,lens_sys.f2,
                     lens_sys.p1,lens_sys.p2);
opengl_window_init();

return true;
}
