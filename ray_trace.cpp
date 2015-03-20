
#include "ray_trace.h"
#include "lens_view.h"

extern config_data  cfg_data;
extern config cfg;
extern Ray_Tracer   rt;
extern lensSystem   lens_sys;
extern lens_window  lens_w;
extern image_window image_w;
extern int interactive;
extern int microlens;
extern ML  ml;
void Light_source::init(){

if (interactive==1){
    Point pt_source;

    cfg.get_switch( "pt_x", pt_source.x);
    cfg.get_switch( "pt_y", pt_source.y);
    cfg.get_switch( "pt_z", pt_source.z);

    light_source_pt.push_back(pt_source);
}
else {
    int x_num, y_num, x_width, y_width, z_dist;
    cfg.get_switch( "pt_x_num",  x_num);
    cfg.get_switch( "pt_y_num",  y_num);
    cfg.get_switch( "pt_z_d" ,  z_dist);
    cfg.get_switch( "pt_x_w",  x_width);
    cfg.get_switch( "pt_y_w",  y_width);   

    float mag=(lens_w.lvoPlane.z-lens_sys.p1)/(lens_sys.p2-lens_w.lviPlane.z);
    
    for (int j=-3;j<=3;j++)
        for (int i=-5;i<=5;i++){
            Point pt_source;
            pt_source.x=i*(400*image_w.ray_w/image_w.w)*mag;
            pt_source.y=j*(400*image_w.ray_h/image_w.h)*mag;
            pt_source.z=z_dist;
            light_source_pt.push_back(pt_source);    
    }
    /*
    for (int j=-(y_num-1)/2;j<=(y_num-1)/2;j++)
        for (int i=-(x_num-1)/2;i<=(x_num-1)/2;i++){
            Point pt_source;
            pt_source.x=i*x_width;
            pt_source.y=j*y_width;
            pt_source.z=z_dist;
            light_source_pt.push_back(pt_source);                    
        }
    */ 
}

}

#define PI 3.1416



//when a or b = 0.5 output will be nan
Point todisk(Point s, float l){
    float phi,r;
    float a=2*s.x-1;
    float b=2*s.y-1;
    
    if(a*a>b*b){
            r=a;
            if (fabs(a)<0.000001) a=0.000001;
            phi=(PI/4)*(b/a);       
        }
    else{
            r=b;
            if (fabs(b)<0.000001) b=0.000001;
            phi=(PI/2)-(PI/4)*(a/b);
        }
    Point u;
    u.x=l*r*cos(phi);
    u.y=l*r*sin(phi);
    u.z=s.z;
    u.w=sqrt(u.x*u.x+u.y*u.y);
    return u;
}

//for object space use entrance pupil
//for image space use exit pupil
void Light_source::ray_gen_pupil(float radius, float z_axis,
                                 vector<Ray>& light_source,
                                 int num){
//total number is (sample_xxx+1)^2
float x,y;
float i,j;
float inc=1/(float) num;

//cv::Mat pupil_sample=cv::Mat::zeros(200,200,CV_8U);

for (int k=0;k<light_source_pt.size();k++){
    for (j=0; j<=num;j++){
        y=(float)j*inc;
        for (i=0; i<=num;i++){
            x=(float)i*inc;
            Point ex=todisk(Point(x,y,z_axis),radius);// todisk(Point(x,y,z_axis));
            
            //int idx_x2 =400* ( ex.x )/radius+400;
            //int idx_y2 =400* ( ex.y )/radius+400;
            //cout<<ex<<endl;
            //cout<< light_source_pt[k] <<endl;
            Vector r=ex-light_source_pt[k];
            Ray l;
            l.o = light_source_pt[k];
            l.d = Normalize(r);
            l.w= 1;//pupil_sample.at<uchar>(idx_y2,idx_x2);
            light_source.push_back(l);
        }   
    }
}
//cv::Mat cm_img;
// Apply the colormap:
//cv::applyColorMap(pupil_sample, cm_img, cv::COLORMAP_JET);
//cv::imwrite("pupil_sample.png",cm_img);
//cv::imwrite("pupil_sample2.png",255-pupil_sample);
}


//rays not ray_chains
bool Ray_Tracer::draw_rays(vector<Ray> rays)
{   
    vector<Ray>::iterator Ri, Rn;
    Ri = rays.end() - 1; 
       
    float len = 20;//to the z axis left 
    //draw all the rays except last one

    for (Ri= rays.begin(); Ri < rays.end(); Ri ++) {
        if (Ri->maxt > Ri->mint ){      
        glColor3f(0, 0, 1);	
        drawLine( Ri->o.z, Ri->o.y, Ri->o.z+len, Ri->o.y+Ri->d.y*(len/Ri->d.z));

        glLineWidth(1);
        }
        else return false;
    }
    return true;
}

