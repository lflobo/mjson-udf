DROP FUNCTION IF EXISTS mjson_array_append;
DROP FUNCTION IF EXISTS mjson_size;
DROP FUNCTION IF EXISTS mjson_get;

CREATE FUNCTION mjson_array_append RETURNS STRING SONAME 'mjson.so';
CREATE FUNCTION mjson_size RETURNS INTEGER SONAME 'mjson.so';
CREATE FUNCTION mjson_get RETURNS STRING SONAME 'mjson.so';