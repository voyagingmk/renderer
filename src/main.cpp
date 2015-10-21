#include <stdio.h>
#include <thread>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iostream>
//#define CATCH_CONFIG_MAIN
//#include "catch.hpp"
#include "base.hpp"
#include "vector.hpp"
#include "renderer.hpp"
#include "sphere.hpp"
#include "perspectivecamera.hpp"
#include "material.hpp"
#include "color.hpp"
#include "plane.hpp"
#include "union.hpp"

#include "json.hpp"

// for convenience
using json = nlohmann::json;

/*
TEST_CASE( "Vector base usages", "[Vector]" ) {
	Vector v1(0,0,0);
	Vector v2(v1);
	REQUIRE( v1 == v2 );
    Vector v3(-1,-1,-1);
	Vector v4(1,1,1);
	REQUIRE( v3 + v4 == v2 );
	REQUIRE( v3 + v4 + v3 + v4 == v2 );
	Vector v10 = (v1 + v4);
	REQUIRE( v3 == -v4 );
	REQUIRE( -v3 * 5 == v4 * 5 );
	REQUIRE( v1.cross(v3+v4) == v2.cross(v3+v4) );
};
*/

void renderArea(Renderer &renderer, cil::CImg<unsigned char> &img, PtrGeometry pUnion, PerspectiveCamera& camera, int maxReflect, int x, int y, int w, int h)
{
	renderer.rayTraceReflection(img, std::static_pointer_cast<Geometry>(pUnion), camera, maxReflect, x,y,w,h);
}

int main(int argc, char ** argv){
	std::ifstream f("config.json");
    std::string config_string( (std::istreambuf_iterator<char>(f) ),
                             (std::istreambuf_iterator<char>()) );
    f.close();
    json config = json::parse(config_string);

	std::cout<<config["camera"]["eye"]<<std::endl;
	
	int width = config["width"], height = config["height"], multithread = config["multithread"];
	printf ("width: %d, height: %d, multithread: %d \n", width, height, multithread);
	cil::CImg<unsigned char> img(width,height,1,3);
	Renderer renderer;
	/*
	PtrSphere sphere = std::make_shared<Sphere>(std::make_shared<Vector>(0,10,-10),10);

	PerspectiveCamera camera(std::make_shared<Vector>(0, 10, 10),
                          std::make_shared<Vector>(0, 0, -1),
                          std::make_shared<Vector>(0, 1, 0),
                          90);
	renderer.renderNormal(img, *sphere, camera, 20);
	*/
	PtrSphere sphere1 = std::make_shared<Sphere>(std::make_shared<Vector>(-10, 10, -10),10);
	sphere1->setMaterial(std::dynamic_pointer_cast<Material>(std::make_shared<PhongMaterial>(Color::Red, Color::White, 16, 0.25)));	

	PtrSphere sphere2 = std::make_shared<Sphere>(std::make_shared<Vector>(10, 10, -10),10);
	sphere2->setMaterial(std::dynamic_pointer_cast<Material>(std::make_shared<PhongMaterial>(Color::Blue, Color::White, 16,0.25)));

	PtrPlane plane = std::make_shared<Plane>(std::make_shared<Vector>(0,1,0),0);
	plane->setMaterial(std::dynamic_pointer_cast <Material>(std::make_shared<CheckerMaterial>(0.1, 0.5)));


    PtrUnion pUnion = std::make_shared<Union>(std::vector<PtrGeometry>({sphere1, sphere2, plane}));

	PerspectiveCamera camera(std::make_shared<Vector>(config["camera"]["eye"]),
                          std::make_shared<Vector>(config["camera"]["front"]),
                          std::make_shared<Vector>(config["camera"]["up"]),
                          config["camera"]["fov"]);
	auto time0 = std::chrono::system_clock::now();
	if(!multithread){
		renderer.rayTrace(img, *pUnion, camera);
		renderer.rayTraceReflection(img, std::static_pointer_cast<Geometry>(pUnion), camera, 4);
	}else{
		std::thread t1(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Geometry>(pUnion), std::ref(camera), 4, 0, 0, width, height/2); 
		std::thread t2(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Geometry>(pUnion), std::ref(camera), 4, 0, height/2, width, height/2); 
		t1.join();
	    t2.join();	
	}
	auto time1 = std::chrono::system_clock::now();
	auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
    printf("cost: %lldms\n",time_cost);
	img.display("");
	return 0;
}
