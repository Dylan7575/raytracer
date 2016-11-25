#include "raytrace.h"
#include "parse.c"


int main(int argc, char** argv) {
    //checking if aount of arguments is correct
    if (argc != 5) {
        fprintf(stderr, "Error need to enter height width source.json output.ppm\n");
        exit(1);
    }

        //checking if width is in correct form
    W = atoi(argv[1]);
    if (W == 0) {
        fprintf(stderr, "Error: width must be a a positive integer \n");
        exit(1);
    }
//checking if height is in correct form
    H = atoi(argv[2]);
    if (H==0) {
        fprintf(stderr, "Error: Height must be a positive integer\n");
        exit(1);
    }

//checking if json file
	int jsonlen=strlen(argv[3]);
	char * last4 = &argv[3][jsonlen-4];

    if (strcmp("json",last4)!=0||argv[3]==NULL) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", argv[3]);
        exit(1);
    }
    FILE* json = fopen(argv[3], "r");


    objects = malloc(sizeof(Object*)*129);
    lights = malloc(sizeof(Light*)*129);


    read_scene(json);


    raycast(json);

//checking if output file is correct
    FILE* output = fopen(argv[4], "w+");
    if (output == NULL) {
        fprintf(stderr, "Error: Could not create file '%s'.\n", argv[4]);
        exit(1);
    }

//writing to p3 format
    write_p3(output);
    fclose(output);

