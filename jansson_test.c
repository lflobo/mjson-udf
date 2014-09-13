#include <string.h>
#include <jansson.h>

int main(int argc, char ** argv) {
	json_t * object;
	json_error_t error;
	char * json = argv[1];
	
	object = json_loads(json, 0, &error);
	if(!object) {
	    fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
	    return 1;
	}

	// [1,2,3,4,10,10,10]
	char * as_string = json_dumps(object, JSON_COMPACT | JSON_PRESERVE_ORDER);
	printf("%s\n", as_string);
	free(as_string);

	json_decref(object);
}