#include <model.h>
#include <bigc.h>
#include <stdio.h>
#include <stdlib.h>

bigc_Model bigc_model_SendToGPU(const void* vertexData, unsigned int bytesOfVertices,
					   const bigc_VertexDataLayout* vertexDataLayout,
					   const unsigned int* indexData, unsigned int bytesOfIndices)
{
	//GPU side model
	GLuint vertexArrayHandle;
	glGenVertexArrays(1, &vertexArrayHandle);
	glBindVertexArray(vertexArrayHandle);

	GLuint vertexBufferHandle;
	glGenBuffers(1, &vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, bytesOfVertices, vertexData, GL_STATIC_DRAW);

	GLuint indexBufferHandle;
	glGenBuffers(1, &indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesOfIndices, indexData, GL_STATIC_DRAW);

	unsigned int offset = 0;
	for(unsigned char attribute = 0; attribute < vertexDataLayout->attributes; attribute++)
	{
		glEnableVertexAttribArray(attribute);

		//Attribute index,
		//components count,
		//component data type,
		//normalized idk,
		//stride (bytes to the next vertex attribute of the same type)
		//offset
		glVertexAttribPointer(attribute,
						 	  vertexDataLayout->attributeComponents[attribute],
							  vertexDataLayout->attributeTypes[attribute],
							  GL_FALSE,
							  vertexDataLayout->bytesToNextValues[attribute],
							  (const void*)offset);

		offset += vertexDataLayout->bytesToNextAttributes[attribute];
	}

	//CPU side model
	bigc_Model model = {vertexArrayHandle,
				   vertexBufferHandle,
				   indexBufferHandle,
				   bytesOfIndices / sizeof(unsigned int)};

	return model;
}

