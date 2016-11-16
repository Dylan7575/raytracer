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