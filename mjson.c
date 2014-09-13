#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

#ifndef bzero
#define bzero(a,b) memset(a,0,b)
#endif

/*
 * returns 1 if the result is null, 0 otherwise
 */
my_bool mjson_to_string(json_t * obj, char * output) {
	char * as_string;
	switch(json_typeof(obj)) {
		case JSON_OBJECT:
		case JSON_ARRAY:
			as_string = json_dumps(obj, JSON_COMPACT | JSON_PRESERVE_ORDER);
			sprintf(output, "%s", as_string);
			free(as_string);
		break;
		case JSON_STRING: sprintf(output, "%s", json_string_value(obj)); break;
		case JSON_INTEGER: sprintf(output, "%lld", json_integer_value(obj)); break;
		case JSON_REAL: sprintf(output, "%f", json_real_value(obj)); break;
		case JSON_TRUE: sprintf(output, "%d", 1); break;
		case JSON_FALSE: sprintf(output, "%d", 0); break;
		case JSON_NULL:
		default:
			return 1;
	}

	// is not null
	return 0;
}

/**
 * mjson_get - retrieve the value at a given position (array) or the element at a given key (object)
 */
my_bool mjson_get_init(UDF_INIT * initid, UDF_ARGS * args, char * message) {
	if(args->arg_count != 2) {
		sprintf(message, "mjson_get - takes exactly 2 arguments - mjson_get(<json>, <key|position>)");
		return 1;
	}
	return 0;
}

char * mjson_get(UDF_INIT * initid, UDF_ARGS * args, char * result, unsigned long * length, my_bool * is_null, my_bool * is_error) {
	// Special case for null
	if(args->args[0] == NULL) {
		*is_null = 1;
		return result;
	}
	
	if(args->arg_type[0] != STRING_RESULT) {
		fprintf(stderr, "the <json> argument must be a STRING - mjson_get(<json>, <key|position>)\n");
		*is_error = 1;
	    return result;
	}

	char * json = args->args[0];
	// hack to truncate json input to the right length
	json[args->lengths[0]] = 0;

	json_t * obj;
	json_error_t error;
	obj = json_loads(json, JSON_DISABLE_EOF_CHECK, &error);


	if(!obj) {
	    fprintf(stderr, "mjson_get - '%s' is not valid JSON - line %d: %s\n", json, error.line, error.text);
	    *is_error = 1;
	    return result;
	}

	my_bool is_object = json_is_object(obj);
	my_bool is_array = json_is_array(obj);

	if(args->args[1] != NULL) {
		const int type = args->arg_type[1];

		json_t * value;
		switch(type) {
			// must be an object
			case STRING_RESULT: {
				char * key = args->args[1];
				key[args->lengths[1]] = 0;
				if(!is_object) {
					fprintf(stderr, "mjson_get - recieved key=%s, but json=%s is an array\n", key, json);
					*is_error = 1;
				} else
					value = json_object_get(obj, key);
			}
			break;

			case INT_RESULT: {
				long long position = *((long long*) args->args[1]);
				if(!is_array) {
					fprintf(stderr, "mjson_get - recieved position=%lld, but json=%s is an object\n", position, json);
					*is_error = 1;
				} else
					value = json_array_get(obj, position);
			}
			break;

			default:
				fprintf(stderr, "mjson_get - unsupported <key|position> value");
				*is_null = 1;
		}

			
		/**
		 * - error occurred
		 * - position is out of bounds
		 * - key is not prese
		 */
		if(!value) {
			*is_null = 1;
		} else {
			*is_null = mjson_to_string(value, result);
		}
	}

	json_decref(obj);
	
	*length = (uint) strlen(result);
	return result;
}


/**
 * mjson_size - get the size of a JSON object/array
 */
my_bool mjson_size_init(UDF_INIT * initid, UDF_ARGS * args, char * message) {
	if(args->arg_count != 1) {
		sprintf(message, "mjson_size - takes exactly 1 arguments - mjson_size(<json>)");
		return 1;
	}
	return 0;
}

long long mjson_size(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *is_error) {
	long long size = -1;
	// Special case for null
	if(args->args[0] == NULL)
		return 0;
	
	if(args->arg_type[0] != STRING_RESULT) {
		fprintf(stderr, "mjson_size - the <json> argument must be a STRING - mjson_size(<json>)\n");
	    return size;
	}

	char * json = args->args[0];
	// hack to truncate json input to the right length
	json[args->lengths[0]] = 0;

	json_t * obj;
	json_error_t error;
	obj = json_loads(json, JSON_DISABLE_EOF_CHECK, &error);

	if(!obj) {
	    fprintf(stderr, "mjson_size - '%s' is not valid JSON - line %d: %s\n", json, error.line, error.text);
	    *is_error = 1;
	    return size;
	}


	if(json_is_array(obj))
		size = json_array_size(obj);

	if(json_is_object(obj))
		size = json_object_size(obj);

	return size;
}

/**
 * mjson_array_append
 */
my_bool mjson_array_append_init(UDF_INIT * initid, UDF_ARGS * args, char * message) {
	if(args->arg_count != 2) {
		sprintf(message, "mjson_array_append - takes exactly 2 arguments - mjson_array_append(<json>, <element>)");
		return 1;
	}
	return 0;
}

char * mjson_array_append(UDF_INIT * initid, UDF_ARGS * args, char * result, unsigned long * length, my_bool * is_null, my_bool * is_error) {
	if(args->args[0] == NULL) {
		* is_null = 1;
		return result;
	}	

	if(args->arg_type[0] != STRING_RESULT) {
		fprintf(stderr, "mjson_array_append - the <json> argument must be a STRING - mjson_array_append(<json>, <element>)");
	    * is_error = 1;
	    return result;
	}

	char * json = args->args[0];
	// hack to truncate json input to the right length
	json[args->lengths[0]] = 0;

	json_t * arr;
	json_error_t error;
	arr = json_loads(json, JSON_DISABLE_EOF_CHECK, &error);

	if(!arr) {
	    fprintf(stderr, "mjson_array_append - '%s' is not valid JSON - line %d: %s\n", json, error.line, error.text);
	    * is_error = 1;
	    return result;
	}

	if(!json_is_array(arr)) {
		fprintf(stderr, "mjson_array_append - '%s' is not a JSON array", json);
	    * is_error = 1;
	    return result;
	}

	if(args->args[1] != NULL) {
		const int type = args->arg_type[1];

		json_t * value;
		switch(type) {
			case STRING_RESULT:
				value = json_pack("s", args->args[1]);
				break;

			case INT_RESULT: {
					long long i = *((long long*) args->args[1]);
					value = json_integer(i);
				}
				break;

			case REAL_RESULT: {
					double f = *((double*) args->args[1]);
					value = json_real(f);
				}
				break;

			case DECIMAL_RESULT: {
					value = json_real(atof(args->args[1]));
				}
				break;
		}

			
		if(!value) {
			fprintf(stderr, "could not convert %s to a json value\n", args->args[1]);
		} else {
			if(json_array_append(arr, value) == -1)
				fprintf(stderr, "mjson_array_append - error adding element to array (json_array_append)\n");
			json_decref(value);
		}
	}

	*is_null = mjson_to_string(arr, result);
	json_decref(arr);

	*length = (uint) strlen(result);
	*is_null = 0;
	return result;
}
