//Include in .c file to avoid circular inclusion or something named like that (including more than one time).
#include <shaders.h>
#include <gl3w.h>
#include <stdio.h>
#include <bigc.h>
#include <string.h>

unsigned int bigc_currentlyBoundShader;

bigc_ShaderProgram bigc_shaders_LoadFromDisk(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	//Open vertex shader file
	FILE* filePointer = fopen(vertexShaderPath, "r"); //The second argument should be in string format (""), not character (''). Otherwise crashes...
	
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
	{
		char buffer[1000];
		char* position;

		do
		{
			fread(buffer, 1, 1000, filePointer);
			position = strstr(buffer, "#end");
		} while(position == NULL);

		characters = position - buffer;
	}

	//Positions the cursor to be at the start of the file
	fseek(filePointer, 0, SEEK_SET);
	//Allocate memory for the string on the heap, because its size is not constant and is determined at runtime.
	string = (char*)malloc(sizeof(char) * characters);

	fseek(filePointer, 0, SEEK_SET); //Put the cursor at the beginning of the file
	fread(string, 1, characters, filePointer); //Put every character from the file to the string
	fclose(filePointer);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &string, NULL); free(string);
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
	filePointer = fopen(fragmentShaderPath, "r");
	
	if(filePointer == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to load a file");
		printf("\tPath: %s\n", fragmentShaderPath);
		#endif

		return BIGC_BAD_SHADER_PROGRAM;
	}

	//Get file size
	{
		char buffer[1000];
		char* position;

		do
		{
			fread(buffer, 1, 1000, filePointer);
			position = strstr(buffer, "#end");
		} while(position == NULL);

		characters = position - buffer;
	}
	
	//Allocate memory for the string
	//Use calloc (to allocate AND intialize everything to 0) because apparently some gibberish was left behind.
	//Probably because of the previous allocation for the vertex shader
	string = (char*)calloc(characters, sizeof(char)*characters);

	fseek(filePointer, 0, SEEK_SET); //Put the cursor at the beginning of the file
	fread(string, 1, characters, filePointer); //Put every character from the file to the string
	fclose(filePointer);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &string, NULL); free(string);
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
	bigc_ShaderProgram shaderProgram = {glCreateProgram(), {}, {}, 0};
	glAttachShader(shaderProgram.handle, vertexShader);
	glAttachShader(shaderProgram.handle, fragmentShader);
	glLinkProgram(shaderProgram.handle);
	glValidateProgram(shaderProgram.handle);
	//In fact this does not delete them, just marks them for deletion.
	//As soon as they are not attached to any shader program anymore, they'll get deleted.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void bigc_shaders_SetFloat(bigc_ShaderProgram* shader, const char* variableName, float value)
{
	int location = -1;

	//Check before if the location is already stored in shader hash-map
	for (unsigned char i = 0; i < shader->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if (strcmp(shader->uniformNames[i], variableName) == 0)
		{
			location = shader->uniformLocations[i];
		}
	}

	//Here we didn't find the uniform stored inside the shader, so we store it now for future fetching
	if (location == -1)
	{
		location = glGetUniformLocation(shader->handle, variableName);

		//Confirm that it did find the uniform inside
		if(location == -1)
		{
			#ifdef DEBUG
			BIGC_LOG_WARNING("a uniform was not found");
			printf("\tName: %s\n", variableName);
			#endif
			return;
		}

		strcpy(shader->uniformNames[shader->uniformsStored], variableName);
		shader->uniformLocations[shader->uniformsStored] = location;
		shader->uniformsStored++;
	}

	if (bigc_currentlyBoundShader != shader->handle)
	{
		glUseProgram(shader->handle);
		bigc_currentlyBoundShader = shader->handle;
	}

	glUniform1f(location, value);
}

void bigc_shaders_SetVec3(bigc_ShaderProgram* shader, const char* variableName, vec3 value)
{
	int location = -1;

	//Check before if the location is already stored in shader hash-map
	for (unsigned char i = 0; i < shader->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if (strcmp(shader->uniformNames[i], variableName) == 0)
		{
			location = shader->uniformLocations[i];
		}
	}

	//Here we didn't find the uniform stored inside the shader, so we store it now for future fetching
	if (location == -1)
	{
		location = glGetUniformLocation(shader->handle, variableName);

		//Confirm that it did find the uniform inside
		if(location == -1)
		{
			#ifdef DEBUG
			BIGC_LOG_WARNING("a uniform was not found");
			printf("\tName: %s\n", variableName);
			#endif
			return;
		}

		strcpy(shader->uniformNames[shader->uniformsStored], variableName);
		shader->uniformLocations[shader->uniformsStored] = location;
		shader->uniformsStored++;
	}

	if (bigc_currentlyBoundShader != shader->handle)
	{
		glUseProgram(shader->handle);
		bigc_currentlyBoundShader = shader->handle;
	}

	glUniform3fv(location, 1, value);
}

void bigc_shaders_SetVec4(bigc_ShaderProgram* shader, const char* variableName, vec4 value)
{
	int location = -1;

	//Check before if the location is already stored in shader hash-map
	for (unsigned char i = 0; i < shader->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if (strcmp(shader->uniformNames[i], variableName) == 0)
		{
			location = shader->uniformLocations[i];
		}
	}

	//Here we didn't find the uniform stored inside the shader, so we store it now for future fetching
	if (location == -1)
	{
		location = glGetUniformLocation(shader->handle, variableName);

		//Confirm that it did find the uniform inside
		if(location == -1)
		{
			#ifdef DEBUG
			BIGC_LOG_WARNING("a uniform was not found");
			printf("\tName: %s\n", variableName);
			#endif
			return;
		}

		strcpy(shader->uniformNames[shader->uniformsStored], variableName);
		shader->uniformLocations[shader->uniformsStored] = location;
		shader->uniformsStored++;
	}

	if (bigc_currentlyBoundShader != shader->handle)
	{
		glUseProgram(shader->handle);
		bigc_currentlyBoundShader = shader->handle;
	}

	glUniform4fv(location, 1, value);
}

void bigc_shaders_SetMatrix(bigc_ShaderProgram* shader, const char* matrixName, mat4 value)
{
	int location = -1;

	//Check before if the location is already stored in shader hash-map
	for (unsigned char i = 0; i < shader->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if (strcmp(shader->uniformNames[i], matrixName) == 0)
		{
			location = shader->uniformLocations[i];
		}
	}

	//Here we didn't find the uniform stored inside the shader, so we store it now for future fetching
	if (location == -1)
	{
		location = glGetUniformLocation(shader->handle, matrixName);

		//Confirm that it did find the uniform inside
		if(location == -1)
		{
			#ifdef DEBUG
			BIGC_LOG_WARNING("a uniform was not found");
			printf("\tName: %s\n", matrixName);
			#endif
			return;
		}

		strcpy(shader->uniformNames[shader->uniformsStored], matrixName);
		shader->uniformLocations[shader->uniformsStored] = location;
		shader->uniformsStored++;
	}

	if (bigc_currentlyBoundShader != shader->handle)
	{
		glUseProgram(shader->handle);
		bigc_currentlyBoundShader = shader->handle;
	}

	glUniformMatrix4fv(location, 1, GL_FALSE, value[0]);
}

void bigc_shaders_FreeFromGPU(unsigned int* shaderHandle)
{
	if (*shaderHandle != 0)
	{
		glDeleteProgram(*shaderHandle);
		*shaderHandle = 0;
	}
}