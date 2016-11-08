#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include <string.h>
#include <ctype.h>
#include "rayhelp.h"
#include "structs.h"
Pixel trace(double* rd,double*ro,int depth);
double* illuminate(Object object,double t_value,double *rd,double*ro);

int main(char*picture_width,char*picture_height,double c_height,double c_width) {
	int img_height = atoi(picture_height);
	int img_width = atoi(picture_width);
	double pix_height= c_height/img_height;
	double pix_width=c_width/img_width;
	double ro[3] = {0,0,0};
	int cx = 0;
	int cy=0;
	int i=0;
	Pixel pix[img_height*img_width];
	for(int y = img_height;y>0;y--){
		for(int x = x; x<img_width;x++){
			double rd[3]= {cx-(c_width)+pix_width*(x+.5),
						   cy-(c_height/2)+pix_height*(y+.5),
						   1};
			normalize(rd);
			pix[i]= trace(rd,ro,0);



		}
		i++;
	}
	write_p3(pix,img_height,img_width,"output.json");
	return 0;
}
Pixel trace(double* rd,double*ro,int depth) {
	double best_t = INFINITY;
	Object object;
	double color[3]={0,0,0};
	for (int i = 0; i < sizeof(objects); i++) {
		double t = 0;
		//switching based upon what type the object is and sending it to the right intersection
		switch (objects[i].kind) {
			case 1:
				t = sphere_intersection(ro, rd, objects[i].sphere.radius, objects[i].sphere.center);
				break;
			case 2:
				t = plane_intersection(ro, rd, objects[i].plane.normal, objects[i].plane.position);
				break;
			case 0:
				break;
			default:
				fprintf(stderr, "Unknown Type Found");
				exit(-1);
		}
		if (t > 0 && t < best_t) {
			best_t = t;
			object = objects[i];
		}
	}
	if(best_t>0 && best_t &&depth<=7){
		double * pixel=illuminate(object,best_t,ro,rd);
		v3_add(color,pixel,color);
		double nrd[3];
		double nro[3];
		double temp[3];
		if(depth<=7){
			if(object.kind==1){
				double N[3];
				v3_subtract(ro, object.sphere.center, N);
				v3_scale(rd, best_t, temp);//getting the new ray origin
				v3_add(temp, ro, nro);
				if(object.sphere.reflectivity>0){
					reflect(rd,N,nrd);
					trace(nrd,nro,depth+1);

				}
				if(object.sphere.refractivity>0){

				}

			}
			if(object.kind==2){
				reflect(rd,object.plane.normal,nrd);
				if(object.sphere.reflectivity>0){

				}
				if(object.sphere.refractivity>0){

				}
			}

		}
		if(object.kind==1){
			if(object.sphere.reflectivity>0){

			}
			if(object.sphere.refractivity>0){

			}

		}
		if(object.kind==2){
			if(object.sphere.reflectivity>0){

			}
			if(object.sphere.refractivity>0){

			}
		}

	}
	else{
		Pixel temp;
		temp.r=0;
		temp.g=0;
		temp.b=0;
		return temp;
	}
}
double* illuminate(Object object,double t_value,double *rd,double*ro) {

	double temp[3];
	double ron[3];
	double rdn[3];
	double color[3] = {0, 0, 0};
	double light_distance;


	for (int j = 0; j < sizeof(lights); j++) {
		//values for equations
		double *N = malloc(sizeof(double) * 3);
		double *L = malloc(sizeof(double) * 3);
		double *R = malloc(sizeof(double) * 3);
		double *V = malloc(sizeof(double) * 3);
		double *diffuse = malloc(sizeof(double) * 3);
		double *specular = malloc(sizeof(double) * 3);
		double *position = malloc(sizeof(double) * 3);

		double t = 0;

		v3_scale(rd, t_value, temp);//getting the new ray origin
		v3_add(temp, ro, ron);



		v3_subtract(lights[j].position, ron, rdn);//getting new ray direction

		//getting the light distance based upon which object type it is
		if (object.kind == 1) {
			light_distance = distance(lights[j].position, object.plane.position);
		}
		else if (object.kind == 2) {
			light_distance = distance(lights[j].position, object.sphere.center);
		}

		normalize(rdn);
		//shadowed value for if its in shadow using same intersection formula as above
		int shadowed;
		for (int i = 0; i < sizeof(objects); i++) {
			shadowed = 0;
			if (equals(objects[i],object))continue;

			//switching based upon what type the object is and sending it to the right intersection
			switch (objects[i].kind) {
				case 1:
					t = sphere_intersection(ron, rdn, objects[i].sphere.radius, objects[i].sphere.center);
					break;
				case 2:
					t = plane_intersection(ron, rdn, objects[i].plane.normal, objects[i].plane.position);
					break;
				case 0:
					break;
				default:
					fprintf(stderr, "Unknown Type Found");
					exit(-1);
			}
			if (t <= light_distance && t > 0 && t<INFINITY) {
				shadowed = 1;//showing that it is shadowed
			}

		}
		if (shadowed == 0) {//if it isnt go ahead with calculations

			//getting the diffuse, specular, and normal from structures
			if (object.kind == 1) {
				v3_subtract(ron, object.sphere.center, N);
				vector_copy(object.sphere.specular, specular);
				vector_copy(object.sphere.diffuse, diffuse);
			}
			if (object.kind == 2) {
				vector_copy(object.plane.diffuse, diffuse);
				vector_copy(object.plane.specular, specular);
				vector_copy(object.plane.normal, N);
			}

			normalize(N);


			vector_copy(rdn, L);//copying rdn into L for ease of matching the equations
			normalize(L);

			reflect(L, N, R);  //doing the reflection equation
			normalize(R);

			vector_copy(rd, V);//getting V and inverting it to get picture
			v3_scale(V, -1, V);



			specularReflection(10, L, R, N, V, lights[j].color, specular);//calculating specular
			diffuseReflection(N, L, lights[j].color, diffuse);//calculating diffuse
			//v3_scale(rdn, -1, V);
			double fRad = frad(light_distance, lights[j].radA0, lights[j].radA1, lights[j].radA2);//calculating radial attenuation
			double fAng = fang(lights[j],L);//calculating angular attenuation


			color[0] += (diffuse[0] + specular[0]) * fRad * fAng;//calculating color by adding and multiplying values up
			color[1] += (diffuse[1] + specular[1]) * fRad * fAng;
			color[2] += (diffuse[2] + specular[2]) * fRad * fAng;
			//printf("%f\n",color[2]);
		}
	}

	return color;
}