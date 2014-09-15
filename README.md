Description
========

This project aims to add some of the (arguable) missing json functions to the MySQL and MariaDB engines, in the form of UDFs.

This project uses the [Jansson JSON Library](https://jansson.readthedocs.org/en/2.6/index.html).

Build/Install
========

The library is built using cmake build system.

If you are planing on installing it, you need to know before hand the <tt>plugin</tt> directory of you MySQL/MariaDB installation.
For instance, if you use Centos, your mysql <tt>plugin_dir</tt> is something like - <tt>/usr/lib64/mysql/plugin/</tt>.

You need to install the <tt>jansson</tt> library for the udf to work. Using cmake you define the install prefix and build.
In the root of the project you can issue:

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/lib64/mysql/plugin ..
make all install
```

To create the functions in your server instance, issue:
```bash
mysql -uroot < mjson.sql
```

Usage
========

Available functions:

* <tt>mjson_get(json, key|position)</tt> - get the value of the given key (object) or position (array) of the supplied json
* <tt>mjson_unset(json, key|position)</tt> - unsets the value at the given key (object) or position (array) of the supplied json
* <tt>mjson_set(json, key|position, value)</tt> - sets the value of the given key (object) or position (array) on the supplied json to the supplied value
* <tt>mjson_array_append(json, value)</tt> - append the given value to the supplied json array
* <tt>mjson_size(json)</tt> - get the number of elements in a json array or object

Advanced:

* <tt>mjson_config(parameter)</tt> - get a <tt>mjson</tt> configuration parameter
* <tt>mjson_config(parameter, value)</tt> - sets a <tt>mjson</tt> parameter

Available parameters:
* <tt>benchmark</tt>: when <tt>1</tt> will output benchmarking information to <tt>mysql.err</tt> 

Examples
========

Let's consider the simple table:

```sql
CREATE TABLE test_table(
  id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
  json_1 BLOB,
  json_2 BLOB
);
```

And lets insert some data in it:
```sql
INSERT INTO test_table(json_1, json_2)
  VALUES
    ('{"key1": [1, 2, 3], "key2": "some text"}', '["a", "b", "c"]'),
    ('{"key3": null, "key4": {"key5": "more data"}}', '[]')
    ;
```

Here are some usage examples:
```sql
-- mjson_size
MariaDB [tests]> SELECT
  mjson_size(json_1) size_1,
  json_1,
  mjson_size(json_2) size_2,
  json_2
  FROM test_table;
+--------+-----------------------------------------------+--------+-----------------+
| size_1 | json_1                                        | size_2 | json_2          |
+--------+-----------------------------------------------+--------+-----------------+
|      2 | {"key1": [1, 2, 3], "key2": "some text"}      |      3 | ["a", "b", "c"] |
|      2 | {"key3": null, "key4": {"key5": "more data"}} |      0 | []              |
+--------+-----------------------------------------------+--------+-----------------+
2 rows in set (0.00 sec)

-- mjson_set
MariaDB [tests]> SELECT
  json_1,
  mjson_set(json_1, 'key1', '{"a": 1}')
  FROM test_table;
+-----------------------------------------------+----------------------------------------------------------+
| json_1                                        | mjson_set(json_1, 'key1', '{"a": 1}')                    |
+-----------------------------------------------+----------------------------------------------------------+
| {"key1": [1, 2, 3], "key2": "some text"}      | {"key1":{"a":1},"key2":"some text"}                      |
| {"key3": null, "key4": {"key5": "more data"}} | {"key3":null,"key4":{"key5":"more data"},"key1":{"a":1}} |
+-----------------------------------------------+----------------------------------------------------------+
2 rows in set (0.00 sec)

-- mjson_set - if key doesn't exist... appends it
MariaDB [tests]> SELECT
  json_1,
  mjson_set(json_1, 'key1', '{"a": 1}')
  FROM test_table WHERE id=2;
+-----------------------------------------------+----------------------------------------------------------+
| json_1                                        | mjson_set(json_1, 'key1', '{"a": 1}')                    |
+-----------------------------------------------+----------------------------------------------------------+
| {"key3": null, "key4": {"key5": "more data"}} | {"key3":null,"key4":{"key5":"more data"},"key1":{"a":1}} |
+-----------------------------------------------+----------------------------------------------------------+

-- mjson_get
MariaDB [tests]> SELECT
  mjson_get(
    mjson_get(
      mjson_set('{}', 'root_key', '{"sub_key": 3.14}'),
      'root_key'),
    'sub_key'
  );
+-----------------------------------------------------------------------------------------------+
| mjson_get(mjson_get(mjson_set('{}', 'root_key', '{"sub_key": 3.14}'), 'root_key'), 'sub_key') |
+-----------------------------------------------------------------------------------------------+
| 3.140000                                                                                      |
+-----------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)

-- mjson_array_append
MariaDB [tests]> SELECT
      mjson_array_append(json_2, '[1,2,3]'),
      mjson_array_append(json_2, 3.14)
      FROM test_table WHERE id=1;
+---------------------------------------+----------------------------------+
| mjson_array_append(json_2, '[1,2,3]') | mjson_array_append(json_2, 3.14) |
+---------------------------------------+----------------------------------+
| ["a","b","c",[1,2,3]]                 | ["a","b","c",3.1400000000000001] |
+---------------------------------------+----------------------------------+
1 row in set (0.00 sec)

-- mjson_unset
MariaDB [(none)]> SELECT mjson_unset('{"a": 1, "b": 2}', 'a');
+--------------------------------------+
| mjson_unset('{"a": 1, "b": 2}', 'a') |
+--------------------------------------+
| {"b":2}                              |
+--------------------------------------+
1 row in set (0.00 sec)

```

