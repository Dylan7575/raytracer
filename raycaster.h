typedef struct{
	int r;
	int g;
	int b;

}Pixel;

Pixel * pix;
static inline double sqr(double v){
	return v*v;
}
double v3_len(double* vect){
	return sqrt(sqr(vect[0])+sqr(vect[1])+vect[2]);
}
void vector_copy(double* vector1, double* vector2){
	vector2[0]=vector1[0];
	vector2[1]=vector1[1];
	vector2[2]=vector1[2];
}
double diffuse(double light, double object, double diffuse_component) {
	if (diffuse_component > 0)
		return light * object * diffuse_component;

	return 0;
}

static inline void v3_add(double* a, double* b, double* c) {
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}
static inline void normalize(double* v){
	double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}
static inline void v3_scale(double* a, double s, double* c) {
	c[0] = s * a[0];
	c[1] = s * a[1];
	c[2] = s * a[2];
}
static inline double v3_dot(double* a, double* b) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static inline void v3_subtract(double* a, double* b, double* c) {
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}

double distance(double *p1, double *p2) {
	return sqrt(sqr(p2[0] - p1[0]) + sqr(p2[1] - p1[1]) +
				sqr(p2[2] - p1[2]));
}

static inline double frad(double lightDistance, double a0, double a1, double a2) {

	if (lightDistance == INFINITY||a0==0.0&&a1==0.0&&a2==0.0) {
		return 1.0;
	}
	else {
		return 1/(a2 * sqr(lightDistance) + a1 * lightDistance + a0);
	}
}
double clamp(double color){
	if(color>1){
		return 1;
	}
	if (color<0){
		return 0;
	}
	return color;
}
double fang(double* light_direction, double* light_to_obj, double a0, double theta) {
	if (light_direction[0] == 0 && light_direction[1] == 0 && light_direction[2] == 0)
		return 1;

	if (v3_dot(light_to_obj, light_direction) < (cos(theta / 180 * 3.14)) * (180 / 3.14))
		return 0;

	return pow(v3_dot(light_to_obj, light_direction), a0);
}
static inline void reflect(double *L, double *N, double *R) {
	double temp = 2 * v3_dot(L, N);
	double temp2[3];
	v3_scale(N,temp , temp2);
	v3_subtract(temp2, L, R);
}


double specular(double light, double object, double diffuse_component, double specular_component) {
	if (specular_component > 0 && diffuse_component > 0)
		return light * object * pow(specular_component, 20);

	return 0;
}
static inline void diffuseReflection(double *N, double *L, double *lColor,
									 double *oColor) {
	double temp = v3_dot(N, L);//getting dot product
	if (temp > 0) {
		double temp2[3];
		temp2[0] = lColor[0] * oColor[0];//multiplying by light color
		//printf("%f\n",temp2[0]);
		temp2[1] = lColor[1] * oColor[1];
		//printf("%f\n",temp2[1]);
		temp2[2] = lColor[2] * oColor[2];
		//printf("%f\n",temp2[2]);
		v3_scale(temp2,temp,oColor);//scaling the colors by the dotproduct of the normal and the new ray direction
	} else {
		oColor[0] = 0;
		oColor[1] = 0;
		oColor[2] = 0;
	}
}



static inline void v3_cross(double* a, double* b, double* c) {
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}



double* refraction(double outer_ior, double inner_ior, double* ray_in, double* N) {
	double* ray_out = malloc(sizeof(double)*3);

	double eta, cosine_theta, cosine_squared;
	eta = outer_ior / inner_ior;
	cosine_theta = v3_dot(ray_in, N) * -1;
	cosine_squared = 1 - eta * eta * (1 - cosine_theta * cosine_theta);

	if (cosine_squared < 0) { //internal reflection
		ray_out[0] = 0.0;
		ray_out[1] = 0.0;
		ray_out[2] = 0.0;
		return ray_out;
	}

	// linear combination
	double* temp = malloc(3 * sizeof(double));
	v3_scale(ray_in, eta, temp);
	v3_scale(N, eta * cosine_theta - sqrt(cosine_squared), ray_out);
	v3_add(temp, ray_out, ray_out);
	//normalize ray_out
	normalize(ray_out);

	return ray_out;
}
int equals(Object a,Object b){
	if(a.kind==b.kind){
		if(a.kind==1){
			if(a.sphere.center[0]==b.sphere.center[0]&&a.sphere.center[1]==b.sphere.center[1]&&a.sphere.center[2]==b.sphere.center[2]&&
					a.sphere.diffuse[0]==b.sphere.diffuse[0]&&a.sphere.diffuse[1]==b.sphere.diffuse[1]&&a.sphere.diffuse[2]==b.sphere.diffuse[2]&&
					a.sphere.specular[0]==b.sphere.specular[0]&&a.sphere.specular[1]==b.sphere.specular[1]&&a.sphere.specular[2]==b.sphere.specular[2]&&
					a.sphere.radius==b.sphere.radius&&a.sphere.ior==b.sphere.ior&&b.sphere.reflectivity==b.sphere.reflectivity&&a.sphere.refractivity==b.sphere.refractivity
			){
				return 1;
			}
			else{
				return 0;
			}
		}
		if(a.kind==2){
			if(a.plane.position[0]==b.plane.position[0]&&a.plane.position[1]==b.plane.position[1]&&a.plane.position[2]==b.plane.position[2]&&
			   a.plane.diffuse[0]==b.plane.diffuse[0]&&a.plane.diffuse[1]==b.plane.diffuse[1]&&a.plane.diffuse[2]==b.plane.diffuse[2]&&
			   a.plane.normal[0]==b.plane.normal[0]&& a.plane.normal[1]==b.plane.normal[1]&& a.plane.normal[2]==b.plane.normal[2]&&
			   a.plane.specular[0]==b.plane.specular[0]&&a.plane.specular[1]==b.plane.specular[1]&&a.plane.specular[2]==b.plane.specular[2]&&
			   a.plane.refractivity==b.plane.refractivity&&a.plane.ior==b.plane.ior&&b.plane.reflectivity==b.plane.reflectivity){
				return 1;
			}
		}
		else return 0;
	}
	else return 0;
}


/*********************Sphere Intersection*************/
double sphere_intersection(double* ro,double* rd,double rad,double* center ){
	//doing math
	double a = sqr(rd[0]) + sqr(rd[1]) + sqr(rd[2]);
	double b = 2 * (rd[0] * (ro[0] - center[0]) + rd[1] * (ro[1] - center[1]) + rd[2] * (ro[2] - center[2]));
	double c = sqr(ro[0] - center[0]) + sqr(ro[1] - center[1]) + sqr(ro[2] - center[2]) - sqr(rad);


	double det = sqr(b) - 4 * a * c;//finding determinant


	if (det < 0)
		return -1;

	det = sqrt(det);

	//for quadratic equation can be positive and it can be negative so we do it twice.
	double t0 = (-b - det) / (2 * a);// One for it being negative

	if (t0 > 0)
		return t0;//returning t value

	double t1 = (-b + det) / (2 * a);// One for it being positive
	if (t1 > 0)
		return t1;//returing t value

	return -1;
}

/*******************Plane Intersection*****************/
double plane_intersection(double*ro,double*rd,double* normal,double* position){
	//doing math;
	normalize(normal);
	double D = -(v3_dot(position,normal)); // distance from origin to plane
	double t = -(normal[0] * ro[0] + normal[1] * ro[1] + normal[2] * ro[2] + D) /
			   (normal[0] * rd[0] + normal[1] * rd[1] + normal[2] * rd[2]);

	if (t > 0)
		return t;//returning t value

	return -1;
}