bigc_Model bigc_model_LoadOBJFromDisk(const char* filePath)
{
	//let's set up everything before opening and parsing the file
	uint32_t vertexCount = 0;
	uint8_t vertexByteSize = 0;
	uint8_t vertexAttributes = 0;
	uint32_t indexCount = 0;

	//model data
	void* vertexData;
	GLuint* indexData;

	//vertex layout
	bigc_VertexDataLayout vertexLayout = {0};

	//open file
	FILE* filePointer = fopen(filePath, "r");
	if(filePointer == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to open a .obj file");
		#endif
		return BIGC_BAD_MODEL;
	}

	//i'm going to do a 2 pass parse of the file, 1 to check the number of attributes so that i can malloc correctly and once (also for vertex layout), 2 to actually fill in the arrays

	//now let's read line by line
	uint8_t lineLength;
	char lineString[100];

	while(fgets(lineString, sizeof(lineString), filePointer) != NULL)
	{
		//here we have a line
		//parse and identify what this line is all about
		char word[2]; //2 chars for the possible elements (v, vn or vt)
		memset(word, 0, sizeof(word));
		sscanf(lineString, "%s", word);

		if(strcmp(word, "v") == 0)
		{
			vertexCount++; //position are mandatory inside an .obj, so it's safe to put this here
			if(!(vertexAttributes & BIGC_VERTEX_POSITION))
			{
				vertexAttributes |= BIGC_VERTEX_POSITION;
				vertexLayout.attributes++;
				vertexLayout.attributeComponents[vertexLayout.attributes - 1] = 3;
				vertexLayout.attributeTypes[vertexLayout.attributes - 1] = GL_FLOAT;
				vertexLayout.bytesToNextValues[vertexLayout.attributes - 1] = sizeof(vec3);
				vertexByteSize += sizeof(vec3);
			}
		}

		else if(strcmp(word, "vn") == 0)
		{
			if(!(vertexAttributes & BIGC_VERTEX_NORMAL))
			{
				vertexAttributes |= BIGC_VERTEX_NORMAL;
				vertexLayout.attributes++;
				vertexLayout.attributeComponents[vertexLayout.attributes - 1] = 3;
				vertexLayout.attributeTypes[vertexLayout.attributes - 1] = GL_FLOAT;
				vertexLayout.bytesToNextValues[vertexLayout.attributes - 1] = sizeof(vec3);
				vertexByteSize += sizeof(vec3);
			}
		}

		else if(strcmp(word, "vt") == 0) //danger: texture coordinates could be more than positions, altho i'm allocating memory for vertexSize * positionCount, its gonna create UB pushing further data into vertexData, this is done because of UV seams.
		{
			if(!(vertexAttributes & BIGC_VERTEX_UV))
			{
				vertexAttributes |= BIGC_VERTEX_UV;
				vertexLayout.attributes++;
				vertexLayout.attributeComponents[vertexLayout.attributes - 1] = 2;
				vertexLayout.attributeTypes[vertexLayout.attributes - 1] = GL_FLOAT;
				vertexLayout.bytesToNextValues[vertexLayout.attributes - 1] = sizeof(vec2);
				vertexByteSize += sizeof(vec2);
			}
		}

		else if(strcmp(word, "f") == 0)
		{
			indexCount+=3;
		}

		memset(lineString, 0, sizeof(lineString));
	}

	//if vertexCount is == 0, positions are not present in the file, so return a bad model
	if(vertexCount == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("an .obj file does not have vertex positions");
		#endif
		return BIGC_BAD_MODEL;
    	fclose(filePointer);
	}

	for(uint8_t attributeIndex = 0; attributeIndex < vertexLayout.attributes - 1; attributeIndex++)
	{
		vertexLayout.bytesToNextAttributes[attributeIndex] = vertexLayout.bytesToNextValues[attributeIndex] * vertexCount;
	}

	//malloc data for vertex
	vertexData = malloc(vertexByteSize * vertexCount);
	
	if(vertexData == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to malloc vertex data while loading an .obj");
		#endif
		return BIGC_BAD_MODEL;
	}

	//malloc for indices
	indexData = (GLuint*)malloc(indexCount * sizeof(GLuint));

	uint32_t indexCounter = 0;

	if(vertexData == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to malloc index data while loading an .obj");
		#endif
		return BIGC_BAD_MODEL;
	}

	//duplicate attributes which do not match by count with the positions (like normals could be less than positions)
	//TODO

	uint32_t byteCounter = 0;
	fseek(filePointer, 0, SEEK_SET); //reset the file cursor to read it from the start
	//second pass
	while(fgets(lineString, sizeof(lineString), filePointer) != NULL)
	{
		//here we have a line
		//parse and identify what this line is all about
		char word[2]; //2 chars for the possible elements (v, vn or vt)
		memset(word, 0, sizeof(word));
		sscanf(lineString, "%s", word);
		
		if(strcmp(word, "v") == 0)
		{
			vec3 position;
			sscanf(lineString, "%*s %f %f %f", &position[0], &position[1], &position[2]);
			memcpy(vertexData + byteCounter, &position, sizeof(vec3));
			byteCounter += sizeof(vec3);
		}

		else if(strcmp(word, "vn") == 0)
		{
			vec3 normal;
			sscanf(lineString, "%*s %f %f %f", &normal[0], &normal[1], &normal[2]);
			memcpy(vertexData + byteCounter, &normal, sizeof(vec3));
			byteCounter += sizeof(vec3);
		}

		else if(strcmp(word, "vt") == 0)
		{
			vec2 uv;
			sscanf(lineString, "%*s %f %f", &uv[0], &uv[1]);
			memcpy(vertexData + byteCounter, &uv, sizeof(vec2));
			byteCounter += sizeof(vec2);
		}

		else if(strcmp(word, "f") == 0)
		{
			//layout for each different case:
			//only positions -> f 1 2 3
			//positions and normals -> f 1//2 1//3 2//7
			//position and uv -> f 1/2 3/4 8/2
			//position, uv and normal -> f 1/4/3 1/9/5 8/2/9
			
			//Positions, UV's and normals
			if((vertexAttributes & BIGC_VERTEX_UV) && (vertexAttributes & BIGC_VERTEX_NORMAL))
			{
				sscanf(lineString, "%*s %u/%*u/%*u %u/%*u/%*u %u/%*u/%*u", &indexData[indexCounter], &indexData[indexCounter + 1], &indexData[indexCounter + 2]);
			}

			//Positions and UV's
			else if(vertexAttributes & BIGC_VERTEX_UV)
			{

			}

			//Positions and normals
			else if(vertexAttributes & BIGC_VERTEX_NORMAL)
			{
				sscanf(lineString, "%*s %u//%*u %u//%*u %u//%*u", &indexData[indexCounter], &indexData[indexCounter + 1]);
			}

			//Only positions
			else
			{

			}

			indexCounter += 3;
		}

		memset(lineString, 0, sizeof(lineString));
	}

	//now actually fill the arrays
	fclose(filePointer);

	bigc_Model model = bigc_model_SendToGPU(vertexData, vertexByteSize * vertexCount, &vertexLayout, indexData, indexCount * sizeof(GLuint));
	return model;
}

void bigc_model_FreeFromGPU(bigc_Model* model)
{
	//Free everything about the model, if it is not already set at 0.
	if(model->vertexArrayHandle != 0)
		glDeleteVertexArrays(1, &model->vertexArrayHandle);
	if(model->vertexBufferHandle != 0)
		glDeleteBuffers(1, &model->vertexBufferHandle);
	if(model->indexBufferHandle != 0)
		glDeleteBuffers(1, &model->indexBufferHandle);
}
