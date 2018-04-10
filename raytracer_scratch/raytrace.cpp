#include <FreeImage.h>
#include <math.h>
#include <string>
#include <typeinfo>
#include <iostream>
#include <algorithm>
using namespace std;

#include "raytrace.h" // already includes variables.h


FIBITMAP* raytrace(int width, int height) { // use our single universal camera
	int pixct = width * height;
	BYTE *pixels = new BYTE[3 * pixct]; // 3 color channels per pixel - RGB
	//std::vector <rgbpix> pixels;
	for (int i = 0; i < height; i++) {
		cout << "Casting ray through pixels in row " << i << " out of " << height << "\n";
		for (int j = 0; j < width; j++) {
			ray pixray = rayThruPixel(i, j);
			intersect hit = rayThruObjects(pixray);
			vec3 color = findColor(hit, maxdepth, pixray);
			BYTE *pixel = new BYTE[3];
			pixel = percentToRGB(color);
			pixels[3 * (width*height - i*width + j)] = pixel[2]; // BLUE FIXME testing *findColor(hit); 
			pixels[3 * (width*height - i*width + j) + 1] = pixel[1]; // GREEN FIXME testing *findColor(hit); 
			pixels[3 * (width*height - i*width + j) + 2] = pixel[0]; // RED FIXME testing *findColor(hit); 
		}
	}
	FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, width, height, width * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
	delete pixels;
	return img;
}



ray rayThruPixel(int h, int w) {
	// Returns a ray from camera to pixel h, w (i, j)
	// ray = eye + (alpha*u + beta*v - w) /
	//			   |alpha*u + beta*v - w|
	// alpha = tan(fovx/2) * ((j - (width/2)) /
	//						 (width/2)
	// beta = tan(fovy/2) * ((height/2) - i) /
	//						(height/2)
	float midh = h - 0.5;
	float midw = w + 0.5;

	float alpha, beta;

	float fovx = cam.fovx; // * 3.14159 / 180;
	float fovy = cam.fovy; // * 3.14159 / 180;

	alpha = tan(fovx / 2) * (midw - (width / 2)) / (width / 2);
	beta = tan(fovy / 2) * ((height / 2) - midh) / (height / 2);

	vec3 partial = alpha*cam.u + beta*cam.v - cam.w;
	ray outray;
	outray.position = cam.position; //vec4(cam.position[0], cam.position[1], cam.position[2], 0.0);
	vec3 partialDir = glm::normalize(partial);
	outray.direction = partialDir; //vec4(partialDir[0], partialDir[1], partialDir[2], 0.0);
	return outray;
}

intersect rayThruObjects(ray ray) {
	// Loops through all objects in scene to test for intersection
	float mindist = INFINITY;
	Shape currobj;
	intersect hit;
	intersect finalint;
	finalint.distance = INFINITY;
	finalint.intersects = false;
	for (int i = 0; i < numobj; i++) {
		currobj = scene.objects[i];
		hit = intersectObj(ray, currobj);
		if (hit.intersects) {
			if ((hit.distance > 0) && (hit.distance < mindist)) {
				finalint = hit;
				mindist = hit.distance;
			}
		}
	};
	return finalint;
}

intersect intersectObj(ray inRay, Shape obj) {
	intersect hit;
	hit.intersects = false;
	
	if (obj.type == "sph") {
		ray transfRay;
		vec4 transfPos = glm::inverse(obj.transform) * vec4(inRay.position[0], inRay.position[1], inRay.position[2], 1.0);
		vec4 transfDir = glm::inverse(obj.transform) * vec4(inRay.direction[0], inRay.direction[1], inRay.direction[2], 0.0);
		transfRay.direction = vec3(transfDir[0], transfDir[1], transfDir[2]);
		transfRay.position = vec3(transfPos[0], transfPos[1], transfPos[2]);
		hit = intersectSphere(transfRay, obj);
	}
	else if (obj.type == "tri") {
		hit = intersectTri(inRay, obj);
	}
	else if (obj.type == "trinormal") {
		hit = intersectTrinorm(inRay, obj);
	}
	else {
		hit.distance = INFINITY;
		hit.intersects = false;
		cout << "Unrecognized object type: " + obj.type + "\n";
	}
	if (hit.intersects && (obj.type == "sph")) { // transform sphere back
		vec4 transfPos = obj.transform * vec4(hit.location[0], hit.location[1], hit.location[2], 1.0);
		hit.location = vec3(transfPos[0], transfPos[1], transfPos[2]);
		vec4 transfNorm = glm::transpose(glm::inverse(obj.transform)) * vec4(hit.normal[0], hit.normal[1], hit.normal[2], 0.0);
		hit.normal = glm::normalize(vec3(transfNorm[0], transfNorm[1], transfNorm[2]));
	}
	return hit;
}

