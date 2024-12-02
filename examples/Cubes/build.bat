gcc ^
-w ^ ^
-I ./include ^
-I ../../include ^
-I ../../deps/include ^
-L ../../lib ^
-L ../../build/lib/ ^
-L ../../deps/lib ^
-o Cubes ^
./src/*.c ^
../../deps/obj/*.o ^
-l bigc -l glfw3 -l gdi32 -l opengl32 -l cglm ^
-mwindows