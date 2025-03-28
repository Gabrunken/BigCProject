gcc ^
-w ^
-D DEBUG ^
-I ./include ^
-I ../../../include ^
-I ../../../deps/include ^
-L ../../../build/lib/debug ^
-L ../../../deps/lib ^
-o Client_debug ^
./src/*.c ^
../../../deps/obj/*.o ^
-l debug_bigc -l glfw3 -l gdi32 -l opengl32 -l cglm -lws2_32