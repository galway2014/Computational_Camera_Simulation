#ifndef _MICRO_LENSMODEL_H_
#define _MICRO_LENSMODEL_H_
#include "geometry.h"
#include <iostream>
#include <cstring>
using namespace std;

typedef struct {

float f;
float r;
float z;
int   w;
int   h;

}ML;
extern ML ml;
//cout<<"ml  "<<ml.r<<"  "<< ml.f <<endl;
inline void microlens_refraction(Ray* r){

 float t= (ml.z-r->o.z)/r->d.z;     
 r->o= r->o + t*r->d;
 
 int sign_x,sign_y;
 if (r->o.x >0) sign_x=1; else sign_x=-1;
 if (r->o.y >0) sign_y=1; else sign_y=-1;  

 int idx=int((r->o.x+sign_x*ml.r)* 0.5/ml.r);
 int idy=int((r->o.y+sign_y*ml.r)* 0.5/ml.r);
 
 float shift_exit_pupil_x= idx*2*ml.r;
 float shift_exit_pupil_y= idy*2*ml.r;

 if ((abs(idx)<=ml.w)&&(abs(idy)<=ml.h)){
    float abs_x=r->o.x-(shift_exit_pupil_x); //transfer to lenslet orintation  
    float abs_y=r->o.y-(shift_exit_pupil_y); //transfer to lenslet orintation

    r->d.x=-abs_x/ml.f+(-r->o.x)/(r->o.z);
    r->d.y=-abs_y/ml.f+(-r->o.y)/(r->o.z);
    r->d.z= -1;
    r->d=Normalize(r->d);
 }
 else {
     r->maxt = 0;
     r->mint = 1;
 }
/*
 int n= int(r->o.x/(2*ml.r));
 int m= int(r->o.y/(2*ml.r));
 //if ((abs(m)<50)&&(abs(n)<50)) cout<<m<<" "<<n<<endl;
 float offset_x=r->o.x-n*2*ml.r;
 float offset_y=r->o.y-m*2*ml.r;

 r->d.x=(-offset_x)/ml.f+r->d.x/r->d.z;
 r->d.y=(-offset_y)/ml.f+r->d.y/r->d.z; 
 r->d=  Normalize(r->d);//(*rays_chains)[k][num].d;
*/        
               
}


#endif
