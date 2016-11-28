void parse_camera(FILE*);
void parse_sphere(FILE*, Object*);
void parse_plane(FILE*, Object*);
void parse_light(FILE*, Light*);
void skip_ws(FILE*);
void expect_c(FILE*, int);
int next_c(FILE*);
char* next_string(FILE*);
double next_number(FILE*);
double* next_vector(FILE*);
double clamp(double);
void output_picture(FILE*);
int line = 1;

void read_scene(FILE* json) {
	int c;
	skip_ws(json);
	expect_c(json, '[');
	skip_ws(json);
	int object_index = 0;
	int light_index = 0;
	while (1) {
		c = next_c(json);


		if (c == ']') {
			fprintf(stderr, "Error: This scene is empty.\n");
			fclose(json);
			objects[object_index] = NULL;
			return;
		}


		if (c == '{') {
			skip_ws(json);


			char* key = next_string(json);
			if (strcmp(key, "type") != 0) {
				fprintf(stderr, "Error: Expected 'type' key. (Line %d)\n", line);
				exit(1);
			}

			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);


			char* value = next_string(json);
			if (strcmp(value, "camera") == 0) {
				parse_camera(json);
			} else if (strcmp(value, "sphere") == 0) {

				objects[object_index] = malloc(sizeof(Object));
				parse_sphere(json, objects[object_index]);
				object_index++;
			} else if (strcmp(value, "plane") == 0) {

				objects[object_index] = malloc(sizeof(Object));
				parse_plane(json, objects[object_index]);
				object_index++;
			} else if (strcmp(value, "light") == 0) {
				lights[light_index] = malloc(sizeof(Light));
				parse_light(json, lights[light_index]);
				light_index++;
			} else {
				fprintf(stderr, "Error: Unknown type '%s'. (Line %d)\n", value, line);
				exit(1);
			}


			skip_ws(json);
			c = next_c(json);
			if (c == ',') {
				skip_ws(json);
			} else if (c == ']') {
				objects[object_index] = NULL;
				lights[light_index] = NULL;
				fclose(json);
				return;
			} else {
				fprintf(stderr, "Error: Expecting ',' or ']'. (Line %d)\n", line);
				exit(1);
			}


			if ((object_index + light_index) == 129) {
				objects[object_index] = NULL;
				lights[light_index] = NULL;
				fclose(json);
				fprintf(stderr, "Error: Too many objects in file.\n");
				return;
			}
		} else {
			fprintf(stderr, "Error: Expecting '{'. (Line %d)\n", line);
			exit(1);
		}
	}
}
void parse_camera(FILE* json) {
	int c;
	skip_ws(json);

	camera.position[0] = 0;
	camera.position[1] = 0;
	camera.position[2] = 0;

	camera.width = 1;
	camera.height = 1;


	while (1) {
		c = next_c(json);
		if (c == '}') {
			break;
		} else if (c == ',') {
			skip_ws(json);
			char* key = next_string(json);

			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);

			double value = next_number(json);

			if (strcmp(key, "width") == 0) {
				camera.width = value;
			} else if (strcmp(key, "height") == 0) {
				camera.height = value;
			} else {
				fprintf(stderr, "Error: Unknown property '%s' for 'camera'. (Line %d)\n", key, line);
				exit(1);
			}
		}
	}
}


