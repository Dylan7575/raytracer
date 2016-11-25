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
void read_scene(FILE*);
void raycast(FILE*);
double* illuminate(double*, double*, Object);
double* shade(Object, double*, double*, int, double);
double* direct_shade(Object, double*, double*, double*, double*);
double* refraction(double, double, double*, double*);

double diffuse(double, double, double);
double specular(double, double, double, double);
double frad(double, double, double, double);
double fang(double*, double*, double, double);
void parse_camera(FILE*);
void parse_sphere(FILE*, Object*);
void parse_plane(FILE*, Object*);
void parse_light(FILE*, Light*);
double sphere_intersect(double*, double*, double*, double);
double plane_intersect(double*, double*, double*, double*);
void skip_ws(FILE*);
void expect_c(FILE*, int);
int next_c(FILE*);
char* next_string(FILE*);
double next_number(FILE*);
double* next_vector(FILE*);
double clamp(double);
void output_picture(FILE*);
double shoot(double* Rd,double* Ro,Object object);
void reflect(double*,double*,double*);
// intitializing line number Width and Height
int line = 1;
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

double* refraction(double outer_ior, double inner_ior, double* ray_in, double* N) {
	//calculating refraction
	double* ray_out = malloc(sizeof(double)*3);

	double eta, cosine_theta, cosine_squared;
	eta = outer_ior / inner_ior;
	cosine_theta = v3_dot(ray_in, N) * -1;
	cosine_squared = 1 - eta * eta * (1 - cosine_theta * cosine_theta);

	if (cosine_squared < 0) {
		ray_out[0] = 0.0;
		ray_out[1] = 0.0;
		ray_out[2] = 0.0;
		return ray_out;
	}


	double* temp = malloc(3 * sizeof(double));
	v3_scale(ray_in, eta, temp);
	v3_scale(N, eta * cosine_theta - sqrt(cosine_squared), ray_out);
	v3_add(temp, ray_out, ray_out);

	normalize(ray_out);

	return ray_out;
}

//comparing two objects
int equals(Object a, Object b) {
	if (a.kind == b.kind &&
		a.diffuse[0] == b.diffuse[0] &&
		a.diffuse[1] == b.diffuse[1] &&
		a.diffuse[2] == b.diffuse[2] &&
		a.specular[0] == b.specular[0] &&
		a.specular[1] == b.specular[1] &&
		a.specular[2] == b.specular[2] &&
		a.position[0] == b.position[0] &&
		a.position[1] == b.position[1] &&
		a.position[2] == b.position[2]) {
		if (a.kind ==0 &&
			a.sphere.radius == b.sphere.radius) {
			return 1;
		} else if (a.kind == 1 &&
				   a.plane.normal[0] == b.plane.normal[0] &&
				   a.plane.normal[1] == b.plane.normal[1] &&
				   a.plane.normal[2] == b.plane.normal[2]) {
			return 1;
		}
	}

	return 0;
}
double frad(double dl, double a0, double a1, double a2) {
	if (dl == INFINITY)
		return 1;

	return 1 / (a2*sqr(dl) + (a1*dl) + a0);
}

// calculates angular attenuation
double fang(double* light_direction, double* light_to_obj, double a0, double theta) {
	if (light_direction[0] == 0 && light_direction[1] == 0 && light_direction[2] == 0)
		return 1;

	if (v3_dot(light_to_obj, light_direction) < (cos(theta / 180 * 3.14)) * (180 / 3.14))
		return 0;

	return pow(v3_dot(light_to_obj, light_direction), a0);
}
double shoot(double* Rd,double* Ro,Object object){
	double t = 0;
	switch (object.kind) {
		case 0:
			t = sphere_intersect(Ro, Rd, object.position,object.sphere.radius);
			break;
		case 1:
			t = plane_intersect(Ro, Rd, object.position, object.plane.normal);
			break;

		default:
			fprintf(stderr, "Error: Unknown object.\n");
			exit(1);
	}
	return t;
}

void write_p3(FILE* fh){

	fprintf(fh,"P3 %d %d 255 \n",W,H);//writing header
	for(int i = 0;i<W*H;i++){
		//looping through pix and writing it to the file
		fprintf(fh,"%d ",pixmap[i].r);
		fprintf(fh,"%d ",pixmap[i].g);
		fprintf(fh,"%d \n",pixmap[i].b);
	}
	fclose(fh);//closing file handle
}
double clamp(double number) {
	// clamps number
	if (number < 0)
		return 0;

	if (number > 1)
		return 1;

	return number;
}
double sphere_intersect(double* Ro, double* Rd, double* C, double r) {
	double a = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
	double b = 2*(Rd[0]*(Ro[0]-C[0]) + Rd[1]*(Ro[1]-C[1]) + Rd[2]*(Ro[2]-C[2]));
	double c = sqr(Ro[0]-C[0]) + sqr(Ro[1]-C[1]) + sqr(Ro[2]-C[2]) - sqr(r);

	// check determinant
	double det = sqr(b) - 4*a*c;
	if (det < 0)
		return -1;

	det = sqrt(det);

	// return t value if an intersect was found, otherwise return -1
	double t0 = (-b - det) / (2*a);
	if (t0 > 0)
		return t0;

	double t1 = (-b + det) / (2*a);
	if (t1 > 0)
		return t1;

	return -1;
}


double plane_intersect(double* Ro, double* Rd, double* P, double* N) {

	double d = -(N[0]*P[0] + N[1]*P[1] + N[2]*P[2]);
	double t = -(N[0]*Ro[0] + N[1]*Ro[1] + N[2]*Ro[2] + d) / (N[0]*Rd[0] + N[1]*Rd[1] + N[2]*Rd[2]);


	if (t > 0)
		return t;

	return -1;
}
void reflect(double *L, double *N, double *R) {

	double temp2[3];
	v3_scale(N,2*v3_dot(L,N) , temp2);
	v3_subtract(L, temp2, R);
}
static inline void diffuseReflection(double *N, double *L, double *lColor,
									 double *oColor) {
	double temp = v3_dot(N, L);
	if (temp > 0) {
		double temp2[3];
		temp2[0] = lColor[0] * oColor[0];

		temp2[1] = lColor[1] * oColor[1];

		temp2[2] = lColor[2] * oColor[2];

		v3_scale(temp2,temp,oColor);
	}
	else {
		oColor[0] = 0;
		oColor[1] = 0;
		oColor[2] = 0;
	}
}
static inline void specularReflection(double factor, double *L, double *R,
									  double *N, double *V, double *lColor,
									  double *oColor) {
	double VR = v3_dot(V, R);
	double NL = v3_dot(N, L);
	if (VR > 0 && NL > 0) {
		double temp = pow(VR, factor);
		double temp2[3];
		temp2[0] = lColor[0] * oColor[0];
		temp2[1] = lColor[1] * oColor[1];
		temp2[2] = lColor[2] * oColor[1];
		v3_scale(temp2,temp,oColor);

	} else {
		oColor[0] = 0;
		oColor[1] = 0;
		oColor[2] = 0;
	}
}
void vector_copy(double* vector1, double* vector2){
	vector2[0]=vector1[0];
	vector2[1]=vector1[1];
	vector2[2]=vector1[2];
}