intersect intersectSphere(ray ray, Shape sph) {
	// Solves for the intersections of the ray with the sphere (if they exist)	

	vec3 P0 = ray.position;
	vec3 C = sph.position;
	vec3 P1 = ray.direction;
	vec3 CP0 = P0 - C;

	// Solve the following quadratic equation for t:
	// (P1 dot P1)*t^2 + (P0 - C)2t + ((P0-C) dot (P0 - C)) - r^2 = 0
	float a = glm::dot(P1, P1);
	float b = 2 * glm::dot(P1, CP0);
	float c = glm::dot(CP0, CP0) - pow(sph.radius, 2);
	float determinant = pow(b, 2) - 4 * a * c;
	intersect retint;
	retint.intersects = false; // initialize to false

	if (determinant >= 0) { // Treating tangets as hits
		float root1 = (-b + sqrt(determinant)) / (2 * a); // Calculate first root
		if (determinant == 0) { // Both roots are the same
			vec3 P = P0 + P1*root1;
			retint.distance = root1;
			retint.normal = glm::normalize(P - C);
			retint.intersects = true;
			retint.obj = sph;
			retint.location = P0 + P1*retint.distance;
		}
		else {
			float root2 = (-b - sqrt(determinant)) / (2 * a); // Calculate second root
			if (root1 < root2) { 
				if (root1 > 0) { // root1 closer and positive
					vec3 P = P0 + P1*root1;
					retint.normal = glm::normalize(P - C);
					retint.distance = root1;
					retint.intersects = true;
					retint.obj = sph;
					retint.location = P0 + P1*retint.distance;
				}
				else if (root2 < 0) { // Both roots are negative- so sphere is behind ray??
					retint.distance = INFINITY;
					retint.intersects = false;
				} else { // root1 is negative and root2 is positive, so choose root2
					vec3 P = P0 + P1*root2;
					retint.normal = glm::normalize(P - C);
					retint.distance = root2;
					retint.intersects = true;
					retint.obj = sph;
					retint.location = P0 + P1*retint.distance;
				}
			}
			else {
				if (root2 > 0) { // root2 is smaller, but still positive
					vec3 P = P0 + P1*root2;
					retint.normal = glm::normalize(P - C);
					retint.distance = root2;
					retint.intersects = true;
					retint.obj = sph;
					retint.location = P0 + P1*retint.distance;
				}
				else if (root1 < 0) { // Both roots are negative
					retint.distance = INFINITY;
					retint.intersects = false;
				} else { // root2 is negative, so choose root1
					vec3 P = P0 + P1*root1;
					retint.normal = glm::normalize(P - C);
					retint.distance = root1;
					retint.intersects = true;
					retint.obj = sph;
					retint.location = P0 + P1*retint.distance;
				}
			}
		}
	}
	else {
		retint.distance = INFINITY;
		retint.intersects = false;
	}
	return retint;
}

