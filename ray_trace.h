#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_
#include <iostream>     // std::cout
#include <algorithm>    // std::fill
#include <vector>       // std::vector
#include <float.h>
#include "geometry.h"
#include "file_parse.h"
#include "lens_model.h"
#include "image_view.h"
#include "display_opengl.h"
#include "microlens_model.h"

class Light_source {
public:
	void init();
        void ray_gen_pupil(float radius, float z_axis,
                           vector<Ray>& light_source,
                           int num);
	int sample_low_num;
	int sample_high_num;
	vector<Point> light_source_pt;  //light source point
 	vector<Ray> light_source2D_low; //rays for lens view(1D) and ray diagram view
 	vector<Ray> light_source2D_high;//rays for image rendering
};

class Ray_Tracer{

public:
    vector<vector<Ray>> rays_chains_low;
    vector<vector<Ray>> rays_chains_high;
    void init();
    void draw();
    bool forward_tracing(Ray inRay, vector<Ray>* rr);
    Light_source light_s;
private:

    bool check_stop (Ray * outRay, const Ray * inRay, lensElement l);
    bool single_surface_refraction(Ray * outRay, const Ray * inRay, 
                                   const float input_miu,
                                   lensElement l);
    void trace_rays(vector<Ray> rs, vector<vector<Ray>>* rays_chains);
    void retrace_rays(vector<vector<Ray>>* rays_chains);
    bool draw_rays(vector<Ray> rays); //bundle of rays
    void draw_ray_chain(vector<Ray> ray_chain);//a chain of rays
    void draw_rays_chains(vector<vector<Ray>> rays_chains);//a bundle chain of rays
    void render_image(vector<vector<Ray>> rays_chains);// project to image plane
    bool project_ray2image(vector<Ray> ray_chain);
};


#endif

