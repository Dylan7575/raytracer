#include "includes.h"
#include "raycaster.h"
//struct for storing pixels to write to ppm file
double* illuminate(double* Rd, double* Ro, Object closest_object,Light* lights,Object* objects);
double* shade(Object current_object, double* rd, double* ro, int level, double ior,Light* lights,Object* objects);
double* direct_shade(Object* current_object, double* pixel_position, double* light_color, double* light_direction, double* light_position);
/***************Writing to pixel buffer to output file*****************/
void write_p3(Pixel* pixel,int w,int h,char* filename){

    FILE *fh = fopen(filename,"w+");//opening the file handle
    if(fh==NULL){
        fprintf(stderr,"Could not Open Output File");
        exit(1);
    }
    fprintf(fh,"P3 %d %d 255 \n",w,h);//writing header
    for(int i = 0;i<w*h;i++){
        //looping through pix and writing it to the file
        fprintf(fh,"%d ",pixel[i].r);
        fprintf(fh,"%d ",pixel[i].g);
        fprintf(fh,"%d \n",pixel[i].b);
    }
    fclose(fh);//closing file handle
}
double shoot(double* Rd,double* Ro,Object object){
	double t = 0;
	switch (object.kind) {
		case 1:
			t = sphere_intersection(Ro, Rd,object.sphere.radius, object.sphere.center);
			break;
		case 2:
			t = plane_intersection(Ro, Rd, object.plane.position, object.plane.normal);
			break;
		case 0:
			break;
		default:
			fprintf(stderr, "Error: Unknown object.\n");
			exit(1);
	}
	return t;
}
void output_picture(char* outputfp,int H,int W,Pixel* pixmap) {
	// create header
	FILE *fh = fopen(outputfp,"w+");
	fprintf(fh, "P6\n%d %d\n%d\n", H, W, 255);
	// writes buffer to output Pixel by Pixel
	fwrite(pixmap, sizeof(Pixel), H*W, fh);
}

void raycast(Object* objects,Light* lights,char* picture_width,char* picture_height,char* output_file) {
	double cx = 0;
	double cy = 0;
	double cz =0;
	int j;
	for(j=0;j< sizeof(objects);j++){
		if(objects[j].kind==0){
			break;
		}
	}
	// calculate pixel height, width
	double pixheight = objects[j].cam.height/atoi(picture_height);
	double pixwidth = objects[j].cam.width/atoi(picture_width);

	// allocate space for number of pixels needed
	Pixel *pixmap = malloc(sizeof(Pixel)*atoi(picture_height)*atoi(picture_width));
	// initialize pixmap index
	int pixmap_index = 0;

	// got through each spot pixel by pixel to see what color it should be
	for (int y=atoi(picture_height); y>0; y--) {
		for (int x=0; x<atoi(picture_width); x++) {
			// ray origin
			double Ro[3] = {cx, cy, cz};
			// ray destination
			double Rd[3] = {cx - ( objects[j].cam.width/2) + pixwidth*(x + 0.5),
							cy - ( objects[j].cam.height/2) + pixheight*(y + 0.5),
							1};
			normalize(Rd);

			double best_t = INFINITY;
			Object closest_object;
			// look for intersection of an object
			for (int i=0; i< sizeof(objects); i++) {
				double t = 0;
				switch (objects[i].kind) {
					case 1:
						t = sphere_intersection(Ro, Rd,objects[i].sphere.radius ,objects[i].sphere.center );
						break;
					case 2:
						t = plane_intersection(Ro, Rd, objects[i].plane.position, objects[i].plane.normal);
						break;
					case 0:
						break;
					default:
						fprintf(stderr, "Error: Unknown object.\n");
						exit(1);
				}

				// save object if it intersects closer to the camera
				if (t > 0 && t < best_t) {
					best_t = t;
					closest_object = objects[i];
				}
			}

			if (best_t > 0 && best_t != INFINITY) {
				double Ron[3];
				v3_scale(Rd, best_t, Ron);
				v3_add(Ron, Ro, Ron);

				double* color = shade(closest_object, Rd, Ron, 0, 1.0,lights,objects);

				// save pixel to pixmap buffer
				pixmap[pixmap_index].r = (unsigned char) (clamp(color[0])*255);
				pixmap[pixmap_index].g = (unsigned char) (clamp(color[1])*255);
				pixmap[pixmap_index].b = (unsigned char) (clamp(color[2])*255);
			} else {
				pixmap[pixmap_index].r = 0;
				pixmap[pixmap_index].g = 0;
				pixmap[pixmap_index].b = 0;
			}

			pixmap_index++;
		}
	}
	write_p3(pixmap,atoi(picture_width),atoi(picture_height),output_file);
	//output_picture(output_file,atoi(picture_height),atoi(picture_width),pixmap);
}
/*******************Raycasting*************************/

