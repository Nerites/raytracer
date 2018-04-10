
#include <iostream>
using namespace std;

#define MAINPROGRAM

#include "raytrace.h"
#include "readfile.h"

void init() { // Sets default values
	transfstack.push(mat4(1.0)); // initialize with an identity matrix
	numobj = 0; // initialize object count
	attenuation[0] = 1.0; // Const
	attenuation[1] = 0.0; // Linear
	attenuation[2] = 0.0; // Quadratic
	ambient[0] = 0.2; // red
	ambient[1] = 0.2; // green
	ambient[2] = 0.2; // blue
	maxdepth = 5; // number of ray bounces
	outname = "output.png";

	// Initialize all other material properties to 0
	emission[0] = 0.0;
	emission[1] = 0.0;
	emission[2] = 0.0;
	specular[0] = 0.0;
	specular[1] = 0.0;
	specular[2] = 0.0;
	diffuse[0] = 0.0;
	diffuse[1] = 0.0;
	diffuse[2] = 0.0;
	shininess = 0.0;
}

int main(int argc, char* argv[]) {
	init();
	if (argc < 2) {
		exit(-1);
	}
	cout << "Reading " << argv[1] << "...";
	readfile(argv[1]);
	FreeImage_Initialise();
	FIBITMAP *img = raytrace(width, height);
	FreeImage_Save(FIF_PNG, img, outname.c_str(), 0);
	FreeImage_DeInitialise();
	return 0;
}