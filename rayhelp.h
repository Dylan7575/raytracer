static inline void normalize(double* v){
	double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}
typedef struct{
	double center[3];
	double diffuse[3];
	double specular[3];
	double radius;
	double refractivity;
	double reflectivity;
	double ior;
}Sphere;

typedef struct{
	double height;
	double width;
}Camera;

typedef struct{
	double position[3];
	double diffuse[3];
	double specular[3];
	double normal[3];
	double refractivity;
	double reflectivity;
	double ior;
}Plane;

typedef struct{
	int kind;
	double position[3];
	double direction[3];
	double radA2;
	double radA1;
	double radA0;
	double angA0;
	double color[3];
	double theta;
}Light;
typedef struct{
	int kind;
	Camera cam;
	Sphere sphere;
	Plane plane;

}Object; //storing them in an object array
typedef struct{
	int r;
	int g;
	int b;

}Pixel;
void write_p3(Pixel*,int w,int h,char*file_name);
Pixel trace(double* rd,double*ro,int depth);
double sphere_intersection(double* ro,double* rd,double rad,double* center );
double plane_intersection(double*ro,double*rd,double* normal,double* position);
static inline double frad(double lightDistance, double a0, double a1, double a2);
double clamp(double color);
static inline double fang(Light light,double*ron);
static inline void reflect(double *L, double *N, double *R);


static inline void specularReflection(double factor, double *L, double *R,
									  double *N, double *V, double *lColor,
									  double *oColor);
static inline void diffuseReflection(double *N, double *L, double *lColor,
									 double *oColor);
void vector_copy(double* vector1, double* vector2);
static inline void v3_add(double* a, double* b, double* c);
static inline void normalize(double* v);
static inline void v3_scale(double* a, double s, double* c);
static inline double v3_dot(double* a, double* b);
static inline void v3_subtract(double* a, double* b, double* c);

double distance(double *p1, double *p2);
int equals(Object a,Object b);
Object objects[129];
Light lights[129];