/*
void raycast(Object* objects,Light* lights,char* picture_width,char* picture_height,char* output_file){

    int j=0,k=0;//loops

    //camera position//
    double cx=0;
    double cy=0;


    //finding the camera in the Objects
    for(j=0;j< sizeof(objects);j++){
        if(objects[j].kind==0){
            break;
        }
    }



    double h =objects[j].cam.height;
    double w=objects[j].cam.width;
    int m =atoi(picture_height);
    int n = atoi(picture_width);
    double pixheight =h/m;
    double pixwidth =w/n;


    Pixel p[m*n];//creating pixel array
    Pixel* pix=malloc(m*n);
	Object object;
	double best_t=INFINITY;

    for(int y=m;y>0;y--) {
        for (int x = 0; x < n; x++) {
            double ro[3] = {0, 0, 0};//ray origin
            double rd[3] = {     //ray direction
                    cx - (w / 2) + pixwidth * (x + 0.5),
                    cy - (h / 2) + pixheight * (y + 0.5),
                    1
            };
            normalize(rd);//normalizing the ray direction
			for(int i=0;i<sizeof(objects);i++){
				double t = shoot(rd,ro,objects[i]);
				if(t>0&&t<best_t){
					best_t=t;
					object= objects[i];
				}
			}
			if(best_t>0 && best_t!=INFINITY){
				double ron[3];
				v3_scale(rd,best_t,ron);
				v3_add(ron,ro,ron);
				double* temp=shade(object,rd,ro,0,1,lights,objects);

				p[k].r= (unsigned char)255*clamp(temp[0]);
				p[k].g= (unsigned char)255*clamp(temp[1]);
				p[k++].b=(unsigned char)255*clamp(temp[2]);
			}
			else{
				p[k].r=0;
				p[k].g=0;
				p[k++].b=0;
			}
        }
    }
	//output_picture(p,h,w,output_file);

    write_p3(p,atoi(picture_height),atoi(picture_width),output_file);
}
*/
double* shade(Object current_object, double* rd, double* ro, int level, double ior,Light* lights,Object* objects) {
	//printf("%d",current_object.kind);

	double* color = malloc(sizeof(double)*3);
	double reflectivity;
	double refractivity;
	double currior;
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;

	if (level >= 7) {
		return color;
	}
	double * temp =illuminate(rd, ro, current_object,lights,objects);
	//return temp;
	v3_add(color, temp, color);



	double N[3];
	if (current_object.kind == 1) {
		v3_subtract(ro, current_object.sphere.center, N);
		refractivity=current_object.sphere.refractivity;
		reflectivity=current_object.sphere.reflectivity;
		currior=current_object.sphere.ior;

	} else {
		refractivity=current_object.plane.refractivity;
		reflectivity=current_object.plane.reflectivity;
		vector_copy(current_object.plane.normal,N);
		currior=current_object.plane.ior;
	}
	normalize(N);

	if (reflectivity > 0) {
		double reflected_ray[3];
		v3_scale(N, 2*v3_dot(N, rd), reflected_ray);
		v3_subtract(rd, reflected_ray, reflected_ray);
		normalize(reflected_ray);

		double best_t = INFINITY;
		Object* closest_object;
		// look for intersection of an object
		for (int i=0; i< sizeof(objects); i++) {
			// skip checking for intersection with the object already being looked at
			if (equals(current_object, objects[i]))
				continue;

			double* offset = malloc(sizeof(double)*3);
			v3_scale(reflected_ray, 0.00001, offset);
			v3_add(offset, ro, offset);

			double t = 0;
			switch (objects[i].kind) {
				case 1:
					t = sphere_intersection(offset, reflected_ray,objects[i].sphere.radius, objects[i].sphere.center);
					break;
				case 2:
					t = plane_intersection(offset, reflected_ray, objects[i].plane.position, objects[i].plane.normal);
					break;
				case 0:
					break;
				default:
					fprintf(stderr, "Error: Unknown object.\n");
					exit(1);
			}

			// save object if it intersects closer to the camera
			if (t > 0 && t < best_t) {
				best_t = t;
				closest_object = &objects[i];
			}
		}

		if (best_t > 0 && best_t != INFINITY) {
			double* Ron = malloc(sizeof(double)*3);
			v3_scale(reflected_ray, best_t, Ron);
			v3_add(ro, Ron, Ron);

			double* reflected_color = malloc(sizeof(double)*3);
			v3_scale(shade(*closest_object, reflected_ray, Ron, level+1, currior,lights,objects), reflectivity, reflected_color);

			v3_add(color, reflected_color, color);
		}
	}

	if (refractivity > 0) {
		double* refracted_ray = refraction(ior, currior, rd, N);

		double* offset = malloc(sizeof(double)*3);
		v3_scale(refracted_ray, 0.00001, offset);
		v3_add(offset, ro, offset);

		double d;
		switch (current_object.kind) {
			case 1:
				d = sphere_intersection(offset, refracted_ray,current_object.sphere.radius ,current_object.sphere.center );
				break;
			case 2:
				d = plane_intersection(offset, refracted_ray, current_object.plane.position, current_object.plane.normal);
				break;
			case 0:
				break;
			default:
				fprintf(stderr, "Error: Unknown object.\n");
				exit(1);
		}

		double* back = malloc(sizeof(double)*3);
		v3_scale(refracted_ray, d, back);
		v3_add(ro, back, back);

		double* back_normal = malloc(sizeof(double)*3);
		if (current_object.kind == 1) {
			v3_subtract(back, current_object.sphere.center, back_normal);
		} else if (current_object.kind == 2) {
			back_normal = current_object.plane.normal;
			v3_scale(back_normal, -1, back_normal);
		}
		normalize(back_normal);

		double* next_ray = refraction(currior, 1.0, refracted_ray, back_normal);

		double best_t = INFINITY;
		Object* closest_object;
		// look for intersection of an object
		for (int i=0; i< sizeof(objects); i++) {
			// skip checking for intersection with the object already being looked at
			if (equals(current_object, objects[i]))
				continue;

			double* offset = malloc(sizeof(double)*3);
			v3_scale(refracted_ray, 0.00001, offset);
			v3_add(offset, ro, offset);

			double t = 0;
			switch (objects[i].kind) {
				case 1:
					t = sphere_intersection(offset, next_ray, objects[i].sphere.radius,objects[i].sphere.center);
					break;
				case 2:
					t = plane_intersection(offset, next_ray, objects[i].plane.position, objects[i].plane.normal);
					break;
				case 0:
					break;
				default:
					fprintf(stderr, "Error: Unknown object.\n");
					exit(1);
			}

			// save object if it intersects closer to the camera
			if (t > 0 && t < best_t) {
				best_t = t;
				closest_object = &objects[i];
			}
		}

		if (best_t > 0 && best_t != INFINITY) {
			double* Ron = malloc(sizeof(double)*3);
			v3_scale(next_ray, best_t, Ron);
			v3_add(ro, Ron, Ron);

			double* refracted_color = malloc(sizeof(double)*3);
			v3_scale(shade(*closest_object, next_ray, Ron, level+1, currior,lights,objects), reflectivity, refracted_color);

			v3_add(color, direct_shade(closest_object, Ron, refracted_color, next_ray, back), color);
			v3_add(color, refracted_color, color);
		}
	}


	return color;
}
double* illuminate(double* Rd, double* Ro, Object closest_object,Light* lights,Object* objects) {
	double* current_color = malloc(sizeof(double)*3);
	current_color[0] = 0;
	current_color[1] = 0;
	current_color[2] = 0;

	double pixel_position[3];
	pixel_position[0] = Ro[0];
	pixel_position[1] = Ro[1];
	pixel_position[2] = Ro[2];
	double specularc[3];
	double diffusec[3];

	double obj_to_cam[3];
	double camera[3]= {0,0,0};
	// find position of pixel in space and get vector from it to the camera
	v3_subtract(camera, pixel_position, obj_to_cam);
	normalize(obj_to_cam);

	// find and save object normal
	double N[3];
	if (closest_object.kind == 1) {
		v3_subtract(pixel_position, closest_object.sphere.center, N);
		vector_copy(closest_object.sphere.diffuse,diffusec);
		vector_copy(closest_object.sphere.specular,specularc);

	} else {
		N[0] = closest_object.plane.normal[0];
		N[1] = closest_object.plane.normal[1];
		N[2] = closest_object.plane.normal[2];
		vector_copy(closest_object.plane.diffuse,diffusec);
		vector_copy(closest_object.plane.specular,specularc);
	}
	normalize(N);

	// look through light to find the ones that influence this pixel
	Light* current_light;
	Object current_object;
	double new_t = 0;
	for (int j=0; j <sizeof(lights); j++) {
		current_light = &lights[j];

		// find vector from light to the object
		double light_to_obj[3];
		v3_subtract(pixel_position, current_light->position, light_to_obj);
		normalize(light_to_obj);

		// find vector from the object to the light
		double obj_to_light[3];
		v3_subtract(current_light->position, pixel_position, obj_to_light);
		normalize(obj_to_light);

		// find distance from the light to the object
		double dl = sqrt(sqr(pixel_position[0]-current_light->position[0]) + sqr(pixel_position[1]-current_light->position[1]) + sqr(pixel_position[2]-current_light->position[2]));

		// boolean that tells if object is in a shadow
		int in_shadow = 0;
		for (int k=0; k<sizeof(objects); k++) {
			current_object = objects[k];
			// skip checking for intersection with the object already being looked at
			if (equals(current_object, closest_object))
				continue;

			double* offset = malloc(sizeof(double)*3);
			v3_scale(obj_to_light, 0.00001, offset);
			v3_add(offset, pixel_position, offset);



			// find new intersection between light and each object looking for one that is closer to the light
			// and casts a shadow on the closest object to the camera at this pixel

			switch (current_object.kind) {
				case 1:
					new_t = sphere_intersection(offset, obj_to_light,current_object.sphere.radius, current_object.sphere.center );
					break;
				case 2:
					new_t = plane_intersection(offset, obj_to_light, current_object.plane.position, current_object.plane.normal);
					break;
				case 0:
					break;
				default:
					fprintf(stderr, "Error: Unknown object.erq   \n");
					exit(1);
			}

			// if there is a closer object to the light, then there is a shadow
			if (new_t > 0 && new_t <= dl) {
				in_shadow = 1;
				break;
			}
		}

		// if not in a shadow then modify the color of the pixel using diffuse/specular components, and the color of the light
		if (in_shadow == 0) {
			double R[3];
			v3_scale(N, 2*v3_dot(N, light_to_obj), R);
			v3_subtract(light_to_obj, R, R);
			normalize(R);


			// calculate (N*L)
			double diffuse_component = v3_dot(N, obj_to_light);
			// calculate (R*V)
			double specular_component = v3_dot(R, obj_to_cam);

			// normalize light direction
			double* light_direction = current_light->direction;
			normalize(light_direction);

			// find diffuse color
			double diffuse_color[3];

			diffuse_color[0] = diffuse(current_light->color[0], diffusec[0], diffuse_component);
			diffuse_color[1] = diffuse(current_light->color[1], diffusec[1], diffuse_component);
			diffuse_color[2] = diffuse(current_light->color[2], diffusec[2], diffuse_component);

			// find specular color
			double specular_color[3];
			specular_color[0] = specular(current_light->color[0], specularc[0], diffuse_component, specular_component);
			specular_color[1] = specular(current_light->color[1], specularc[1], diffuse_component, specular_component);
			specular_color[2] = specular(current_light->color[2], specularc[2], diffuse_component, specular_component);

			// get attenuation values
			double rad = frad(dl, current_light->angA0, current_light->radA1, current_light->radA2);
			double ang = fang(light_direction, light_to_obj, current_light->angA0, current_light->theta);

			// modify color if pixel to reflect changes from illumination
			current_color[0] += rad * ang * (diffuse_color[0] + specular_color[0]);
			current_color[1] += rad * ang * (diffuse_color[1] + specular_color[1]);
			current_color[2] += rad * ang * (diffuse_color[2] + specular_color[2]);
		}
	}

	return current_color;
}
double* direct_shade(Object* current_object, double* pixel_position, double* light_color, double* light_direction, double* light_position) {
	double* color = malloc(sizeof(double)*3);
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	double diffc[3];
	double specc[3];
	double* N = malloc(sizeof(double)*3);
	if (current_object->kind == 1) {
		v3_subtract(pixel_position, current_object->sphere.center, N);
		vector_copy(diffc,current_object->sphere.diffuse);
		vector_copy(diffc,current_object->sphere.specular);

	}
	if(current_object->kind==1){
		N = current_object->plane.normal;
		vector_copy(diffc,current_object->plane.diffuse);
		vector_copy(diffc,current_object->plane.specular);
	}
	normalize(N);

	// find vector from the object to the light
	double obj_to_light[3];
	v3_subtract(light_position, pixel_position, obj_to_light);
	normalize(obj_to_light);

	double obj_to_cam[3];
	double camera[3]={0,0,0};
	v3_subtract(camera, pixel_position, obj_to_cam);
	normalize(obj_to_cam);

	double R[3];
	v3_scale(N, 2*v3_dot(N, light_direction), R);
	v3_subtract(light_direction, R, R);
	normalize(R);

	// calculate (N*L)
	double diffuse_component = v3_dot(N, obj_to_light);
	// calculate (R*V)
	double specular_component = v3_dot(R, obj_to_cam);

	// find diffuse color
	double diffuse_color[3];
	diffuse_color[0] = diffuse(light_color[0], diffc[0], diffuse_component);
	diffuse_color[1] = diffuse(light_color[1], diffc[1], diffuse_component);
	diffuse_color[2] = diffuse(light_color[2], diffc[2], diffuse_component);

	// find specular color
	double specular_color[3];
	specular_color[0] = specular(light_color[0], specc[0], diffuse_component, specular_component);
	specular_color[1] = specular(light_color[1], specc[1], diffuse_component, specular_component);
	specular_color[2] = specular(light_color[2], specc[2], diffuse_component, specular_component);

	// modify color if pixel to reflect changes from illumination
	color[0] += (diffuse_color[0] + specular_color[0]);
	color[1] += (diffuse_color[1] + specular_color[1]);
	color[2] += (diffuse_color[2] + specular_color[2]);

	return color;
}