#include "mjson_util.h"
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <mysql/mysql.h>
#include <jansson.h>

/*
 * returns 1 if the result is null, 0 otherwise
 */
my_bool mjstring(json_t * obj, char * output) {
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

// typedef struct st_udf_init
// {
//   my_bool maybe_null;          /* 1 if function can return NULL */
//   unsigned int decimals;       /* for real functions */
//   unsigned long max_length;    /* For string functions */
//   char *ptr;                   /* free pointer for function data */
//   my_bool const_item;          /* 1 if function always returns the same value */
//   void *extension;
// } UDF_INIT;

// typedef struct st_udf_args
// {
//   unsigned int arg_count;		/* Number of arguments */
//   enum Item_result *arg_type;		/* Pointer to item_results */
//   char **args;				/* Pointer to argument */
//   unsigned long *lengths;		/* Length of string arguments */
//   char *maybe_null;			/* Set to 1 for all maybe_null args */
//   char **attributes;                    /* Pointer to attribute name */
//   unsigned long *attribute_lengths;     /* Length of attribute arguments */
//   void *extension;
// } UDF_ARGS;

void mjdebug(const char * fname, UDF_INIT *initid, UDF_ARGS *args) {
	fprintf(stderr,
		"-- fname=%s -- \n"
		"UDF_INIT {\n"
		"  maybe_null=%d,\n"
		"  decimals=%d,\n"
		"  max_length=%ld,\n"
		"  const_item=%d\n"
		"}\n"
		"UDF_ARGS {\n"
		"  arg_count=%d,\n"
		"  args=[\n",
		fname,
		initid->maybe_null,
		initid->decimals,
		initid->max_length,
		initid->const_item,
		args->arg_count
	);

	int i;
	for(i=0; i<args->arg_count; i++) {
		// args->args[i][args->lengths[i]] = 0;
		fprintf(stderr,
			"    {\n"
			"      length=%ld,\n"
			"      attribute='%s',\n"
			"      arg='%s',\n"
			"    }\n",
			args->lengths[i],
			args->attributes[i],
			args->args[i]
		);
	}

	fprintf(stderr,
		"  ]\n"
		"}\n"
	);
}

json_t * mjloads(const char * json, my_bool * ok) {
	json_t * obj;
	json_error_t error;
	obj = json_loads(json, 0, &error);
	if(!obj) {
		fprintf(stderr, "'%s' is not valid JSON - line %d: %s\n", json, error.line, error.text);
		*ok = 0;
	} else
		*ok = 1;
	return obj;
}

char * mjarg(UDF_ARGS * args, int index) {
	int l;
    char *s;
    char *colval = NULL;
    if( (l=args->lengths[index] ) && (s=args->args[index]))
        colval = mjstrtrunc(s, l);
    return colval ;
}

long long mjarg_int(UDF_ARGS * args, int index) {
	return *((long long*) args->args[index]);
}

double mjarg_real(UDF_ARGS * args, int index) {
	return *((double*) args->args[index]);
}

double mjarg_decimal(UDF_ARGS * args, int index) {
	return atof(args->args[index]);
}

char * mjstrtrunc(char * string, size_t size) {
    char * substr = malloc( size + 1 ) ;
    if(!substr) {
        fprintf(stderr, "Not enough memory: %zu\n" , size);
        return NULL ;
    }
    memcpy(substr, string, size);
    substr[ size ] = '\0'; 
    return substr ;
}

json_t * mjvalue(UDF_ARGS * args, int index, my_bool primitive_only) {
	const int type = args->arg_type[index];
	if(type == STRING_RESULT) {
		if(!primitive_only) {
			json_t * obj = json_loads(args->args[index], 0, NULL);
			// it's an object?
			if(json_is_array(obj) || json_is_object(obj))
				return obj;
		}
		// return as string
		return json_pack("s", args->args[index]);
	}

	if(type == INT_RESULT)
		return json_integer(mjarg_int(args, index));
	if(type == REAL_RESULT)
		return json_real(mjarg_real(args, index));
	if(type == DECIMAL_RESULT)
		return json_real(mjarg_decimal(args, index));
	return NULL;
}

double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}