intersect intersectTri(ray ray, Shape tri) {
	vec3 P0 = ray.position;
	vec3 P1 = ray.direction;
	vec3 A = tri.vertices[0];
	vec3 B = tri.vertices[1];
	vec3 C = tri.vertices[2];
	intersect retint;
	vec3 partial = tri.normal; 
	vec3 n = partial / glm::length(partial);
	retint.normal = -n; //tri.normal;

	// Determine if ray intersects with plane
	if (glm::dot(P1, n) == 0) { // plane is parallel to ray direction
		retint.distance = INFINITY;
		retint.intersects = false;
	}
	else { // Plane intersect -> Check for triangle intersection
		float planedist = (glm::dot(A, n) - glm::dot(P0, n)) / (glm::dot(P1, n));
		vec3 P = P0 + P1*planedist;

		vec3 planeA_norm = glm::cross(n, C - B) / (glm::dot(glm::cross(n, C - B), A - C));
		float planeA_w = -glm::dot(planeA_norm, C);
		float alpha = glm::dot(planeA_norm, P) + planeA_w;

		vec3 planeB_norm = glm::cross(n, A - C) / (glm::dot(glm::cross(n, A - C), B - A));
		float planeB_w = -glm::dot(planeB_norm, A);
		float beta = glm::dot(planeB_norm, P) + planeB_w;

		vec3 planeC_norm = glm::cross(n, B - A) / (glm::dot(glm::cross(n, B - A), C - B));
		float planeC_w = -glm::dot(planeC_norm, B);
		float gamma = glm::dot(planeC_norm, P) + planeC_w;

		if ((alpha >= 0) && (beta >= 0) && (gamma >= 0)) {
			retint.distance = planedist;
			retint.location = P;
			retint.intersects = true;
			retint.obj = tri;
		}
		else {
			retint.intersects = false;
			retint.distance = INFINITY;
		}
	}
	return retint;
}

intersect intersectTrinorm(ray ray, Shape tri) {
	vec3 P0 = ray.position;
	vec3 P1 = ray.direction;
	vec3 A = tri.vertices[0];
	vec3 B = tri.vertices[1];
	vec3 C = tri.vertices[2];
	vec3 partial = tri.normal;
	vec3 n = partial / glm::length(partial);
	intersect retint;

	// Determine if ray intersects with plane
	if (glm::dot(P1, n) == 0) { // plane is parallel to ray direction
		retint.distance = INFINITY;
		retint.intersects = false;
	}
	else { // Plane intersect
		float planedist = (glm::dot(A, n) - glm::dot(P0, n)) / (glm::dot(P1, n));
		vec3 P = P0 + P1*planedist;

		// If all 3 cross products point in the same direction as triangle normal (dot product positive),
		// point is inside the triangle. 
		if ((glm::dot(glm::cross(P - A, B - A), n) >= 0)
			&& (glm::dot(glm::cross(P - B, C - B), n) >= 0)
			&& (glm::dot(glm::cross(P - C, A - C), n) >= 0)) {
			retint.distance = planedist;
			retint.location = P;
			retint.intersects = true;
			retint.obj = tri;
		}

		// Couldn't quite get the barycentric method working
		/*vec3 planeA_norm = glm::cross(n, C - B) / (glm::dot(glm::cross(n, C - B), A - C));
		float planeA_w = -glm::dot(planeA_norm, C);
		float alpha = glm::dot(planeA_norm, P) + planeA_w;

		vec3 planeB_norm = glm::cross(n, A - C) / (glm::dot(glm::cross(n, A - C), B - A));
		float planeB_w = -glm::dot(planeB_norm, A);
		float beta = glm::dot(planeB_norm, P) + planeB_w;

		vec3 planeC_norm = glm::cross(n, B - A) / (glm::dot(glm::cross(n, B - A), C - B));
		float planeC_w = -glm::dot(planeC_norm, B);
		float gamma = glm::dot(planeC_norm, P) + planeC_w;

		if ((alpha >= 0) && (beta >= 0) && (gamma >= 0)) {
		retint.distance = planedist;
		retint.location = P;
		retint.intersects = true;
		retint.obj = tri;
		}*/
		else {
			retint.intersects = false;
			retint.distance = INFINITY;
		}
	}
	return retint;
}