void parse_sphere(FILE* json, Object* object) {
	int c;


	int hasradius = 0;
	int hascolor = 0;
	int hasposition = 0;
	int hasspecular = 0;


	object->kind = 0;

	object->reflectivity = 0;
	object->refractivity = 0;
	object->ior = 1;

	skip_ws(json);

	// check fields for this sphere
	while(1) {
		c = next_c(json);
		if (c == '}') {
			break;
		} else if (c == ',') {
			skip_ws(json);
			char* key = next_string(json);

			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);

			// set values for this sphere depending on what key was read and sets its 'boolean' to reflect the found field
			if (strcmp(key, "radius") == 0) {
				object->sphere.radius = next_number(json);
				hasradius = 1;
			} else if (strcmp(key, "reflectivity") == 0) {
				object->reflectivity = clamp(next_number(json));
			} else if (strcmp(key, "refractivity") == 0) {
				object->refractivity = clamp(next_number(json));
			} else if (strcmp(key, "ior") == 0) {
				object->ior = next_number(json);
			} else if (strcmp(key, "diffuse_color") == 0) {
				double* value = next_vector(json);
				object->diffuse[0] = value[0];
				object->diffuse[1] = value[1];
				object->diffuse[2] = value[2];
				hascolor = 1;
			} else if (strcmp(key, "specular_color") == 0) {
				double* value = next_vector(json);
				object->specular[0] = value[0];
				object->specular[1] = value[1];
				object->specular[2] = value[2];
				hasspecular=1;
			} else if (strcmp(key, "position") == 0) {
				double* value = next_vector(json);
				object->position[0] = value[0];
				object->position[1] = value[1];
				object->position[2] = value[2];
				hasposition = 1;
			} else {
				fprintf(stderr, "Error: Unknown property '%s' for 'sphere'. (Line %d)\n", key, line);
				exit(1);
			}
		}
	}

	// check for missing fields
	if (!hasradius) {
		fprintf(stderr, "Error: Sphere missing 'radius' field. (Line %d)\n", line);
		exit(1);
	}

	if (!hascolor) {
		fprintf(stderr, "Error: Sphere missing 'color' field. (Line %d)\n", line);
		exit(1);
	}

	if (!hasposition) {
		fprintf(stderr, "Error: Sphere missing 'position' field. (Line %d)\n", line);
		exit(1);
	}
	if (!hasspecular){
		fprintf(stderr,"Error: Sphere missing 'specular field (Line %d)\n'",line);
		exit(1);
	}
}

// gets plane information and stores it into an object
void parse_plane(FILE* json, Object* object) {
	int c;

	// used to check that all fields for a plane are present
	int hasnormal = 0;
	int hasdiffuse = 0;
	int hasposition = 0;
	int hasspecular =0;

	// set object kind to plane
	object->kind = 1;
	object->reflectivity = 0;
	object->refractivity = 0;
	object->ior = 1;


	skip_ws(json);

	// check fields for this plane
	while(1) {
		c = next_c(json);
		if (c == '}') {
			break;
		} else if (c == ',') {
			skip_ws(json);
			char* key = next_string(json);

			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);


			if (strcmp(key, "normal") == 0) {
				double* value = next_vector(json);
				object->plane.normal[0] = value[0];
				object->plane.normal[1] = value[1];
				object->plane.normal[2] = value[2];
				hasnormal = 1;
			} else if (strcmp(key, "diffuse_color") == 0) {
				double* value = next_vector(json);
				object->diffuse[0] = value[0];
				object->diffuse[1] = value[1];
				object->diffuse[2] = value[2];
				hasdiffuse = 1;
			} else if (strcmp(key, "specular_color") == 0) {
				double* value = next_vector(json);
				object->specular[0] = value[0];
				object->specular[1] = value[1];
				object->specular[2] = value[2];
				hasspecular=1;
			} else if (strcmp(key, "position") == 0) {
				double* value = next_vector(json);
				object->position[0] = value[0];
				object->position[1] = value[1];
				object->position[2] = value[2];
				hasposition = 1;}
			else if (strcmp(key, "reflectivity") == 0) {
				object->reflectivity = clamp(next_number(json));
			} else if (strcmp(key, "refractivity") == 0) {
				object->refractivity = clamp(next_number(json));
			} else if (strcmp(key, "ior") == 0) {
				object->ior = next_number(json);
			} else {
				fprintf(stderr, "Error: Unknown property '%s' for 'sphere'. (Line %d)\n", key, line);
				exit(1);
			}
		}
	}


	if (!hasnormal) {
		fprintf(stderr, "Error: Plane missing 'normal' field. (Line %d)\n", line);
		exit(1);
	}

	if (!hasdiffuse) {
		fprintf(stderr, "Error: Plane missing 'diffuse' field. (Line %d)\n", line);
		exit(1);
	}

	if (!hasposition) {
		fprintf(stderr, "Error: Plane missing 'position' field. (Line %d)\n", line);
		exit(1);
	}
	if (!hasspecular){
		fprintf(stderr,"Error: Plane missing 'specular field (Line %d)\n'",line);
		exit(1);
	}
}


