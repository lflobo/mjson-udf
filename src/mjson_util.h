#ifndef MJSON_UTIL
#define MJSON_UTIL

#include <jansson.h>
#include <mysql/mysql.h>

my_bool mjstring(json_t * obj, char * output);

void mjdebug(const char * fname, UDF_INIT *initid, UDF_ARGS *args);

json_t * mjloads(const char * json, my_bool * ok);

char * mjarg(UDF_ARGS * args, int index);

char * mjstrtrunc(char * string, size_t size);

json_t * mjvalue(UDF_ARGS * args, int index, my_bool primitive_only);

#endif