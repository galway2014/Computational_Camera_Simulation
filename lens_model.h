#ifndef _LENSMODEL_H_
#define _LENSMODEL_H_

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream> 
#include <math.h>
#include "transform.h"
#include "display_opengl.h"

using namespace std;

class lensElement {
public:
    lensElement(void): 
	    isStop(true), rad(0), zpos(0), ndr(0), aper(0), isActive(false) {
		    edge[0] = edge[1] = 0.0f;
		}
	lensElement(float _rad, float _xpos, float _ndr, float _aper) {
		this->Init(_rad, _xpos, _ndr, _aper);
		edge[0] = edge[1] = 0.0f;
	}   
	// Method to initialize a lens interface object
	void Init(float _rad, float _xpos, float _ndr, float _aper) {
		this->isStop = false;
		rad = _rad; zpos = _xpos; 
		ndr = _ndr; aper = _aper;
		isActive = true;
		if (rad == 0.0f && ndr == 0.0f)
			this->isStop = true;
	}
	// Draw a single lens interfaces
	void Draw(int offset_z);
	bool isAir(void) { return (ndr == 1.0f)?true:false; }
	//bool refractOnce(CRay * outRay, const CRay * inRay, const float input_miu );
	//bool checkStop(CRay * outRay, const CRay * inRay, float m_fleffective_aperture);

    bool  isStop;   // Flag to identify the aperture stop
    float rad;      // Radius for the lens interface
	float zpos;     // Lens interface position (1D);
	float ndr;      // Index of refraction
	float aper;     // Aperture (diameter) of the interface
	bool  isActive; // Flag to set interface active
	float edge[2];  // Top edge point of each interface  //edge[X] = z edge[Y] = y, are used in drawing
};

struct LensSurface {
	bool  isStop;
	float thick;
	float index;
	float centerZ; // center of the surface sphere
	float posZ;// instersection of the lens 
	float radius, radiusSquared;
	float nRatio, nRatioSquared; // nInside / nOutside
	float apRadius, apRadiusSquared,apsubRadius_x1,apsubRadius_x2,apsubRadius_y1,apsubRadius_y2; // aperture / 2
};

class lensSystem {
public:
    vector<lensElement> lenses; // Vector of lens elements
    vector<LensSurface> lensSurfaces;  //
    lensElement *getAperture(void);
    void calExitPupil();        
    void calEntrancePupil();
    void calImagePlane();
    void calF1F2P1P2(int start, 
                     int end, 
                     float& ff1,
                     float& ff2,
                     float& pp1,
                     float& pp2);
    int  numLenses(void) { return int(lenses.size());}
    bool load(const char *file);         //
    bool parse_spec(const char *file); //Similar to load

    void draw(void);
    float maxAperture(void);
    Point pupil_ex;		// Exit pupil for the lens system
    Point pupil_en;		// Entrance pupil for the lens system
    float sys_len;              // total length of the system
    float f1, f2;               // Focal points for the lens
    float p1, p2;               // Locations of principle planes    
    int   Aperture_num;
    float stop_radius;
    float filmDist, filmDiag, totalFilmDist;//shan   
//private: 
	
};



#endif