bool lightIsVisible(vec3 intersectPos, vec3 lightDirection, string lightType, vec3 lightLoc) {
	ray shadowRay;
	vec3 adjustedPos;
	// Add some epsilon displacement towards light to prevent self-shading
	adjustedPos[0] = intersectPos[0] + (0.001 * lightDirection[0]);
	adjustedPos[1] = intersectPos[1] + (0.001 * lightDirection[1]);
	adjustedPos[2] = intersectPos[2] + (0.001 * lightDirection[2]);
	shadowRay.position = adjustedPos;
	shadowRay.direction = lightDirection;
	intersect hit;
	hit.intersects = false;
/*	for (int i = 0; i < numobj; i++) {
		Shape currobj = scene.objects[i];
		hit = intersectObj(shadowRay, currobj);
		if (hit.intersects) {
			return false;
		}
	}
	return true;*/
	hit = rayThruObjects(shadowRay);
	if (hit.intersects == true) {
		if (lightType == "pt") {
			if (hit.distance > (glm::length(lightLoc - intersectPos))) {
				return true; // closest intersection is behind the light 
			}
			else {
				return false; // intersection is in front of light
			}
		}
		else { // any intersect will block directional light
			return false;
		}
	}
	else { return true; } // no intersection detected
}

BYTE * percentToRGB(vec3 color) {
	BYTE *returnpix = new BYTE[3];
	float red = color[0];
	float green = color[1];
	float blue = color[2];
	if (red > 1.0) {
		red = 1.0;
	}
	else if (red < 0.0) {
		red = 0.0;
	}
	if (green > 1.0) {
		green = 1.0;
	}
	else if (green < 0.0) {
		green = 0.0;
	}
	if (blue > 1.0) {
		blue = 1.0;
	}
	else if (blue < 0.0) {
		blue = 0.0;
	}
	returnpix[0] = red * 255;
	returnpix[1] = green * 255;
	returnpix[2] = blue * 255;
	return returnpix;
}

