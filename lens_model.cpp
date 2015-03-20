#include "lens_model.h"
#define LVSD 24 //arc resolution
#include "lens_view.h"
extern lens_window lens_w;
extern float* debug_y;
extern float* debug_z;

void lensElement::Draw(int offset_z) {

	float nextX, nextY, thisX, thisY;
	thisY = -this->aper / 2.0f;
	float rs = this->rad / fabs(this->rad);
	int i;
	
	float step = this->aper / LVSD;	//#define LVSD 24 arc resolution
	float posx = sqrt(rad*rad - thisY*thisY);
	thisX = rs * posx - rad + zpos;
	
	for (i = 1; i <= LVSD; i++) {
	  nextY = thisY + step;
	  posx = sqrt(rad*rad - nextY*nextY);
	  nextX = rs * posx - rad + zpos;
          //drawLine(0, 0, -50, 50);
	  drawLine(thisX, thisY+offset_z, nextX, nextY+offset_z);
	  thisX = nextX; thisY = nextY;
	}
	this->edge[0] = thisX;   //edge[X] = z edge[Y] = y, are used in drawing
	this->edge[1] = thisY;
	//cout<<"draw  "<<thisX<<endl;

}


bool lensSystem::load(const char *file) {
ifstream lensfile(file);
if (!lensfile) return false;
     
this->lenses.clear();
// Iterate over the lens element specs and build
// the lens system
float dist = 0.0f;
float temp =0;  //
while (!lensfile.eof()) {
    lensElement lens; 
    string data;

    lensfile >> data;
	// Remove comments and empty lines
	if (!data.compare("#") || !data.compare("")) {
	getline(lensfile, data);
	continue;
	}
	float rad  = (float)atof(data.c_str());

	lensfile >> data;
	float zpos = temp;
	temp=(float)atof(data.c_str());
	//cout<<zpos<<endl;
	float axpos = dist - zpos; 
	lensfile >> data;
	float ndr  = (float)atof(data.c_str());
	lensfile >> data;
	float aper = (float)atof(data.c_str());
	if (!aper && !rad && !zpos && !ndr) 
			return false;

	// Initialize the lens interface and store
	lens.Init(rad, axpos, ndr, aper);		
	this->lenses.push_back(lens);
	dist -= zpos;

}
        sys_len=fabs(dist);

return true;
}

bool lensSystem::parse_spec(const char *file) {
	ifstream infile;
	infile.open(file, ifstream::in);
	if(!infile)	return false;
	// Read lens spec file (from outside to inside)
	vector<float> radius, axpos, N, aperture,posZ;
	int nSurfaces = 0;
	float posz=0;
	char line[256];
	infile.getline(line, 256);
	while(!infile.eof()) {
		if(line[0] != '#') {
			float r, p, n, a;
			sscanf(line, "%f%f%f%f", &r, &p, &n, &a);
                        
			radius.push_back(r);
			axpos.push_back(p);
			N.push_back(n);
			if(n == 0.f) aperture.push_back(a);//same
			else	     aperture.push_back(a);
			totalFilmDist += p;
			posz-=p; // sign convention!
                        posZ.push_back(posz);
			nSurfaces++;
		}
		infile.getline(line, 256);
	}
	infile.close();
        //rear to front 
	// Push into lens surface datastructure (from inside to outside)
	axpos[nSurfaces - 1] = filmDist;
	float total = -totalFilmDist;

	for(int i = nSurfaces - 1; i >= 0; i--) {
		LensSurface ls;	
		ls.radius = radius[i];
		ls.radiusSquared = radius[i] * radius[i];
		total += axpos[i];
		ls.centerZ = total - radius[i];
                ls.posZ=(i==0)?0:posZ[i-1];
		ls.index=N[i];
		ls.thick=axpos[i];
		if(N[i] == 0.f) {
			ls.isStop = true;//bug
			Aperture_num=i;
			ls.nRatio = 1.f;
			ls.nRatioSquared = 1.f;
		}
		else {
			ls.isStop = false;
			ls.nRatio = (i > 0 && N[i - 1] != 0.f) ? (N[i] / N[i - 1]) : N[i];
			ls.nRatioSquared = ls.nRatio * ls.nRatio;
		}
		ls.apRadius = aperture[i] * 0.5f;
		ls.apRadiusSquared = ls.apRadius * ls.apRadius;
		lensSurfaces.push_back(ls);		
	}
        int size=lensSurfaces.size();
        stop_radius=lensSurfaces[size-Aperture_num-1].apRadius;
	return true;	
}


