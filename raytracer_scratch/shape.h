#include <glm/glm.hpp>
#include <string>

typedef glm::vec3 vec3;
typedef glm::mat4 mat4;

struct Shape {
	std::string type; // either "sph" or "tri"
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float emission[3];
	float shininess;
	mat4 transform;
	// Sphere only
	vec3 position;
	float radius;
	vec3 vertices[3];
	vec3 vertnorms[3];
	vec3 normal;
};
/*	Sphere toSphere() {
		Sphere sph;
		for (int i = 0; i < 3; i++) {
			sph.ambient[i] = this->ambient[i];
			sph.diffuse[i] = this.diffuse[i];
			sph.specular[i] = specular[i];
			sph.emission[i] = emission[i];
		}
		sph.shininess = shininess;
		sph.position = vec3(0, 0, 0); // default value
		sph.radius = 1.0; // default value
		return sph;
	}
	operator Sphere() {
		Sphere sph;
		for (int i = 0; i < 3; i++) {
			sph.ambient[i] = ambient[i];
			sph.diffuse[i] = diffuse[i];
			sph.specular[i] = specular[i];
			sph.emission[i] = emission[i];
		}
		sph.shininess = shininess;
		return sph;

	}
};

struct Sphere : Shape {
	vec3 position;
	float radius;
};

struct Triangle : Shape {
};*/