vec3 findColor(intersect hit, int currDepth, ray prevRay) { // returns 3 bytes
	BYTE *returnpix = new BYTE[3];
	float red, green, blue; 
	std::vector<bool> dirVisible; // visibility of directional light
	std::vector<bool> ptVisible; // visibility of point light
	if (hit.intersects) {
		// Initialize color channels with ambient and emissive light
		red = hit.obj.ambient[0] + hit.obj.emission[0];
		green = hit.obj.ambient[1] + hit.obj.emission[1];
		blue = hit.obj.ambient[2] + hit.obj.emission[2];
		
		// Accumulate diffuse/specular/shininess contributions from each light
		for (int i = 0; i < numDirLights; i++) {
			vec3 direction = glm::normalize(vec3(directional[i].dirOrPos[0], directional[i].dirOrPos[1], directional[i].dirOrPos[2]));
			if (lightIsVisible(hit.location, direction, "dir", vec3(0, 0, 0))) {
				float diffCoeff; // diffuse coefficient is max(N dot L, 0)
				float specCoeff; // max(N dot H, 0)

				diffCoeff = glm::dot(hit.normal, direction);
				if (diffCoeff < 0) {
					diffCoeff = 0;
				}
				// Since it's a directional light, we don't consider attenuation
				red = red + ((directional[i].color[0])*(hit.obj.diffuse[0]*diffCoeff));
				green = green + ((directional[i].color[1])*(hit.obj.diffuse[1] * diffCoeff));
				blue = blue + ((directional[i].color[2])*(hit.obj.diffuse[2] * diffCoeff));
			}
		}

		for (int i = 0; i < numPointLights; i++) {
			vec3 lightPos = vec3(point[i].dirOrPos[0], point[i].dirOrPos[1], point[i].dirOrPos[2]);
			vec3 hitToLight = lightPos - hit.location; //hit.location - lightPos; // direction of light originates at light and goes to hit location
			float distance = glm::length(hitToLight);
			vec3 direction = glm::normalize(hitToLight);
			if (lightIsVisible(hit.location, direction, "pt", lightPos)) {
				float diffCoeff; // diffuse coefficient is max(N dot L, 0)
				float specCoeff; // max(N dot H, 0)
				vec3 H;
				/*vec3 R;
				// R = -L + 2*(L dot N)*N
				// Normalize L or not?
				R = glm::dot(direction, hit.normal)*hit.normal; // (L dot N) * N
				R[0] = R[0] * 2;
				R[1] = R[1] * 2;
				R[2] = R[2] * 2;
				R = R - direction;*/
				// H = (L + V)/|L + V|
				H = glm::normalize(direction - prevRay.direction);

				diffCoeff = glm::dot(glm::normalize(direction), hit.normal);
				specCoeff = glm::dot(glm::normalize(H), hit.normal);
				if (diffCoeff < 0) {
					diffCoeff = 0;
				}
				if (specCoeff < 0) {
					specCoeff = 0;
				}
				specCoeff = pow(specCoeff, hit.obj.shininess);
				float attenFactor = attenuation[0] + (attenuation[1] * distance) + (attenuation[2] * pow(distance,2));
				
				red = red + ((point[i].color[0] / (attenFactor))*((hit.obj.diffuse[0] * diffCoeff) + (hit.obj.specular[0] * specCoeff)));
				green = green + ((point[i].color[1] / (attenFactor))*((hit.obj.diffuse[1] * diffCoeff) + (hit.obj.specular[1] * specCoeff)));
				blue = blue + ((point[i].color[2] / (attenFactor))*((hit.obj.diffuse[2] * diffCoeff) + (hit.obj.specular[2] * specCoeff)));
							
			}
		}
		if (currDepth > 0) { // contributions from reflections
			vec3 R;
			// R = -V + 2*(V dot N)*N
			// Normalize L or not?
			R = -glm::dot(prevRay.direction, hit.normal)*hit.normal; // (L dot N) * N
			R[0] = R[0] * 2;
			R[1] = R[1] * 2;
			R[2] = R[2] * 2;
			R = R + prevRay.direction;
			ray reflectedRay;
			reflectedRay.direction = glm::normalize(R);
			reflectedRay.position[0] = hit.location[0] + reflectedRay.direction[0] * 0.001;
			reflectedRay.position[1] = hit.location[1] + reflectedRay.direction[1] * 0.001;
			reflectedRay.position[2] = hit.location[2] + reflectedRay.direction[2] * 0.001;
			intersect nextHit;
			nextHit.intersects = false;
			nextHit = rayThruObjects(reflectedRay);

			if (nextHit.intersects) {
				vec3 reflection = findColor(nextHit, currDepth - 1, reflectedRay);

				vec3 H = glm::normalize(reflectedRay.direction - prevRay.direction);
				float specCoeff = glm::dot(glm::normalize(H), glm::normalize(hit.normal));
				if (specCoeff < 0) {
					specCoeff = 0;
				}
				specCoeff = pow(specCoeff, hit.obj.shininess);
				float attenFactor = attenuation[0] + (attenuation[1] * nextHit.distance) + (attenuation[2] * pow(nextHit.distance, 2));
				red = red + (hit.obj.specular[0] * specCoeff * reflection[0] / attenFactor);
				green = green + (hit.obj.specular[1] * specCoeff * reflection[1] / attenFactor);
				blue = blue + (hit.obj.specular[2] * specCoeff * reflection[2] / attenFactor);
			}
		}
	}
	else {
		red = scene.bgcolor[0];
		green = scene.bgcolor[1];
		blue = scene.bgcolor[2];
	}

	// Clamp intensity to the range 0 < I < 1.0 (as a percentage of 255)
	if (red > 1.0) {
		red = 1.0;
	}
	else if (red < 0.0) {
		red = 0.0;
	}
	if (green > 1.0) {
		green = 1.0;
	}
	else if (green < 0.0) {
		green = 0.0;
	}
	if (blue > 1.0) {
		blue = 1.0;
	}
	else if (blue < 0.0) {
		blue = 0.0;
	}

	return vec3(red, green, blue);
}
