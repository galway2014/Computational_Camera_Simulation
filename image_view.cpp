#include "image_view.h"

extern config_data cfg_data;
extern config cfg;
extern image_window image_w;
extern lens_window lens_w;
extern int interactive;

GLuint compileASMShader(GLenum program_type, const char *code)
{
    GLuint program_id;
    glGenProgramsARB(1, &program_id);
    glBindProgramARB(program_type, program_id);
    glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);

    GLint error_pos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);

    if (error_pos != -1)
    {
        const GLubyte *error_string;
        error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        printf("Program error at position: %d\n%s\n", (int)error_pos, error_string);
        return 0;
    }
    return program_id;
}



static void render_image(void)
{	
    // execute filter, writing results to pbo
    unsigned int *dResult;
    //cout<<"rendering image"<<endl;
    size_t num_bytes;
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // load texture from pbo
        //glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo); --for cuda?
        glBindTexture(GL_TEXTURE_2D, image_w.texid);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                        image_w.w,  image_w.h, 
                        GL_RGBA, GL_UNSIGNED_BYTE, 
                        image_w.hImage);
        //glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0); --for cuda?

        // fragment program is required to display floating point texture
        //glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, shader); --for cuda?
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
        glDisable(GL_DEPTH_TEST);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0, 0);
            glVertex2f(0, 0);
            glTexCoord2f(1, 0);
            glVertex2f(1, 0);
            glTexCoord2f(1, 1);
            glVertex2f(1, 1);
            glTexCoord2f(0, 1);
            glVertex2f(0, 1);
        }
        glEnd();
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
    }

    glutSwapBuffers();
    glutReportErrors();
}

void reshape(int x, int y) //change view point from center to left lower coner
{
    glViewport(0, 0, x, y); 

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
}

void cleanup()
{
    if (image_w.hImage)
        free(image_w.hImage);
    
    glDeleteBuffersARB (1, &image_w.pbo);
    glDeleteTextures   (1, &image_w.texid);
    glDeleteProgramsARB(1, &image_w.shader);
}

static void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: /* Escape key */ exit(0);
        	case 'b': 
                        image_w.bg=!image_w.bg;  
                        break;
                case 'm': 
                        if (image_w.mode==0) image_w.mode=1;
                        else if (image_w.mode==1) image_w.mode=2;
                        else image_w.mode=0;                        
                        break;
		case '=':
			image_w.zoom=image_w.zoom*1.1;
			//cout<<image_w.zoom<<endl;
			break;
		case '-':
			image_w.zoom=image_w.zoom*0.9;
			//cout<<image_w.zoom<<endl;
			break;
		case 'a'://down
			image_w.ox=image_w.ox+0.01;
			break;
		case 's'://up
			image_w.ox=image_w.ox-0.01;
			break;
		case 'd'://up
			image_w.oy=image_w.oy+0.01;
			break;
		case 'f'://right
			image_w.oy=image_w.oy-0.01;
			break;
                case 'o': // decrese image plane
			if (lens_w.lviPlane.z  <  800)  lens_w.lviPlane.z += 0.1;
			break;	
		case 'p': // increase fstop
			if (lens_w.lviPlane.z  > -800)  lens_w.lviPlane.z -= 0.1;
			break;
		case 'k': // decrese image plane
			if (lens_w.lviPlane.z  <  800)  lens_w.lviPlane.z += 0.02;
			break;	
		case 'l': // increase fstop
			if (lens_w.lviPlane.z  > -800)  lens_w.lviPlane.z -= 0.02;
			break;                                                                   
                case 'z'://right
			image_w.save=true;
			break;	        
		default:break;
	}
	
	glutSetWindow( image_w.windows_id );
	glutPostRedisplay();  // Update screen with new rotation data
}

static void Motion(int x, int y) {
	glutPostRedisplay();
        //cout<<"click"<<endl;
}

static void lvClick(int button, int state, int x, int y) {
	glutPostRedisplay();
}


bool image_window::opengl_window_init(){
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(w, h);
    this->windows_id =glutCreateWindow("Image Rendering");
    glutDisplayFunc(render_image);
    glutMotionFunc(Motion);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(lvClick);
    glutIdleFunc(NULL);
    //glutTimerFunc(REFRESH_DELAY, timerEvent, 0);

    glewInit();

    // create pixel buffer object
    glGenBuffersARB(1, &pbo);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, w*h*sizeof(GLubyte)*4, hImage, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    // create texture for display
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // load shader program
    shader = compileASMShader(GL_FRAGMENT_PROGRAM_ARB, (const char*)shader_code); 
return true;
}

bool image_window::init(){
    hImage   = new unsigned int[w*h];// unsigned is 32 bit( alpha R G B )
    image_cv =  cv::Mat::zeros(h,w,CV_32F);
    
    if (interactive==1) {

        shader_code = "!!ARBfp1.0\n"
        "TEX result.color, fragment.texcoord, texture[0], 2D; \n"
        "END";
        this->opengl_window_init();
        glutCloseFunc(cleanup);
    }     
    return true;
}