void Ray_Tracer::draw_ray_chain(vector<Ray> ray_chain)
{   
    for (int k=0; k<ray_chain.size(); k++){
	
	vector<Ray>::iterator Ri, Rn;
	//If the last ray is invalid, then abort;
	Ri = ray_chain.end() - 1; 
	if (Ri->maxt > Ri->mint ){
                //draw all the rays except last one
            for (Ri= ray_chain.begin(); Ri < ray_chain.end()-1; Ri ++) {
                glColor3f(0, 0, 1);	
                Rn = Ri + 1;
                drawLine( Ri->o.z, Ri->o.y, Rn->o.z, Rn->o.y);
                //cout<<Ri->o<<" "<<Ri->d<<endl;
            }

            //draw the last ray.
            float len = lens_w.lviPlane.z; 
            if (Ri->maxt >= Ri->mint) 
            {
                glColor3f(0, 0, 1);		
                drawLine( Ri->o.z, 
                          Ri->o.y, 
                          len, 
                          Ri->o.y+((len-Ri->o.z)/Ri->d.z)*Ri->d.y );				
            }
        }
    }
}


bool Ray_Tracer::single_surface_refraction(Ray * outRay, const Ray * inRay, const float input_miu, lensElement l ){

	Point center (0,0,(l.zpos - l.rad)); //center of the lens sphere
	Vector CO = center-inRay->o;
	float C = CO.LengthSquared();
	float D = Dot(CO, inRay->d);
	float E = D * D - C + (l.rad*l.rad);
	if(E < 0.f)	{//cout<<" 0 ";
            return false;}
		float t = (l.rad > 0.f) ? (D - sqrtf(E)) : (D + sqrtf(E));
	Point Plens = inRay->o + t * inRay->d;

	if((Plens.x*Plens.x + Plens.y*Plens.y) > (l.aper*l.aper/4))
            {//cout<<Plens.x<<" "<<Plens.y<<" "<<l.aper<<" 1 ";
             return false;}
        
	Vector I, N, T;
	// Calculate the normal at the intersection
	N = (l.rad > 0.f) ? Normalize(Plens  - center)
			  : Normalize(center - Plens);
	I = inRay->d;
	// Calculate the direction of the refracted ray
	float c1, c2;
	c1 = -Dot(I, N);
	c2 = 1.f - (input_miu*input_miu)* (1.f - c1 * c1);
	if(c2 < 0.f)	{//cout<<" 2 ";
            return false;}
	c2 = sqrtf(c2);
	T = input_miu * I + (input_miu * c1 - c2) * N;
	// Update the ray origin and direction
	outRay->o = Plens;
	outRay->d = Normalize(T);	
  
	return true;
}



void Ray_Tracer::draw_rays_chains(vector<vector<Ray>> rays_chains){

  //only need to draw the 2D slice of 4D rays
   
  for (int k=0; k<rays_chains.size(); k++){
        //cout<<k<<"  "<<rays_chains[k*num+row_num].size()<<endl;
        //if (rays_chains[k].size()>8) cout<<k<<endl;
      int cnt=0;
      for(int t=0;t<rays_chains[k].size();t++)
          if(fabs(rays_chains[k][t].o.x)>0.001) cnt++;
          //cout<<rays_chains[k][1]<<endl;
      if(cnt==0) draw_ray_chain(rays_chains[k]);   
    }
  
}