float lensSystem::maxAperture(void){
    float dist = 0.0f;
    vector<lensElement>::iterator Li;
    for (Li = lenses.begin(); Li != lenses.end(); Li++) 
        if (Li->aper > dist) 
        dist = Li->aper;
        return dist;
    }

// Method to get the aperture interface
lensElement* lensSystem::getAperture(void) {
    int i;
    for (i = 0; i < numLenses(); i++) 
	if (lenses[i].isStop)
	return (&lenses[i]);
    return NULL;
}

void lensSystem::calImagePlane(){
    
float ff1,ff2,pp1,pp2;
calF1F2P1P2(0,lensSurfaces.size()-1,ff1,ff2,pp1,pp2);

// find the image point of object point
float inv_focal_length    = 1/ (ff2 - pp2);
float t                   =     pp2 - pp1;
	
Matrix4x4 thick_matrix
       (1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1+(inv_focal_length*t), t,
	0, 0, inv_focal_length,  1);
        
Point object_pt=lens_w.lvoPlane;
Point image_pt;
Transform thicklens_transfer(thick_matrix); 
	 
object_pt.z=object_pt.z-pp1;      // coordinate transfer
//cout<<"-oplane-"<<object_pt<<endl;
thicklens_transfer(object_pt,&image_pt); // thick lens transformation
//cout<<"-implane-"<<image_pt<<endl;    
image_pt.z=image_pt.z+pp1;   // coordinate transfer back
		 
lens_w.lviPlane=Point(image_pt.x,image_pt.y,image_pt.z); 

}

void lensSystem::calExitPupil(){

int size=lensSurfaces.size();

float stop_r = stop_radius;//lensSurfaces[size-Aperture_num-1].apRadius; // aperture size

float stop_z = lensSurfaces[size-Aperture_num-1].posZ; //stop z position

if (size > 5){
	float ff1,ff2,pp1,pp2;
	calF1F2P1P2(Aperture_num+1,size-1,ff1,ff2,pp1,pp2);

	// find the image point of aperture
	float inv_focal_length    = 1/ (ff2 - pp2);
	float t                   =     pp2 - pp1;
	//cout<<"t and inv_focal_length "<<inv_focal_length<<" "<<t<<endl;
		Matrix4x4 thick_matrix
		           (1, 0, 0, 0,
		            0, 1, 0, 0,
		            0, 0, 1+(inv_focal_length*t), t,
		            0, 0, inv_focal_length,  1);
		    
	Point object_aperture(stop_r,stop_r, stop_z);
	//cout<<"object_aperture "<<object_aperture<<" "<<t<<endl;
	Point image_aperture;
	Transform thinklens_transfer(thick_matrix); 
		 
	object_aperture.z=object_aperture.z-pp1;      // coordinate transfer
	thinklens_transfer(object_aperture,&image_aperture); // thick lens transformation
	image_aperture.z=image_aperture.z+pp1;   // coordinate transfer back
		 
	pupil_ex=Point(image_aperture.x,image_aperture.x,image_aperture.z);
}

else pupil_ex=Point(stop_r,stop_r,stop_z);
}


