#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>


// storing camera data
typedef struct {
	double position[3];
	double width;
	double height;
} Camera;

// objects for storing data
typedef struct {
	int kind;
	double position[3];
	double diffuse[3];
	double specular[3];
	double reflectivity;
	double refractivity;
	double ior;
	union {
		struct {
			double radius;
		} sphere;
		struct {
			double normal[3];
		} plane;
	};
} Object;

// struct that stores lights
typedef struct {
	double color[3];
	double position[3];
	double direction[3];
	double theta;
	double radA2;
	double radA1;
	double radA0;
	double angA0;
} Light;

// struct that stores the pixels
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Pixel;


static inline double sqr(double v) {
	return v*v;
}


static inline void normalize(double* v) {
	double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));

	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}

typedef double* v3;

static inline void v3_add(v3 a, v3 b, v3 c) {
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}


static inline void v3_subtract(v3 a, v3 b, v3 c) {
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}


static inline void v3_scale(v3 a, double s, v3 c) {
	c[0] = s * a[0];
	c[1] = s * a[1];
	c[2] = s * a[2];
}


static inline double v3_dot(v3 a, v3 b) {
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

//initializing functions

void raycast(FILE*);
double* illuminate(double*, double*, Object);
double* shade(Object, double*, double*, int, double);
double* direct_shade(Object, double*, double*, double*, double*);

// intitializing line number Width and Height
int W;
int H;

//creating global variable for objects and lights and pixmap
Object** objects;
Light** lights;
Pixel* pixmap;
// default camera
Camera camera;
//defining epsilon constant
#define EPSILON .00001