// render the image by projecting the rays
bool Ray_Tracer::project_ray2image(vector<Ray> ray_chain){

    uchar4* pixel= (uchar4*)image_w.hImage;  
    ///find the valid rays last element x y coordinates.
    vector<Ray>::iterator Ri;
    
    float idx_x,idx_y;
    
    //cout<<ray_chain.size()<<" "<<lens_sys.lensSurfaces.size()<<endl;
    if (ray_chain.size()<=lens_sys.lensSurfaces.size()) //one more segment as starting 
        return false;
    
    Ri= ray_chain.end()-1; 
    //cout<<Ri->o.z<<endl;
    float len = lens_w.lviPlane.z; 
    if (Ri->maxt >= Ri->mint) 
	Ri->o= Ri->o + ((len-Ri->o.z)/Ri->d.z)*Ri->d;			
       
    float display_ratio_x =image_w.zoom*image_w.w / image_w.ray_w;
    float display_ratio_y =image_w.zoom*image_w.h / image_w.ray_h;  
    //cout<<display_ratio_x<<" "<<display_ratio_y<<endl;

    int idx_x2 =int ( (Ri->o.x-lens_w.lviPlane.x+image_w.ox)*display_ratio_x + image_w.w/2);
    int idx_y2 =int ( (Ri->o.y-lens_w.lviPlane.y+image_w.oy)*display_ratio_y + image_w.h/2);
    //cout<<image_w.w/2<<" "<<image_w.h/2<<endl;

    if (image_w.mode==false){
        if ((idx_x2<image_w.w)&& (idx_y2<image_w.h)&&(idx_x2>=0)&&(idx_y2>=0))
            image_w.image_cv.at<float>(idx_y2,idx_x2)++;
    }
    else{
        if ((idx_x2<image_w.w)&& (idx_y2<image_w.h)&&(idx_x2>=0)&&(idx_y2>=0))
            image_w.image_cv.at<float>(idx_y2,idx_x2)=ray_chain.begin()->w;    
    }

    return true;
}


