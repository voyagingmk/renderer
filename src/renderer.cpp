#include "renderer.hpp"
#include "perspectivecamera.hpp"
#include "shape.hpp"
#include "intersect_result.hpp"
#include "vector.hpp"
#include "material.hpp"
#include "color.hpp"
#include "ray.hpp"

void Renderer::renderDepth(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera, float maxDepth){
    scene.init();
    camera.init();
    int w = img.width(), h = img.height();
    for (int y = 0; y < h; y++) {
		float sy = 1.0f - (float)y / h;
        for (int x = 0; x < w; x++) {
			float sx = (float)x / w;
            PtrRay ray = camera.generateRay(sx, sy);
            PtrIntersectResult result = scene.intersect(ray);
            if (result->getGeometry()) {
                int depth = 255.0f - min((result->getDistance() / maxDepth) * 255.0f, 255.0f);
                img.atXYZC(x, y, 0, 0) = depth;
                img.atXYZC(x, y, 0, 1) = depth;
                img.atXYZC(x, y, 0, 2) = depth;
                //atXYZC(x, y, 0, 0) = 255;
            }
        }
    }
}
void Renderer::renderNormal(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera, float maxDepth){
    scene.init();
    camera.init();
    int w = img.width(), h = img.height();
    for (int y = 0; y < h; y++) {
        float sy = 1.0f - (float)y / h;
        for (int x = 0; x < w; x++) {
            float sx = (float)x / w;
            PtrRay ray = camera.generateRay(sx, sy);
            PtrIntersectResult result = scene.intersect(ray);
            if (result->getGeometry()) {
                PtrVector pNormal = result->getNormal();
                img.atXYZC(x, y, 0, 0) = (pNormal->x() + 1) * 128;
                img.atXYZC(x, y, 0, 1) = (pNormal->y() + 1) * 128;
                img.atXYZC(x, y, 0, 2) = (pNormal->z() + 1) * 128;
                //atXYZC(x, y, 0, 0) = 255;
            }
        }
    }
};


void Renderer::rayTrace(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera){
    scene.init();
    camera.init();
    int w = img.width(), h = img.height();
    for (int y = 0; y < h; y++) {
        float sy = 1.0f - (float)y / h;
        for (int x = 0; x < w; x++) {
            float sx = (float)x / w;
            PtrRay ray = camera.generateRay(sx, sy);
            PtrIntersectResult result = scene.intersect(ray);
            if (result->getGeometry()) {
                PtrMaterial pMaterial = result->getGeometry()->getMaterial();
                PtrColor color = pMaterial->sample(ray, result->getPosition(), result->getNormal());
                //printf("c=%f,%f,%f\n", color->r(),color->g(),color->b());
                img.atXYZC(x, y, 0, 0) = min( int(color->r() * 255), 255);
                img.atXYZC(x, y, 0, 1) = min( int(color->g() * 255), 255);
                img.atXYZC(x, y, 0, 2) = min( int(color->b() * 255), 255);
            }
        }
    }
}

PtrColor Renderer::rayTraceRecursive( PtrGeometry scene, PtrRay ray, int maxReflect) {
    PtrIntersectResult result = scene->intersect(ray);
    if (result->getGeometry()) {
    	PtrMaterial pMaterial = result->getGeometry()->getMaterial();
        float reflectiveness = pMaterial->getReflectiveness();
        PtrColor color = pMaterial->sample(ray, result->getPosition(), result->getNormal());
        //printf("c=%f,%f,%f\n", color->r(),color->g(),color->b());
		color = std::make_shared<Color>(*color * (1.0f - reflectiveness));
        
        if (reflectiveness > 0 && maxReflect > 0) {
            PtrVector r =std::make_shared<Vector>( *(result->getNormal()) * ( -2 * (result->getNormal()->dot(*(ray->getDirection()))) ) + *(ray->getDirection()) );
            PtrRay new_ray = std::make_shared<Ray>(result->getPosition(), r);
            PtrColor reflectedColor = rayTraceRecursive(scene, new_ray, maxReflect - 1);
            assert(reflectedColor->r() >= 0 && reflectedColor->g() >= 0 && reflectedColor->b() >= 0);
            *color = *color + *reflectedColor * reflectiveness;
        }
        return color;
    }
    else
        return Color::Black;
}
 
void Renderer::rayTraceReflection(cil::CImg<unsigned char> &img, PtrGeometry scene, PerspectiveCamera& camera, int maxReflect, int px, int py, int pw, int ph) {
    scene->init();
    camera.init();
    int w = pw, h = ph, img_width = img.width(), img_height = img.height();
    if(w == 0)
		w = img_width;
	if(h == 0)
		h = img_height;
    for (int y = py, yMax = py + h; y < yMax; y++) {
        float sy = 1.0 - (float)y / img_height;
        for (int x = px, xMax = px + w; x < xMax; x++) {
            float sx = (float)x / img_width;
            //printf("sx,sy=%f,%f\n",sx,sy);
            PtrRay ray = camera.generateRay(sx, sy);
            PtrColor color = rayTraceRecursive(scene, ray, maxReflect);
            img.atXYZC(x, y, 0, 0) = min( int(color->r() * 255), 255);
            img.atXYZC(x, y, 0, 1) = min( int(color->g() * 255), 255);
            img.atXYZC(x, y, 0, 2) = min( int(color->b() * 255), 255);
        }
    }
}
