#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

#include "mjson_util.h"

/**
 * mjson_get - get the value of JSON Object/Array at specified key/position
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

	my_bool ok;
	char * json = mjarg(args, 0);
	json_t * obj = mjloads(json, &ok);

	if(!ok) {
	    *is_error = 1;
	} else {
		my_bool is_object = json_is_object(obj);
		my_bool is_array = json_is_array(obj);

		if(args->args[1] != NULL) {
			const int type = args->arg_type[1];

			json_t * value;
			switch(type) {
				// must be an object
				case STRING_RESULT: {
					char * key = mjarg(args, 1);
					if(!is_object) {
						fprintf(stderr, "mjson_get - recieved key=%s, but json=%s is an array\n", key, json);
						*is_error = 1;
					} else
						value = json_object_get(obj, key);
					free(key);
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
				*is_null = mjstring(value, result);
			}
		}

		json_decref(obj);
	}

	free(json);
	*length = (uint) strlen(result);
	return result;
}

/**
 * mjson_set - set the value at a given position (array) or the element at a given key (object)
 */
my_bool mjson_set_init(UDF_INIT * initid, UDF_ARGS * args, char * message) {
	if(args->arg_count != 3) {
		sprintf(message, "mjson_set - takes exactly 3 arguments - mjson_get(<json>, <key|position>, <value>)");
		return 1;
	}
	return 0;
}

char * mjson_set(UDF_INIT * initid, UDF_ARGS * args, char * result, unsigned long * length, my_bool * is_null, my_bool * is_error) {
	// Special case for null
	if(args->args[0] == NULL) {
		*is_null = 1;
		return result;
	}
	
	if(args->arg_type[0] != STRING_RESULT) {
		fprintf(stderr, "the <json> argument must be a STRING - mjson_set(<json>, <key|position>, <value>)\n");
		*is_error = 1;
	    return result;
	}

	my_bool ok;
	char * json = mjarg(args, 0);
	json_t * obj = mjloads(json, &ok);

	if(!ok) {
	    *is_error = 1;
	} else {
		my_bool is_object = json_is_object(obj);
		my_bool is_array = json_is_array(obj);

		if(args->args[1] != NULL) {
			const int type = args->arg_type[1];
			switch(type) {
				// must be an object
				case STRING_RESULT: {
					char * key = mjarg(args, 1);
					if(!is_object) {
						fprintf(stderr, "mjson_set - recieved key=%s, but json=%s is an array\n", key, json);
						*is_error = 1;
					} else {
						json_t * json_value = mjvalue(args, 2, 0);
						json_object_set(obj, key, json_value);
						json_decref(json_value);
					}
					free(key);
				}
				break;

				case INT_RESULT: {
					long long position = *((long long*) args->args[1]);
					char * value = mjarg(args, 2);
					if(!is_array) {
						fprintf(stderr, "mjson_set - recieved position=%lld, but json=%s is an object\n", position, json);
						*is_error = 1;
					} else {
						json_t * json_value = mjvalue(args, 2, 0);
						json_array_set(obj, position, json_value);
						json_decref(json_value);
					}
					free(value);
				}
				break;

				default:
					fprintf(stderr, "mjson_set - unsupported <key|position> value");
					*is_null = 1;
			}

				
			*is_null = mjstring(obj, result);
		}
		json_decref(obj);
	}

	free(json);
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


	my_bool ok;
	char * json = mjarg(args, 0);
	json_t * obj = mjloads(json, &ok);
	free(json);

	if(!ok) {
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

	my_bool ok;
	char * json = mjarg(args, 0);
	json_t * arr = mjloads(json, &ok);

	if(!ok) {
	    * is_error = 1;
	} else {
		if(!json_is_array(arr)) {
			fprintf(stderr, "mjson_array_append - '%s' is not a JSON array", json);
		    * is_error = 1;
		} else {
			if(args->args[1] != NULL) {
				json_t * value = mjvalue(args, 1, 0);
				if(!value) {
					fprintf(stderr, "could not convert %s to a json value\n", args->args[1]);
				} else {
					if(json_array_append(arr, value) == -1)
						fprintf(stderr, "mjson_array_append - error adding element to array (json_array_append)\n");
					json_decref(value);
				}
			}
		}
		*is_null = mjstring(arr, result);
		json_decref(arr);
	}

	*length = (uint) strlen(result);
	return result;
}

/**
 * mjson_unset - Removes the element at specified key|position from the JSON Object/Array
 */
my_bool mjson_unset_init(UDF_INIT * initid, UDF_ARGS * args, char * message) {
	if(args->arg_count != 2) {
		sprintf(message, "mjson_array_remove - takes exactly 2 arguments - mjson_array_remove(<json>, <key|position>)");
		return 1;
	}
	return 0;
}

char * mjson_unset(UDF_INIT * initid, UDF_ARGS * args, char * result, unsigned long * length, my_bool * is_null, my_bool * is_error) {
	if(args->args[0] == NULL) {
		* is_null = 1;
		return result;
	}

	if(args->arg_type[0] != STRING_RESULT) {
		fprintf(stderr, "mjson_array_append - the <json> argument must be a STRING - mjson_array_append(<json>, <element>)");
	    * is_error = 1;
	    return result;
	}

	my_bool ok;
	char * json = mjarg(args, 0);
	json_t * obj = mjloads(json, &ok);

	if(!ok) {
		    *is_error = 1;
		} else {
			my_bool is_object = json_is_object(obj);
			my_bool is_array = json_is_array(obj);

			if(args->args[1] != NULL) {
				const int type = args->arg_type[1];
				switch(type) {
					case STRING_RESULT: {
						char * key = mjarg(args, 1);
						if(!is_object) {
							fprintf(stderr, "mjson_array_remove - recieved key=%s, but json=%s is an array\n", key, json);
							*is_error = 1;
						} else {
							*is_error = json_object_del(obj, key);
						}
						free(key);
					}
					break;

					case INT_RESULT: {
						long long position = *((long long*) args->args[1]);
						if(!is_array) {
							fprintf(stderr, "mjson_array_remove - recieved position=%lld, but json=%s is an object\n", position, json);
							*is_error = 1;
						} else {
							*is_error = json_array_remove(obj, position);
						}
					}
					break;

					default:
						fprintf(stderr, "mjson_array_remove - unsupported <key|position> value");
						*is_null = 1;
				}

				*is_null = mjstring(obj, result);
			}
			json_decref(obj);
		}

	*length = (uint) strlen(result);
	return result;
}
