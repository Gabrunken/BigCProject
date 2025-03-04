gcc ^
-w ^
-I ./include ^
-I ../../include ^
-I ../../deps/include ^
-L ../../lib ^
-L ../../build/lib/debug ^
-L ../../deps/lib ^
-o Test ^
./src/*.c ^
../../deps/obj/*.o ^
-l debug_bigc -l glfw3 -l gdi32 -l opengl32 -l cglm ^
-mwindows