void lensSystem::calEntrancePupil(){

int size=lensSurfaces.size();
float stop_r = stop_radius;//lensSurfaces[size-Aperture_num-1].apRadius; // aperture size
float stop_z = lensSurfaces[size-Aperture_num-1].posZ; //stop z position

if (size > 5){
float ff1,ff2,pp1,pp2;
calF1F2P1P2(0,Aperture_num-1,ff1,ff2,pp1,pp2);

// find the image point of aperture
float inv_focal_length    = 1/ (ff2 - pp2);
float t                   =     pp2 - pp1;
//cout<<"t and inv_focal_length "<<inv_focal_length<<" "<<t<<endl;
	Matrix4x4 thick_matrix
               (1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, -t,
                0, 0, -inv_focal_length, 1+(inv_focal_length*t));
        

Point image_aperture(stop_r,stop_r, stop_z);
//cout<<"object_aperture "<<object_aperture<<" "<<t<<endl;
Point object_aperture;
Transform thinklens_transfer(thick_matrix); 
	 
image_aperture.z=image_aperture.z-pp1;      // coordinate transfer
thinklens_transfer(image_aperture,&object_aperture); // thick lens transformation
object_aperture.z=object_aperture.z+pp1;   // coordinate transfer back
	 
pupil_en=Point(object_aperture.x,object_aperture.x,object_aperture.z);
}

else pupil_en=Point(stop_r,stop_r,stop_z);
}

//Given all the setting, OUTPUT the f1,f2,p1,p2
void lensSystem::calF1F2P1P2(int start, 
                             int end, 
                             float& ff1,
                             float& ff2,
                             float& pp1,
                             float& pp2) {

int size=lensSurfaces.size();

if (size>5){

	//lensSurfaces is from rear to front, u,h,n,d,c is loaded from front to rear
	float u[size],h[size-1],n[size],d[size-1],c[size-1];
	for(int i = 0; i <= size-1; i++) {	
		d[i]  = lensSurfaces[size-i-1].thick;
		c[i]  =(lensSurfaces[size-i-1].isStop)? 0: 1/lensSurfaces[size-i-1].radius;
		n[i+1]=(lensSurfaces[size-i-1].isStop)? 1: lensSurfaces[size-i-1].index;
	}

	//forward paraxial ray tracing
	//initial position
	u[start]=0.f;
	h[start]=1;
	n[start]=1.f;

	for (int i=start; i<=end;i++){
	   float K=(n[i+1]-n[i])*(c[i]);
	   u[i+1]= (n[i+1]<0.01)? u[i]:( -h[i]* K + n[i]*u[i]  ) / n[i+1];
	   if (i<end) h[i+1]= h[i]+d[i] * u[i+1];
	}

	ff2  = h[end]/u[end+1]+lensSurfaces[size-1-end].posZ ; //  location of focal point, lenssurfaces is in  reverse order!
	//cout<<"final result "<<f2<<endl;
	pp2 =  ff2 - h[start]/u[end+1];

	//backward paraxial ray tracing
	//initial position
	u[end+1]=0.f;
	h[end]  =1;
	n[end+1]=1.f;

	for (int i=end; i>=start;i--)
	{
	   float K=(n[i]-n[i+1])*(-c[i]);
	   u[i]= (n[i]<0.01)? u[i+1]:( -h[i]* K + n[i+1]*u[i+1] ) / n[i];
	   if(i>start) h[i-1]= h[i]+d[i-1] * u[i];

	}
	ff1 =-h[start]/(u[start])+lensSurfaces[size-1-start].posZ;
	pp1 = ff1 + h[end]/(u[start]);
}
}