void Ray_Tracer::render_image(vector<vector<Ray>> rays_chains){
    std::fill(image_w.hImage, image_w.hImage+image_w.w*image_w.h, 0);
    image_w.image_cv=cv::Mat::zeros(image_w.h,image_w.w,CV_32F);
    
    cv::Mat image = cv::Mat( image_w.h,
                             image_w.w, 
                             CV_8UC4, 
                             (uchar*) image_w.hImage);
    //spatial coordinates to pixels 
	//cout<<rays_chains.size()<<endl;
    for (int k=0; k<rays_chains.size(); k++){
        project_ray2image(rays_chains[k]);
    }
     
   //normalize and convert to uchar
    cv::Mat temp;
    cv::normalize(image_w.image_cv, temp, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    
    //mv data from image_cv to image
    for (int j=0; j<image_w.h; j++)
        for (int i=0; i<image_w.w; i++) 
            for (int k=0; k<3; k++)
                image.at<cv::Vec4b>(j,i)[k]=
                temp.at<uchar>(j,i);
                //temp_color.at<cv::Vec3b>(j,i)[2-k];           
                                               
    // cv::applyColorMap(image, image, cv::COLORMAP_JET);            
    if (interactive==1){  
        
    if (image_w.mode==1){  //PSF mode  
        image=image*128; //magnify 
        cv::GaussianBlur( image, image, cv::Size( 21, 21 ), 0, 0 );
    }
 
    if (image_w.mode==2){     //Mixture mode  
        cv::Mat image_2=image.clone();
        image_2=image_2*128; //magnify 
        cv::GaussianBlur( image_2, image_2, cv::Size( 21, 21 ), 0, 0 );
        
        for (int j=0; j<image.rows; j++){
            for (int i=0; i<image.cols; i++){
                for (int k=0; k<4; k++){
                    if(i<=j)
                        image.at<cv::Vec4b>(j,i)[k]=0;
                    
                    if(i>j) 
                        image_2.at<cv::Vec4b>(j,i)[k]=0;
                        
                    image.at<cv::Vec4b>(j,i)[k]= 
                            image.at<cv::Vec4b>(j,i)[k]+ 
                            image_2.at<cv::Vec4b>(j,i)[k];
                }
            }
        }                  
    }            
        
        
        cv::line(image,  cv::Point(image_w.w-100,image_w.h-50), 
                         cv::Point(image_w.w-50, image_w.h-50), 
                         cv::Scalar(255,255,255),1,8,0);
        //the width of the scale is 50 pixels   
        float scale = (50.f/(float)image_w.w)*1000.f/((float) image_w.ray_w*image_w.zoom);       // number to be converted to a string
        ostringstream scale_text;   // stream used for the conversion
        scale_text << int(scale)<<" um";      // insert the textual representation of 'Number' in the characters in the stream

        cv::putText(image, scale_text.str(), cv::Point(image_w.w-100, image_w.h-75), cv::FONT_HERSHEY_COMPLEX, 0.5,
                     cv::Scalar(255, 255, 255), 1, 8 );

        ostringstream dist_text;   // image plane
        Point sensor_pos=lens_w.lviPlane+Point(image_w.ox,image_w.oy,0);
        dist_text << "Sensor Position "<<sensor_pos<<" mm";   
        cv::putText(image, dist_text.str(), cv::Point(60, image_w.h-55), cv::FONT_HERSHEY_COMPLEX, 0.5,
                     cv::Scalar(255, 255, 255), 1, 8 );


        if (image_w.bg==true)
        image =  cv::Scalar::all(255) - image;  
        // invert background?
    }
                
        if(image_w.save == true && interactive==1) {
            cv::Mat img_save;
            cv::cvtColor(image, img_save, cv::COLOR_BGRA2BGR);
            cv::imwrite("image_w.png",img_save);
            image_w.save = false;
        }
        else if (interactive==0) { //save automatically for non-interactive mode
            cv::Mat psf_array=cv::Mat::zeros(7*33,11*33,CV_8U);
            for (int j=-3;j<=3;j++)
                for (int i=-5;i<=5;i++){
                cv::Point pt= cv::Point(i*400+image_w.w/2,j*400+image_w.h/2);
                cv::Rect ROI =cv::Rect(pt.x-16,  pt.y-16,33,33);
                cv::Rect ROI2=cv::Rect(33*(i+5),33*(j+3),33,33);
                temp(ROI).copyTo(psf_array(ROI2));      
                //image.at<cv::Vec4b>(pt)=cv::Vec4b(255,0,0,1);
                cv::imwrite("image_w.png",psf_array);
        }       
    }        
                                             
    cv::flip(image,image,0);          
}


inline void invalidateRay(Ray * outRay) {
	outRay->maxt = 0;
	outRay->mint = 1;
        //cout<<" TEST "<<endl;
	return;
}


bool Ray_Tracer::check_stop (Ray * outRay, const Ray * inRay, lensElement l){
	//Totally, we want to cal the O.z + t * D.z = zpos
	float stopZ = l.zpos;	

	//All we set is outRay->o , ->d; the other parts we use inRay
	(*outRay) = (*inRay);

	//if ( abs(inRay->d.z) < RAY_EPSILON) {
	//	//set the ray to be invalid		
	//	invalidateRay(outRay);				
	//	return false;
	//}
	float t = (stopZ - inRay->o.z ) / inRay->d.z;
	
	if (t >  (*inRay).maxt || t <  (*inRay).mint){
		//set the ray to be invalid		
		invalidateRay(outRay);				
		return false;
	}
	
	//If the intersection point is outside of the lense itself, then return false;
	float intersectionP = (*inRay)(t).y*(*inRay)(t).y+(*inRay)(t).x*(*inRay)(t).x;
	if ( (intersectionP) > lens_sys.stop_radius*lens_sys.stop_radius) { //abs(intersectionPy) > (input_fstop/2) || 
		invalidateRay(outRay);
		return false;
	}

	(*outRay).d = (*inRay).d;
	(*outRay).o = (*inRay)(t);

	return true; 
}

bool Ray_Tracer::forward_tracing(Ray inRay, vector<Ray>* rr) {

	vector<lensElement>::iterator Li, Lp;		
	//Refract for the Lens
	int index = 0; 
	Ray outRay; 
        rr->push_back(inRay);//save the starting point
	float miu = -1;  
	//As to the miu = ni/nt, we need to add the air to the leftmost side!!! also take care of the Stop???
	//This is from Image to Obejct. (lenses.end()-1 -> lenses.begin)
 
	for (Li = lens_sys.lenses.begin(); Li < lens_sys.lenses.end(); Li++)  {
            
            if (Li->isActive && !Li->isStop) {
			Lp = Li - 1; 
			while (Lp->isStop) Lp = Lp - 1; 
			if (Lp < lens_sys.lenses.begin() )  {
				//the first element
				miu =  1.0f/Li->ndr;  // Li->ndr/1.0
			} else {
				miu = Lp->ndr / Li->ndr;
			}
                        bool b_penetrated = single_surface_refraction(&outRay, &inRay, miu, *Li);	        
			rr->push_back(outRay);						//Now we also push_back the invalidated ray!
			index ++;			
			inRay = outRay;
                        //cout<<"["<<inRay.d<<" "<<outRay.d<<"] "<<endl;
			if (!b_penetrated){
                            invalidateRay(&outRay); 
                            return false;
                        }
		} else if (Li->isActive && Li->isStop ) {
			// Check the stop. If penetrated, then nothing happen. If not, push back an invalidated Ray.
			bool b_penetrated = check_stop(&outRay, &inRay,*Li);
			rr->push_back(outRay);	//Now we also push_back the invalidated ray!
			index ++;			
			inRay = outRay;
			if (!b_penetrated) {
				invalidateRay(&outRay); 
				return false;
			}
		}   
	}

    return true;
}

void Ray_Tracer::trace_rays(vector<Ray> rs, vector<vector<Ray>>* rays_chains){

    //fresh new
    rays_chains->clear();
    //cout<<rs.size()<<endl;
	for (int k=0; k<rs.size(); k++){
		vector<Ray> rr;
	    //cout<<rs[k].d<<endl;
		if (forward_tracing(rs[k],&rr)==true){
	        //cout<<endl<<"---"<<endl;//rr[0].d<<endl;
		rays_chains->push_back(rr);
		//cout<<k<<" ---rays number is"<<rr.size()<<endl;
                }
	}

}

static float om;
void Ray_Tracer::init(){

light_s.init();
//for ray diagram

    light_s.ray_gen_pupil(lens_sys.pupil_en.x,
                          lens_sys.pupil_en.z,
                          light_s.light_source2D_low,
                          light_s.sample_low_num);

    light_s.ray_gen_pupil(lens_sys.pupil_en.x,
                          lens_sys.pupil_en.z,        
                          light_s.light_source2D_high,
                          light_s.sample_high_num);
    om=lens_w.lvoPlane.z;

}


void Ray_Tracer::retrace_rays(vector<vector<Ray>>* rays_chains){

    for (int k=0; k<(*rays_chains).size(); k++){
        //cout<<(*rays_chains)[k][11].o.z<<endl;       
        int num=(*rays_chains)[k].size()-1;
        Ray r=(*rays_chains)[k][num];
        microlens_refraction(&r);       
        (*rays_chains)[k].push_back(r);                                   
    }
}

void Ray_Tracer::draw(){
  //cout<<"Ray tracing"<<endl;

  //check if object moved
  if (interactive==1){
    if(fabs(om-lens_w.lvoPlane.z)>0.001){
      light_s.light_source_pt[0]=lens_w.lvoPlane;  //suppose only one point source
      light_s.light_source2D_low.clear();
      light_s.light_source2D_high.clear();
      light_s.ray_gen_pupil(lens_sys.pupil_en.x,
                            lens_sys.pupil_en.z,
                            light_s.light_source2D_low,
                            light_s.sample_low_num);

      light_s.ray_gen_pupil(lens_sys.pupil_en.x,
                            lens_sys.pupil_en.z,        
                            light_s.light_source2D_high,
                            light_s.sample_high_num);   
      om=lens_w.lvoPlane.z;
    }
  }

  //ray tracing    
  trace_rays(light_s.light_source2D_low, &rays_chains_low);
  trace_rays(light_s.light_source2D_high,&rays_chains_high);  
  
  //detect computational element
  if (microlens==1){
      //cout<<"including microlens"<<endl;
      retrace_rays(&rays_chains_low);
      retrace_rays(&rays_chains_high);
  }
  
  
  //display
  draw_rays_chains(rays_chains_low);

  /*
  drawLine(-71.885,2,-31.9896,2);
  glColor3f(0.7, 0.1, 0.7);
  drawLine(-31.9896,2,-28.8243,1.9334);
  glColor3f(0.1, 0.4, 0.7);
  drawLine(-28.8243,1.9334,-28.5392,1.92218);    
  glColor3f(0.7, 0.1, 0.1);  
  drawLine(-28.5392,1.92218,-22.5948,1.55797);  
  glColor3f(0.2, 0.7, 0.2);
  drawLine(-22.5948,1.55797,-21.3098,1.47482);  
  glColor3f(0.4, 0.7, 0.5);    
  drawLine(-21.3098,1.47482,-11.2233,1.04994);    
  glColor3f(0.1, 0.7, 0.7);
  drawLine(-11.2233,1.04994,-7.91932,1.08054);    
  glColor3f(0.7, 0.7, 0.1); 
  drawLine(-7.91932,1.08054,-3.91236,1.11646);
  glColor3f(0.1, 0.7, 0.7);    
  drawLine(-3.91236,1.11646,-3.76731,1.11299);    
  glColor3f(0.2, 0.1, 0.5);  
  drawLine(-3.76731,1.11299,-0.0197585,1.07905);    
  glColor3f(0.7, 0.1, 0.1);
  drawLine(-0.0197585,1.07905,-0.0197585+0.999803*100,1.07905-0.0198551*100); 
*/
 
  render_image(rays_chains_high);

}

