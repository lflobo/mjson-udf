Description
========

This project aims to add some of the (arguable) missing json functions to the MySQL and MariaDB engines, in the form of UDFs.

This project uses the Jansson JSON Library (https://jansson.readthedocs.org/en/2.6/index.html).

Build/Install
========

The library is built using cmake build system.

If you are planing on installing it, you need to know before hand the <tt>plugin</tt> directory of you MySQL/MariaDB installation.
For instance, if you use Centos, your mysql <tt>plugin_dir</tt> is something like - <tt>/usr/lib64/mysql/plugin/</tt>.

You need to instal the <tt>jansson</tt> library for the udf to work. Using cmake you define the install prefix and build.
In the root of the project you can issue:

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/lib64/mysql/plugin ..
make all install
```

Usage
========

Available functions:

* <tt>mjson_get(json, index|position)</tt> - get the value of the given key (object) or position (array) of the supplied json
* mjson_size(json) - get the number of elements in a json array or object
* mjson_array_append(json, value) - append the given value to the supplied json array
