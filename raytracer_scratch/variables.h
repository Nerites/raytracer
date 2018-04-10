// File storing global variables
#include <stack>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "shape.h"

#ifdef MAINPROGRAM
#define EXTERN
#else
#define EXTERN extern
#endif


typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;


// Custom structs
struct scene_t {
	vec3 bgcolor;
	std::vector<Shape> objects;
};

struct vertnormal {
	vec3 position;
	vec3 normal;
};

struct light {
	vec3 color;
	vec3 dirOrPos; // Either direction of directional light or location of point light
	string lightType; // "dir" or "pt"
};

struct camera {
	vec3 position;
	vec3 u; // Coordinate frame
	vec3 v;
	vec3 w;
	float fovy; // given in radians; translated assuming input is given in degrees
	float fovx; // fovx = fovy * width / height
};


EXTERN int width;
EXTERN int height;
EXTERN int maxdepth;
EXTERN string outname;
EXTERN int maxverts;
EXTERN int numverts;
EXTERN int maxvertnorms;
EXTERN int numvertnorms;
EXTERN int numobj;
EXTERN int numPointLights;
EXTERN int numDirLights;
EXTERN scene_t scene;
EXTERN camera cam;

EXTERN float ambient[3];
EXTERN float diffuse[3];
EXTERN float specular[3];
EXTERN float emission[3];
EXTERN float shininess;
EXTERN float attenuation[3];
EXTERN std::vector<light> directional;
EXTERN std::vector<light> point;
EXTERN stack <mat4> transfstack; 

//EXTERN stack <vec3> vertices;
//EXTERN stack <vertnormal> vertnorms;

EXTERN std::vector<vec3> vertices;
EXTERN std::vector<vertnormal> vertnorms;
//EXTERN stack <Shape> scene;