void parse_light(FILE* json, Light* light) {
	int c;


	int hascolor = 0;
	int hasposition = 0;
	int radial2 = 0;
	int radial1 = 0;
	int radial0 = 0;


	light->theta = 0;

	skip_ws(json);

	while (1) {
		c = next_c(json);
		if (c == '}') {
			break;
		} else if (c == ',') {
			skip_ws(json);
			char *key = next_string(json);

			skip_ws(json);
			expect_c(json, ':');
			skip_ws(json);


			if (strcmp(key, "color") == 0) {
				double *value = next_vector(json);
				light->color[0] = value[0];
				light->color[1] = value[1];
				light->color[2] = value[2];
				hascolor = 1;
			} else if (strcmp(key, "position") == 0) {
				double *value = next_vector(json);
				light->position[0] = value[0];
				light->position[1] = value[1];
				light->position[2] = value[2];
				hasposition = 1;
			} else if (strcmp(key, "radial-a2") == 0) {
				light->radA2 = next_number(json);
				radial2 = 1;
			} else if (strcmp(key, "radial-a1") == 0) {
				light->radA1 = next_number(json);
				radial1 = 1;
			} else if (strcmp(key, "radial-a0") == 0) {
				light->radA0 = next_number(json);
				radial0 = 1;
			} else if (strcmp(key, "theta") == 0) {
				light->theta = next_number(json);
			} else if (strcmp(key, "direction") == 0) {
				double *value = next_vector(json);
				light->direction[0] = value[0];
				light->direction[1] = value[1];
				light->direction[2] = value[2];
			} else if (strcmp(key, "angular-a0") == 0) {
				light->angA0 = next_number(json);
			} else {
				fprintf(stderr, "Error: Unknown property '%s' for 'light'. (Line %d)\n", key, line);
				exit(1);
			}
		}
	}


	if (!hascolor) {
		fprintf(stderr, "Error: Light missing 'color' field. (Line %d)\n", line);
		exit(1);
	}

	if (!hasposition) {
		fprintf(stderr, "Error: Light missing 'position' field. (Line %d)\n", line);
		exit(1);
	}

	if (!radial2 || !radial1 || !radial0) {
		fprintf(stderr, "Error: Light missing one or more radial field(s). (Line %d)\n", line);
		exit(1);
	}
}


void skip_ws(FILE* json) {
	int c = next_c(json);

	while(isspace(c))
		c = next_c(json);

	ungetc(c, json);
}


void expect_c(FILE* json, int d) {
	int c = next_c(json);

	if (c == d)
		return;


	fprintf(stderr, "Error: Expected '%c'. (Line %d)\n", d, line);
	exit(1);
}

int next_c(FILE* json) {
	int c = fgetc(json);


	if (c == '\n')
		line++;

	if (c == EOF) {
		fprintf(stderr, "Error: Unexpected end of file. (Line %d)\n", line);
		exit(1);
	}

	return c;
}


char* next_string(FILE* json) {
	char buffer[129];
	int c = next_c(json);


	if (c != '"') {
		fprintf(stderr, "Error: Expected string. (Line %d)\n", line);
		exit(1);
	}

	c = next_c(json);

	int i = 0;

	while (c != '"') {

		if (i >= 128) {
			fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported. (Line %d)\n", line);
			exit(1);
		}


		if (c == '\\') {
			fprintf(stderr, "Error: Strings with escape codes are not supported. (Line %d)\n", line);
			exit(1);
		}


		if (c < 32 || c > 126) {
			fprintf(stderr, "Error: Strings may only contain ASCII characters. (Line %d)\n", line);
			exit(1);
		}


		buffer[i] = c;
		i++;
		c = next_c(json);
	}


	buffer[i] = 0;
	return strdup(buffer);
}

double next_number(FILE* json) {
	double value;

	if (fscanf(json, "%lf", &value) != 1) {
		fprintf(stderr, "Error: Number value not found. (Line %d)\n", line);
		exit(1);
	}

	return value;
}


double* next_vector(FILE* json) {
	double* v = malloc(3*sizeof(double));


	expect_c(json, '[');
	skip_ws(json);
	v[0] = next_number(json);
	skip_ws(json);


	expect_c(json, ',');
	skip_ws(json);
	v[1] = next_number(json);
	skip_ws(json);


	expect_c(json, ',');
	skip_ws(json);
	v[2] = next_number(json);
	skip_ws(json);


	expect_c(json, ']');
	return v;
}