//freeing memory
    for (int i=0; objects[i] != NULL; i++)
        free(objects[i]);
    free(objects);

    for (int i=0; lights[i] != NULL; i++)
        free(lights[i]);
    free(lights);

    return (EXIT_SUCCESS);
}
//sending out rays to file
void raycast(FILE* json) {
    double cx = camera.position[0];
    double cy = camera.position[1];
    double cz = camera.position[2];

//getting pixel height and width
    double pixheight = camera.height/H;
    double pixwidth = camera.width/W;


    pixmap = malloc(sizeof(Pixel)*H*W);

    int pixmap_index = 0;

    for (int y=H; y>0; y--) {
        for (int x=0; x<W; x++) {

            double Ro[3] = {cx, cy, cz};

            double Rd[3] = {cx - (camera.width/2) + pixwidth*(x + 0.5),
                            cy - (camera.height/2) + pixheight*(y + 0.5),
                            1};
            normalize(Rd);

            double best_t = INFINITY;
            Object closest_object;

            for (int i=0; objects[i] != NULL; i++) {
                //checking for intersections while looping through objects
                double t = 0;
                t=shoot(Rd,Ro,*objects[i]);



                if (t > 0 && t < best_t) {
                    best_t = t;
                    closest_object = *objects[i];
                }
            }

            if (best_t > 0 && best_t != INFINITY) {
                double Ron[3];
                v3_scale(Rd, best_t, Ron);
                v3_add(Ron, Ro, Ron);

                double* color = shade(closest_object, Rd, Ron, 0, 1.0);

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
}


double* illuminate(double* Rd, double* Ro, Object closest_object) {
    double* current_color = malloc(sizeof(double)*3);
    current_color[0] = 0;
    current_color[1] = 0;
    current_color[2] = 0;

    double pixel_position[3];
    pixel_position[0] = Ro[0];
    pixel_position[1] = Ro[1];
    pixel_position[2] = Ro[2];

    double obj_to_cam[3];


    v3_subtract(camera.position, pixel_position, obj_to_cam);
    normalize(obj_to_cam);


    double N[3];
    if (closest_object.kind == 0) {
        v3_subtract(pixel_position, closest_object.position, N);
    } else {
        N[0] = closest_object.plane.normal[0];
        N[1] = closest_object.plane.normal[1];
        N[2] = closest_object.plane.normal[2];
    }
    normalize(N);


    Light* current_light;
    Object* current_object;
    double new_t = 0;
    for (int j=0; lights[j] != NULL; j++) {
        current_light = lights[j];

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
        for (int k=0; objects[k] != NULL; k++) {
            current_object = objects[k];
            // skip checking for intersection with the object already being looked at
            if (equals(*current_object, closest_object))
                continue;

            double* offset = malloc(sizeof(double)*3);
            v3_scale(obj_to_light, EPSILON, offset);
            v3_add(offset, pixel_position, offset);
            // find new intersection between light and each object looking for one that is closer to the light
            // and casts a shadow on the closest object to the camera at this pixel
            new_t=shoot(obj_to_light,offset,*current_object);



            if (new_t > 0 && new_t <= dl) {
                in_shadow = 1;
                break;
            }
        }


        if (in_shadow == 0) {
            double R[3];
			reflect(light_to_obj,N,R);
            normalize(R);




            double* light_direction = current_light->direction;
            normalize(light_direction);


            double diffuse_color[3];
			vector_copy(closest_object.diffuse,diffuse_color);
			diffuseReflection(N,obj_to_light,current_light->color,diffuse_color);



            double specular_color[3];
			vector_copy(closest_object.specular,specular_color);
			specularReflection(20,obj_to_light,R,N,obj_to_cam,current_light->color,specular_color);




            double rad = frad(dl, current_light->radA0, current_light->radA1, current_light->radA2);
            double ang = fang(light_direction, light_to_obj, current_light->angA0, current_light->theta);


            current_color[0] += rad * ang * (diffuse_color[0] + specular_color[0]);
            current_color[1] += rad * ang * (diffuse_color[1] + specular_color[1]);
            current_color[2] += rad * ang * (diffuse_color[2] + specular_color[2]);
        }
    }

    return current_color;
}

double* shade(Object current_object, double* rd, double* ro, int level, double ior) {
    double* color = malloc(sizeof(double)*3);
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;

    if (level == 7) {
        return color;
    }

    v3_add(color, illuminate(rd, ro, current_object), color);

    double N[3];
    if (current_object.kind == 0) {
        v3_subtract(ro, current_object.position, N);
    } else {
        N[0] = current_object.plane.normal[0];
        N[1] = current_object.plane.normal[1];
        N[2] = current_object.plane.normal[2];
    }
    normalize(N);

    if (current_object.reflectivity > 0) {
        double reflected_ray[3];
		reflect(rd,N,reflected_ray);

        normalize(reflected_ray);

        double best_t = INFINITY;
        Object* closest_object;

        for (int i=0; objects[i] != NULL; i++) {

            if (equals(current_object, *objects[i]))
                continue;

            double* offset = malloc(sizeof(double)*3);
            v3_scale(reflected_ray, EPSILON, offset);
            v3_add(offset, ro, offset);

            double t=shoot(reflected_ray,offset,*objects[i]);

            if (t > 0 && t < best_t) {
                best_t = t;
                closest_object = objects[i];
            }
        }

        if (best_t > 0 && best_t != INFINITY) {
            double* Ron = malloc(sizeof(double)*3);
            v3_scale(reflected_ray, best_t, Ron);
            v3_add(ro, Ron, Ron);

            double* reflected_color = malloc(sizeof(double)*3);
            v3_scale(shade(*closest_object, reflected_ray, Ron, level+1, current_object.ior), current_object.reflectivity, reflected_color);

            v3_add(color, reflected_color, color);
        }
    }

    if (current_object.refractivity > 0) {
        double* refracted_ray = refraction(ior, current_object.ior, rd, N);

        double* offset = malloc(sizeof(double)*3);
        v3_scale(refracted_ray, EPSILON, offset);
        v3_add(offset, ro, offset);

        double d;
        d=shoot(refracted_ray,offset,current_object);


        double* back = malloc(sizeof(double)*3);
        v3_scale(refracted_ray, d, back);
        v3_add(ro, back, back);

        double* back_normal = malloc(sizeof(double)*3);
        if (current_object.kind == 0) {
            v3_subtract(back, current_object.position, back_normal);
        } else if (current_object.kind == 1) {
            back_normal = current_object.plane.normal;
            v3_scale(back_normal, -1, back_normal);
        }
        normalize(back_normal);

        double* next_ray = refraction(current_object.ior, 1.0, refracted_ray, back_normal);

        double best_t = INFINITY;
        Object* closest_object;
        for (int i=0; objects[i] != NULL; i++) {

            if (equals(current_object, *objects[i]))
                continue;

            double* offset = malloc(sizeof(double)*3);
            v3_scale(refracted_ray, EPSILON, offset);
            v3_add(offset, ro, offset);

            double t = 0;
            t=shoot(next_ray,offset,*objects[i]);



            if (t > 0 && t < best_t) {
                best_t = t;
                closest_object = objects[i];
            }
        }

        if (best_t > 0 && best_t != INFINITY) {
            double* Ron = malloc(sizeof(double)*3);
            v3_scale(next_ray, best_t, Ron);
            v3_add(ro, Ron, Ron);

            double* refracted_color = malloc(sizeof(double)*3);
            v3_scale(shade(*closest_object, next_ray, Ron, level+1, current_object.ior), current_object.reflectivity, refracted_color);

            v3_add(color, direct_shade(*closest_object, Ron, refracted_color, next_ray, back), color);
            v3_add(color, refracted_color, color);
        }
    }


    return color;
}

double* direct_shade(Object current_object, double* pixel_position, double* light_color, double* light_direction, double* light_position) {
    double* color = malloc(sizeof(double)*3);
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;

    double* N = malloc(sizeof(double)*3);
    if (current_object.kind == 0)
        v3_subtract(pixel_position, current_object.position, N);
    else
        N = current_object.plane.normal;
    normalize(N);


    double obj_to_light[3];
    v3_subtract(light_position, pixel_position, obj_to_light);
    normalize(obj_to_light);

    double obj_to_cam[3];
    v3_subtract(camera.position, pixel_position, obj_to_cam);
    normalize(obj_to_cam);

    double R[3];
	reflect(light_direction,N,R);

    normalize(R);

    double diffuse_color[3];
	vector_copy(current_object.diffuse,diffuse_color);
	diffuseReflection(N,obj_to_light,light_color,diffuse_color);



    double specular_color[3];
	vector_copy(current_object.specular,specular_color);
	specularReflection(20,obj_to_light,R,N,obj_to_cam,light_color,specular_color);


    color[0] += (diffuse_color[0] + specular_color[0]);
    color[1] += (diffuse_color[1] + specular_color[1]);
    color[2] += (diffuse_color[2] + specular_color[2]);

    return color;
}




