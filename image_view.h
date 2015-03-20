#ifndef _IMAGEVIEW_H_
#define _IMAGEVIEW_H_
#include "file_parse.h"
#include "display_opengl.h"
#include "lens_view.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <cstring>

typedef struct // to represent a pixel using vertex 
{
    unsigned char x, y, z, w; // x=R, y=R, z=B, w=alpha
} uchar4;

class image_window
{
public:

    bool  init();
    int   windows_id;
    int   w,h;
    float ray_w,ray_h;//the actual range of rays to be rendered
    bool  bg   = false; //false= dark true= white
    uchar  mode = false; //false= ray, true= image 
    bool  save = false;//bg==1 white, bg==0 dark trigger by key 'b'    
    unsigned int  *hImage;//image buffer
    cv::Mat       image_cv;//not initialized
    bool opengl_window_init();//config_data *data
    GLuint pbo;     // OpenGL pixel buffer object
    GLuint texid;   // texture
    GLuint shader;
    char *shader_code;
    float zoom=1;
    float ox=0;
    float oy=0;
};


#endif

