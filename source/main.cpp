#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
#include <vector>
#include <cfloat> // FLT_MAX
#include <iostream>
#include <limits>
#include <cstddef>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
using namespace std;

#include "object3d.h"
#include "scene_parser.h"

#include "image.h"
#include "vectors.h"
#include "sphere.h"
#include "camera.h"
#include "ray.h"
#include "hit.h"
#include "group.h"
#include "matrix.h"
#include "light.h"
#include "material.h"
#include "normal.h"
#include "shader.h"
#include "diffuse.h"
#include "raytracer.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

float clamp(float a)
{
    if(a > 1.0f)
        return 1.0f;
    else if(a < 0.0f)
        return 0.0f;
    else
        return a;
}

float specularExponent(Ray r, Hit h, Vec3f dir)
{
    // (h.n)^e -- h=halfway vector (direction+view)/|(direction+view)|, n = normal
    Vec3f view = r.getDirection(); view.Negate(); 
    Vec3f half = (dir+view); half.Normalize(); // Halfway vector
    float t = half.Dot3(h.getNormal()); t = max(0.0f,t);
    return t;
}
int main(int argc, char** argv) {

char *input_file = NULL;
    int width = 100;
    int height = 100;
    char *output_file = NULL;
    float depth_min = 0;
    float depth_max = 1;
    char *depth_file = NULL;
    char *normal_file = NULL;
    bool shade_back = false;
    bool shadows = true;
    float weight = 1;
    int bounces = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-input")) {
            i++; assert (i < argc); 
            input_file = argv[i];
        }
        else if (!strcmp(argv[i],"-size")) {
            i++; assert (i < argc); 
            width = atoi(argv[i]);
            i++; assert (i < argc); 
            height = atoi(argv[i]);
        }
        else if (!strcmp(argv[i],"-output")) {
            i++; assert (i < argc); 
            output_file = argv[i];
        }
        else if (!strcmp(argv[i],"-depth")) {
            i++; assert (i < argc); 
            depth_min = atof(argv[i]);
            i++; assert (i < argc); 
            depth_max = atof(argv[i]);
            i++; assert (i < argc); 
            depth_file = argv[i];
        }
        else if(!strcmp(argv[i],"-normals")){
            i++; assert(i<argc);
            normal_file = argv[i];
        }
        else if(!strcmp(argv[i],"-shade_back")){
            shade_back = true;
        }
        else if(!strcmp(argv[i],"-bounces")){
            i++; assert(i<argc);
            bounces = atoi(argv[i]);
        }
        else if(!strcmp(argv[i],"-weight")){
            i++; assert(i<argc);
            weight = atof(argv[i]);
        }
        else if(!strcmp(argv[i],"-noshadow")){
            shadows=false;
        }
        else {
            printf ("whoops error with command line argument %d: '%s'\n",i,argv[i]);
            assert(0);
        }
    } 


   // Read input file
    SceneParser *s = new SceneParser(input_file);
    Camera *c = s->getCamera();
    Group *g = s->getGroup();

 // Define Image Files
    Image *output_image = new Image(width, height);
    Image *normal_image = new Image(width, height);
    Image *depth_image = new Image(width, height);
 
Vec3f background = s->getBackgroundColor();
output_image->SetAllPixels(background);
normal_image->SetAllPixels(background);
depth_image->SetAllPixels(background);


 // Set output files
    bool out=false, normout=false, depthout=false;
    if(output_file != NULL)
        out = true;
    if(normal_file != NULL)
        normout = true;
   if(depth_file != NULL)
        depthout = true;

   // Main loop
    for(int i=0;i<width;i++) {
        for(int j=0;j<height;j++)
        {
            float xi = (float)i / width; 
            float yj = (float)j / height;
            Vec2f p(xi,yj); //0,0 to 1,1

            Ray r = c->generateRay(p);
            Hit h(FLT_MAX, NULL, Vec3f(0,0,0)); // The object hit
            float tmin = c->getTMin();

            RayTracer t(s, bounces, shadows, shade_back);
            
            /*tasks.push_back([&img,t,i,j,p,width,height]()
            {*/
                //Vec3f cc = t.castRay(r,h,tmin,0,1);  // comment out for anti-aliasing
                Vec3f cc = t.antiAlias(p, float(width), float(height)); // comment out for no AA
                output_image->SetPixel(i,j,cc);
            /*});*/

            //if(debugg)
            {
                Hit hh(FLT_MAX, NULL, Vec3f(0,0,0)); // The object hit
                g->intersect(r,hh,tmin);
                if(hh.getMaterial() != NULL)
                {
                    Vec3f n = hh.getNormal();
                    if(depthout) {
                        float dc = clamp((depth_max-hh.getT()) / (depth_max-depth_min)); // depth colour
                        depth_image->SetPixel(i,j,Vec3f(dc,dc,dc));
                    }
                    if(normout) {
                        n.Set(abs(n.x()), abs(n.y()), abs(n.z())); //positive or negative
                        normal_image->SetPixel(i,j,n);
                    }
                }
            }
        }
    }
	 //save tga   
			if(normal_file != NULL)
			{
			normal_image->SaveTGA(normal_file);
     	 	cout<<normal_file<<endl;
			}
 			if(output_file != NULL)   	
			{
	 		output_image->SaveTGA(output_file);
   			cout<<output_file<<endl;	
		    }
         	if(depth_file != NULL)
         	{
         	depth_image->SaveTGA(output_file);
   			cout<<depth_image<<endl;
		 	}
}

