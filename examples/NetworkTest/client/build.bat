gcc ^
-w ^
-I ./include ^
-I ../../../include ^
-I ../../../deps/include ^
-L ../../../build/lib ^
-L ../../../deps/lib ^
-o Client ^
./src/*.c ^
../../../deps/obj/*.o ^
-l bigc -l glfw3 -l gdi32 -l opengl32 -l cglm -lws2_32 ^
-mwindows