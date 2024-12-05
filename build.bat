cd build/obj
gcc -w -c -I../../include -I../../deps/include ../../src/*.c
cd ../lib
ar rcs libbigc.a ../obj/*.o
cd ../../..