#ifndef _LENVIEW_H_
#define _LENVIEW_H_
#include <opencv2/opencv.hpp>
#include <stdarg.h>
#include "display_opengl.h"
#include "file_parse.h"
#include "microlens_model.h"
#include "ray_trace.h"

#define BGR 0.05f
#define BGG 0.05f
#define BGB 0.15f
#define SBFONT GLUT_BITMAP_9_BY_15
#define BARC 0.75f
#define BART 0.0f
#define BARH 25
using namespace std;

class lens_window
{
public:

    bool  init();
    int   windows_id;
    float lvZoom   = 8.0f;
    float lvTransX = 0.0f;
    float lvTransY = 0.0f;	
    int   lastx = 0;
    int   lasty = 0;
	bool  display= true;
    int   lvWidth,lvHeight;
    bool  lvPressed[3] = {false,false,false};
    bool  bg = false;//bg==1 white, bg==0 dark trigger by key 'b'
    bool  save = false;//bg==1 white, bg==0 dark trigger by key 'b'
    //---optics setup
    Point lviPlane;  //support it is squre
    Point lvoPlane;  //support it is squre
    bool opengl_image_save();
private:  
    bool opengl_window_init();//config_data *data

};



#endif

