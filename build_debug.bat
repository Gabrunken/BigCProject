cd build/obj/debug
gcc -w -D DEBUG -c -I../../../include -I../../../deps/include ../../../src/*.c
cd ../../lib/debug
ar rcs libdebug_bigc.a ../../obj/debug/*.o