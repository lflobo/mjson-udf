all: test
	gcc -ljansson -shared -o mjson.so mjson.c

test:
	gcc -ljansson -o jansson_test jansson_test.c

clean:
	rm -rf *.so jansson_test

