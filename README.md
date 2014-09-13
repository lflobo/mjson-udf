=== Description ===

This project aims to add some of the (arguable) missing json functions to the MySQL and MariaDB engines, in the form of UDFs.
This project uses the [https://jansson.readthedocs.org/en/2.6/index.html jansson library].

=== Install ===

```bash
make
cp mjson.so <mysql plugin path>
mysql -uroot < mjson.sql
```

=== Usage ===

Available functions:

* mjson_get(json, index|position) - get the value of the given key (object) or position (array) of the supplied json
* mjson_size(json) - get the number of elements in a json array or object
* mjson_array_append(json, value) - append the given value to the supplied json array