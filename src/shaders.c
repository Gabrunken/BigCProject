//Include in .c file to avoid circular inclusion or something named like that (including more than one time).
#include <shaders.h>
#include <gl3w.h>
#include <stdio.h>
#include <bigc.h>
#include <string.h>

unsigned int bigc_currentlyBoundShader;

bigc_ShaderProgram bigc_shaders_LoadFromDisk(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	bigc_ShaderProgram shaderProgram = {};

	//Open vertex shader file
	FILE* filePointer = fopen(vertexShaderPath, "rb");
	
	if(filePointer == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to load a file");
		printf("\tPath: %s\n", vertexShaderPath);
		#endif

		return BIGC_BAD_SHADER_PROGRAM;
	}

	char* string;
	unsigned int characters;
	
	//Get file size
	fseek(filePointer, 0, SEEK_END);
	characters = ftell(filePointer);

	//Positions the cursor to be at the start of the file
	fseek(filePointer, 0, SEEK_SET);
	//Allocate memory for the string on the heap, because its size is not constant and is determined at runtime.
	string = (char*)calloc(characters, sizeof(char) * characters);

	fread(string, 1, characters, filePointer); //Put every character from the file to the string
	fclose(filePointer);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const GLchar* const *)&string, NULL); free(string);
	glCompileShader(vertexShader);

	//Check for compilation errors
	{
		int compileResult;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileResult);
		if (compileResult == FALSE)
		{
			#ifdef DEBUG
			int errorLength;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &errorLength);

			char* message = (char*)malloc(errorLength*sizeof(char));
			glGetShaderInfoLog(vertexShader, errorLength, &errorLength, message);

			BIGC_LOG_ERROR("failed to compile vertex shader");
			printf("\tPath: %s\n", vertexShaderPath);
			printf("\tError details: %s\n", message);

			free(message);
			#endif
			glDeleteShader(vertexShader);

			return BIGC_BAD_SHADER_PROGRAM;
		}
	}

	//Open fragment shader file
	filePointer = fopen(fragmentShaderPath, "rb");
	
	if(filePointer == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to load a file");
		printf("\tPath: %s\n", fragmentShaderPath);
		#endif

		return BIGC_BAD_SHADER_PROGRAM;
	}

	//Get file size
	fseek(filePointer, 0, SEEK_END);
	characters = ftell(filePointer);
	
	//Allocate memory for the string
	//Use calloc (to allocate AND intialize everything to 0) because apparently some gibberish was left behind.
	//Probably because of the previous allocation for the vertex shader
	string = (char*)calloc(characters, sizeof(char)*characters);

	fseek(filePointer, 0, SEEK_SET); //Put the cursor at the beginning of the file
	fread(string, 1, characters, filePointer); //Put every character from the file to the string
	fclose(filePointer);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const GLchar* const *)&string, NULL); free(string);
	glCompileShader(fragmentShader);

	//Check for compilation errors
	{
		int compileResult;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileResult);
		if (compileResult == FALSE)
		{
			#ifdef DEBUG
			int errorLength;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &errorLength);

			char* message = (char*)malloc(errorLength*sizeof(char));
			glGetShaderInfoLog(fragmentShader, errorLength, &errorLength, message);

			BIGC_LOG_ERROR("failed to compile fragment shader");
			printf("\tPath: %s\n", fragmentShaderPath);
			printf("\tError details: %s\n", message);

			free(message);
			#endif

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
			
			return BIGC_BAD_SHADER_PROGRAM;
		}
	}

	//Here both shaders are compiled and ready to be linked to a program
	shaderProgram.handle = glCreateProgram();
	glAttachShader(shaderProgram.handle, vertexShader);
	glAttachShader(shaderProgram.handle, fragmentShader);
	glLinkProgram(shaderProgram.handle);
	glValidateProgram(shaderProgram.handle);
	//In fact this does not delete them, just marks them for deletion.
	//As soon as they are not attached to any shader program anymore, they'll get deleted.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//now find every uniform inside the shaders
	GLint everyUniformStored; //Including the ones inside the UBOs
	glGetProgramiv(shaderProgram.handle, GL_ACTIVE_UNIFORMS, &everyUniformStored); //first of all get the number of them

	for(int i = 0; i < everyUniformStored; i++)
	{
        GLint blockIndex;
        glGetActiveUniformsiv(shaderProgram.handle, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex); //This will help me know if it is inside a uniform block (UBO)

        if(blockIndex != -1) //It is a uniform inside a uniform block (UBO)
        {
        	continue;
        }

        int nameLength;
        int arraySize; //if the uniform is an array, the size is the array elements defined in glsl, if it is not an array, size is 1          
        
        glGetActiveUniform(
        	shaderProgram.handle,
        	i,
        	sizeof(shaderProgram.uniformNames[shaderProgram.uniformsStored]),
        	&nameLength,
        	&arraySize,
        	(GLenum*)&shaderProgram.uniformDataTypes[shaderProgram.uniformsStored],
        	shaderProgram.uniformNames[shaderProgram.uniformsStored]);

        shaderProgram.uniformLocations[shaderProgram.uniformsStored] = glGetUniformLocation(shaderProgram.handle, shaderProgram.uniformNames[shaderProgram.uniformsStored]); //also retrieve location
        
		shaderProgram.uniformsStored++;

		if(shaderProgram.uniformLocations[shaderProgram.uniformsStored] == -1)
		{
			#ifdef DEBUG
			BIGC_LOG_ERROR("failed to get a uniform location");
			printf("\tName: %s\n", shaderProgram.uniformNames[shaderProgram.uniformsStored]);
        	printf("\tData type: %d\n", shaderProgram.uniformDataTypes[shaderProgram.uniformsStored]);
			#endif
		}
    }

	return shaderProgram;
}

