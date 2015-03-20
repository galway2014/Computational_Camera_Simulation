#include "lens_model.h"
#include "lens_view.h"
#include "image_view.h"
#include "ray_trace.h"
#include "file_parse.h"
#include "microlens_model.h"
config_data cfg_data;
config cfg;

lens_window lens_w;
image_window image_w;
lensSystem  lens_sys;
Ray_Tracer rt;
ML          ml;
int interactive;
int microlens;
int autofocus;



int main(int argc, char** argv) {

///load configuration fiels
cfg.parse_command_line(argc,argv);
parse_config_data(cfg,cfg_data);

///init opengl
glutInit(&argc, argv);
cout<<"step 1"<<endl;
///init lens view window
lens_w.init();
cout<<"step 2"<<endl;
///init image rendering window
image_w.init();
cout<<"step 3"<<endl;
///init ray tracer
rt.init();
cout<<"step 4"<<endl;
//update display
glutMainLoop();
return 0;
}

