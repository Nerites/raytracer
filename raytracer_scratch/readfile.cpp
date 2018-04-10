/* Read and parse the input file specified in arguments */

#include <iostream>
#include <string>
#include <fstream> // for ifstream type and is_open()
#include <istream> // for getline()
#include <sstream> // for stringstream
using namespace std;


#include <glm/glm.hpp>
#include "variables.h"
#include "readfile.h"
#include "Transform.h"

bool readvals(stringstream &s, const int numvals, float* values) {
	// Helper function for reading in values from a cmdline
	for (int i = 0; i < numvals; i++) {
		s >> values[i];
		if (s.fail()) {
			cout << "Failed reading value " << i << ", will skip\n";
			return false;
		}
	}
	return true;
}

void rightmultiply(const mat4 & M, stack<mat4> &transfstack)
{
	// Helper function for right-multiplying new transforms onto existing transform.
	// Output = Old * New;
	mat4 &T = transfstack.top();
	T = T * M;
}


void readfile(const char* filename) {
	string str, cmd;
	ifstream in;
	in.open(filename);
	if (in.is_open()) {
		getline(in, str);
		while (in) {
			if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[0] != '#')) { // No comments or blank lines
				stringstream s(str);
				s >> cmd;
				float values[10]; // Up to 10 params
				bool validinput; // set if input is valid

				if (cmd == "size") { // Set image size
					validinput = readvals(s, 2, values);
					if (validinput) {
						width = (int)values[0];
						height = (int)values[1];
					}
				}
				else if (cmd == "maxdepth") {
					validinput = readvals(s, 1, values);
					if (validinput) {
						maxdepth = values[0];
					}
				}
				else if (cmd == "output") {
					validinput = readvals(s, 1, values);
					if (validinput) {
						outname = values[0];
					}
				}
				else if (cmd == "camera") { // Initialize camera-- overwrites each time camera cmd is encountered
					validinput = readvals(s, 10, values);
					if (validinput) {
						vec3 eye = vec3(values[0], values[1], values[2]);
						vec3 center = vec3(values[3], values[4], values[5]);
						vec3 up = vec3(values[6], values[7], values[8]);
						vec3 a = eye - center;
						// eye = a, up = b
						// w = a / ||a||
						// u = (b x w) / ||b x w||
						// v = w x u
						cam.position = eye;
						cam.w = glm::normalize(a);
						cam.u = glm::normalize(glm::cross(up, cam.w)); 
						cam.v = glm::cross(cam.w, cam.u); // No normalization needed since w and u are already unit vectors
						cam.fovy = values[9] * pi / 180;
						cam.fovx = atan((tan(cam.fovy/2)*width/height))*2;
					}
				}

				// MATERIAL PROPERTIES
				else if (cmd == "ambient") {
					// Replaces ambient light color set for overall scene, if it exists.
					// ambient r g b
					validinput = readvals(s, 3, values); 
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							ambient[i] = values[i];
						}
					}
				}
				else if (cmd == "diffuse") {
					// diffuse r g b
					validinput = readvals(s, 3, values);
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							diffuse[i] = values[i];
						}
					}
				}
				else if (cmd == "specular") {
					// specular r g b
					validinput = readvals(s, 3, values);
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							specular[i] = values[i];
						}
					}
				}
				else if (cmd == "emission") {
					// emission r g b
					validinput = readvals(s, 3, values);
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							emission[i] = values[i];
						}
					}
				}
				else if (cmd == "shininess") {
					// shininess s
					validinput = readvals(s, 1, values);
					if (validinput) {
						shininess = values[0];
					}
				}

				// GEOMETRY
				else if (cmd == "maxverts") {
					// maxverts n
					validinput = readvals(s, 1, values);
					if (validinput) {
						maxverts = values[0];
					}
				}
				else if (cmd == "maxvertnorms") {
					// maxvertnorms n
					validinput = readvals(s, 1, values);
					if (validinput) {
						maxvertnorms = values[0];

					}
				}
				else if (cmd == "vertex") {
					// vertex x y z
					validinput = readvals(s, 3, values);
					if (validinput) {
						vertices.push_back(vec3(values[0], values[1], values[2]));
						numverts = numverts + 1;
					}
				} 
				else if (cmd == "vertexnormal") {
					// vertexnormal x y z nx ny nz
					validinput = readvals(s, 6, values);
					if (validinput) {
						vertnormal currvertnorm;
						currvertnorm.position = vec3(values[0], values[1], values[2]);
						currvertnorm.normal = vec3(values[3], values[4], values[5]);
						vertnorms.push_back(currvertnorm);
						numvertnorms = numvertnorms + 1;
					}
				}
				else if (cmd == "tri") {
					// tri v1 v2 v3
					validinput = readvals(s, 3, values);
					if (validinput) {
						Shape currtri;
						currtri.type = "tri";
						// Geometry
						vec3 A = vertices[values[0]];
						vec3 B = vertices[values[1]];
						vec3 C = vertices[values[2]];
						vec4 A4 = vec4(A[0], A[1], A[2], 1.0);
						vec4 B4 = vec4(B[0], B[1], B[2], 1.0);
						vec4 C4 = vec4(C[0], C[1], C[2], 1.0);
						A4 = transfstack.top() * A4;
						B4 = transfstack.top() * B4;
						C4 = transfstack.top() * C4;
						currtri.vertices[0] = vec3(A4[0], A4[1], A4[2]);
						currtri.vertices[1] = vec3(B4[0], B4[1], B4[2]);
						currtri.vertices[2] = vec3(C4[0], C4[1], C4[2]);
						vec3 partial = glm::cross((currtri.vertices[2] - currtri.vertices[0]), (currtri.vertices[1] - currtri.vertices[0])); // (C - A) x (B - A)
						vec3 norm = partial / (glm::length(partial));
						currtri.normal = norm;
						// Material properties
						for (int i = 0; i < 3; i++) { 
							currtri.ambient[i] = ambient[i];
							currtri.specular[i] = specular[i];
							currtri.diffuse[i] = diffuse[i];
							currtri.emission[i] = emission[i];
						}
						currtri.shininess = shininess;
						// Transform
						currtri.transform = mat4(1.0); 
						//transfstack.pop();
						scene.objects.push_back(currtri); 
						numobj = numobj + 1;
					}
				}
				else if (cmd == "trinormal") {
					// trinormal v1 v2 v3
					validinput = readvals(s, 3, values);
					if (validinput) {
						Shape currtri;
						currtri.type = "tri";
						// Geometry
						vec3 A = vertnorms[values[0]].position;
						vec3 B = vertnorms[values[1]].position;
						vec3 C = vertnorms[values[2]].position;
						currtri.vertices[0] = A;
						currtri.vertices[1] = B;
						currtri.vertices[2] = C;
						vec3 norm = vertnorms[values[0]].normal +
							vertnorms[values[1]].normal +
							vertnorms[values[2]].normal;
						norm = norm; // / glm::length(norm);
						currtri.normal = norm;
						// Material properties
						for (int i = 0; i < 3; i++) {
							currtri.ambient[i] = ambient[i];
							currtri.specular[i] = specular[i];
							currtri.diffuse[i] = diffuse[i];
							currtri.emission[i] = emission[i];
						}
						currtri.shininess = shininess;
						// Transform
						currtri.transform = transfstack.top(); 
						//transfstack.pop();
						scene.objects.push_back(currtri);
						numobj = numobj + 1;
					}
				}
				else if (cmd == "sphere") {
					// sphere x y z radius
					validinput = readvals(s, 4, values);
					if (validinput) {
						Shape currsph;
						currsph.type = "sph";
						// Geometry
						vec3 currpos = vec3(values[0], values[1], values[2]);
						float currrad = values[3];
						currsph.position = currpos;
						currsph.radius = currrad;
						// Material properties
						for (int i = 0; i < 3; i++) {
							currsph.ambient[i] = ambient[i];
							currsph.specular[i] = specular[i];
							currsph.diffuse[i] = diffuse[i];
							currsph.emission[i] = emission[i];
						}
						currsph.shininess = shininess;
						// Transform
						currsph.transform = transfstack.top(); 
						//transfstack.pop();
						scene.objects.push_back(currsph);
						numobj = numobj + 1;
					}
				}

				// TRANSFORMATIONS
				else if (cmd == "translate") {
					// translate x y z
					validinput = readvals(s, 3, values);
					if (validinput) {
						mat4 T = Transform::translate(values[0], values[1], values[2]);
						rightmultiply(T, transfstack);
					}
				}
				else if (cmd == "rotate") {
					// rotate x y z angle
					validinput = readvals(s, 4, values);
					if (validinput) {
						mat3 R3 = Transform::rotate(values[3], vec3(values[0], values[1], values[2]));
						// Returns matrix in column major ordering:
						// rotate3[0][0] rotate3[1][0] rotate3[2][0]
						// rotate3[0][1] rotate3[1][1] rotate3[2][1]
						// rotate3[0][2] rotate3[1][2] rotate3[2][2]

						// Convert to mat4:
						// R11 R12 R13 0
						// R21 R22 R23 0
						// R31 R32 R33 0
						// 0   0   0   1
						// (column-major ordering)
						mat4 R;
						R[0] = vec4(R3[0][0], R3[0][1], R3[0][2], 0); // Column 0
						R[1] = vec4(R3[1][0], R3[1][1], R3[1][2], 0); // Column 1
						R[2] = vec4(R3[2][0], R3[2][1], R3[2][2], 0); // Column 2
						R[3] = vec4(0, 0, 0, 1);					  // Column 3										

						rightmultiply(R, transfstack);
					}
				}
				else if (cmd == "scale") {
					// scale x y z
					validinput = readvals(s, 3, values);
					if (validinput) {
						mat4 S = Transform::scale(values[0], values[1], values[2]);
						rightmultiply(S, transfstack);
					}
				}

				else if (cmd == "directional") {
					// directional x y z r g b
					validinput = readvals(s, 6, values);
					light inputLight;
					if (validinput) {
						inputLight.lightType = "dir";
						inputLight.dirOrPos = vec3(values[0], values[1], values[2]);
						inputLight.color = vec3(values[3], values[4], values[5]);
						directional.push_back(inputLight);
						numDirLights = numDirLights + 1;
					}
				}
				else if (cmd == "point") {
					// point x y z r g b
					validinput = readvals(s, 6, values);
					light inputLight;
					if (validinput) {
						inputLight.lightType = "pt";
						inputLight.dirOrPos = vec3(values[0], values[1], values[2]);
						inputLight.color = vec3(values[3], values[4], values[5]);
						point.push_back(inputLight);
						numPointLights = numPointLights + 1;
					}
				}
				else if (cmd == "attenuation") {
					// attenuation const linear quadratic
					// default (1, 0, 0) (no attenuation)
					validinput = readvals(s, 3, values);
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							attenuation[i] = values[i];
						}
					}
				}
				else if (cmd == "ambient") {
					// ambient r g b
					validinput = readvals(s, 3, values);
					if (validinput) {
						for (int i = 0; i < 3; i++) {
							ambient[i] = values[i];
						}
					}
				}
				else if (cmd == "pushTransform") {
					// Initializes a new transform matrix on the stack.
					// Must occur before each different shape transformation.
					transfstack.push(mat4(1.0)); 
				}
				else if (cmd == "popTransform") {
					if (transfstack.size() <= 1) {
						cerr << "Stack has no elements. Cannot pop\n";
					}
					else {
						transfstack.pop();
					}
				}
				else {
					cerr << "Unknown Command: " << cmd << ". Skipping \n";
				}
			}
			getline(in, str); // get next line
		}
	} else {
	cerr << "Unable to Open Input Data File " << filename << "\n";

	throw 2;
	}
}