void bigc_shaders_SetIntOrBool(const bigc_ShaderProgram* shader, const char* variableName, int value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set an int or bool uniform in an invalid shader");
		printf("\tName: %s\n", variableName);
		#endif
		return;
	}

	//find the uniform that matches its name with "variableName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], variableName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniform1i(shader->uniformLocations[uniformIndex], value);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type int or bool not found");
	printf("\tName: %s\n", variableName);
	#endif
}

void bigc_shaders_SetFloat(const bigc_ShaderProgram* shader, const char* variableName, float value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set a float uniform in an invalid shader");
		printf("\tName: %s\n", variableName);
		#endif
		return;
	}

	//find the uniform that matches its name with "variableName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], variableName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniform1f(shader->uniformLocations[uniformIndex], value);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type float not found");
	printf("\tName: %s\n", variableName);
	#endif
}

void bigc_shaders_SetVec2(const bigc_ShaderProgram* shader, const char* variableName, vec2 value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set an vec2 uniform in an invalid shader");
		printf("\tName: %s\n", variableName);
		#endif
		return;
	}

	//find the uniform that matches its name with "variableName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], variableName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniform2fv(shader->uniformLocations[uniformIndex], 1, value);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type vec2 not found");
	printf("\tName: %s\n", variableName);
	#endif
}

void bigc_shaders_SetVec3(const bigc_ShaderProgram* shader, const char* variableName, vec3 value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set an vec3 uniform in an invalid shader");
		printf("\tName: %s\n", variableName);
		#endif
		return;
	}

	//find the uniform that matches its name with "variableName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], variableName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniform3fv(shader->uniformLocations[uniformIndex], 1, value);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type vec3 not found");
	printf("\tName: %s\n", variableName);
	#endif
}

void bigc_shaders_SetVec4(const bigc_ShaderProgram* shader, const char* variableName, vec4 value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set an vec4 uniform in an invalid shader");
		printf("\tName: %s\n", variableName);
		#endif
		return;
	}

	//find the uniform that matches its name with "variableName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], variableName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniform4fv(shader->uniformLocations[uniformIndex], 1, value);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type vec4 not found");
	printf("\tName: %s\n", variableName);
	#endif
}

void bigc_shaders_SetMat4(const bigc_ShaderProgram* shader, const char* matrixName, mat4 value)
{
	if(shader->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot set a mat4 uniform in an invalid shader");
		printf("\tName: %s\n", matrixName);
		#endif
		return;
	}
	
	//find the uniform that matches its name with "matrixName"
	for(uint8_t uniformIndex = 0; uniformIndex < shader->uniformsStored; uniformIndex++)
	{
		//Check if the uniforms match by name
		if(strcmp(shader->uniformNames[uniformIndex], matrixName) == 0)
		{
			glUseProgram(shader->handle);
			bigc_currentlyBoundShader = shader->handle;
			glUniformMatrix4fv(shader->uniformLocations[uniformIndex], 1, GL_FALSE, value[0]);
			return;
		}
	}

	#ifdef DEBUG
	BIGC_LOG_WARNING("uniform of type mat4 not found");
	printf("\tName: %s\n", matrixName);
	#endif
}

void bigc_shaders_FreeFromGPU(bigc_ShaderProgram* shaderPointer)
{
	if(shaderPointer->handle != 0)
	{
		glDeleteProgram(shaderPointer->handle);
		shaderPointer->handle = 0;
	}
}