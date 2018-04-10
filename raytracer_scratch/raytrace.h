#include <glm/glm.hpp>
#include <FreeImage.h>
#include "variables.h"


// Defines
const int maxobjects = 30;

// Structs
struct rgbpix {
	int red;
	int blue;
	int green;
};


struct intersect {
	float distance;
	vec3 location;
	bool intersects;
	vec3 normal;
	Shape obj;
};


struct ray {
	vec3 position;
	vec3 direction;
};


// functions  in raytrace
FIBITMAP* raytrace(int width, int height);
ray rayThruPixel(int height, int width); // i iterates through height, j through width
intersect rayThruObjects(ray ray); // checks intersection of ray with all objects in a scene
intersect intersectObj(ray ray, Shape obj); // Returns intersection of ray with object (either triangle or sphere)
intersect intersectSphere(ray ray, Shape sph); // Returns intersection of ray with sphere 
intersect intersectTri(ray ray, Shape tri); // Returns intersection of ray with triangle
intersect intersectTrinorm(ray ray, Shape tri); // Returns intersection of ray with triangle
vec3 findColor(intersect hit, int currDepth, ray prevRay); // Returns color of pixel at intersect point
BYTE* percentToRGB(vec3 color);
bool lightIsVisible(vec3 intersectPos, vec3 lightDir, string lightType, vec3 lightPos);