void lensSystem::draw(void) {
	
	//int size=lensSurfaces.size();
        if (lens_w.bg==true) 
            glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
        else
            glColor3f(0.7, 0.7, 0.7);       
        
        glLineWidth(1);
	vector<lensElement>::iterator Li, Ln;
	
	//The curve for the Lens
	for (Li = lenses.begin(); Li != lenses.end(); Li++)
		if (Li->isActive && !Li->isStop) 
			Li->Draw(0);

	for (Li = lenses.begin(); Li != lenses.end() - 1; Li++) {
		Ln = Li + 1; //!!!

		if ((Ln->isStop || Li->isStop) || Li->isAir() || !Li->isActive) 
			continue;

		float lip[2] = {Li->edge[0], Li->edge[1]};
		float lnp[2] = {Ln->edge[0], Ln->edge[1]};
		 		//cout<<lip[0]<<" "<<lip[1]<<endl;
		 		//cout<<lnp[0]<<" "<<lnp[1]<<endl;
		if (Li->aper < Ln->aper) {
			drawLine(lip[0],  lip[1], lip[0],  lnp[1]);
			drawLine(lnp[0],  lnp[1], lip[0],  lnp[1]);	
			drawLine(lip[0], -lip[1], lip[0], -lnp[1]);
			drawLine(lnp[0], -lnp[1], lip[0], -lnp[1]);	
		} else {
			drawLine(lip[0],  lip[1], lnp[0],  lip[1]);
			drawLine(lnp[0],  lnp[1], lnp[0],  lip[1]);
			drawLine(lip[0], -lip[1], lnp[0], -lip[1]);
			drawLine(lnp[0], -lnp[1], lnp[0], -lip[1]);
		}
	}
   	if (lens_w.display==true){     
	//Draw Image and Object Plane

		// Draw the aperture stop interface
		lensElement *stop = this->getAperture(); 
		float maxp = maxAperture();
		float haxp = stop_radius;
		float taxp = haxp + maxp / 2.0f;
		if (lens_w.bg==true) 
		        glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
		    else
		        glColor3f(0.7, 0.7, 0.7);   
		glLineWidth(2);
	
		drawLine(stop->zpos,  haxp, stop->zpos,  taxp);
		drawLine(stop->zpos, -haxp, stop->zpos, -taxp);
		    
		glLineWidth(1);
		    
		//Draw the exit pupil
		Point ep = Point (pupil_ex.x, pupil_ex.x, pupil_ex.z);
                //cout<<"exit pupil is "<<ep<<endl;
		float hep = ep.y; 
		float tep = hep + maxp/2.0f; 
		float zep = ep.z; 
		glColor3f(0, 1, 0);
		glLineWidth(2);
		    drawLine(zep,  hep, zep,  tep);
		drawLine(zep, -hep, zep, -tep);		
		    drawText(ep.z , -25,  SBFONT, "Ex"); 
		    //Draw the entrance pupil
		ep = Point (pupil_en.x, pupil_en.x, pupil_en.z);
                //cout<<"entrance pupil is "<<ep<<endl;
		hep = ep.y; 
		tep = hep + maxp/2.0f; 
		zep = ep.z; 
		glColor3f(0, 1, 0);
		glLineWidth(2);
		    drawLine(zep,  hep, zep,  tep);
		drawLine(zep, -hep, zep, -tep);		
		    drawText(ep.z , -25,  SBFONT, "En"); 
	   
		    //a. P and P' 
		    glColor3f(1, 0, 0);
		    glLineWidth(2);
		    drawLine(p1, -20 ,   p1, 20);	
		    drawLine(p2, -20 ,   p2, 20);	
		    drawText(p1, -20 -10, SBFONT, "P"); 
		    drawText(p2, -20 -10, SBFONT, "P'"); 

		    glLineWidth(1);

		    //b. F and F'    //by lsystem.draw()
		    //--drawSquare(1,1,1, f1 - 1, - 1  , f1 + 1 ,  1);	
		    //--drawSquare(1,1,1, f2 - 1, - 1  , f2 + 1 ,  1);	
		    //--drawText(f1 , -10,  SBFONT, "F"); 
		    //--drawText(f2 , -10,  SBFONT, "F'"); 
    }    
	glLineWidth(1);
        if (lens_w.bg==true) 
            glColor3f(BGR, BGG, BGB);//BGx=Back Ground x is channel
        else
            glColor3f(0.7, 0.7, 0.7);   
}


