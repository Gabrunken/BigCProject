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
				   		{vertexBufferHandle},
				   		indexBufferHandle,
				   		bytesOfIndices / sizeof(unsigned int),
						vertexDataLayout->attributes};

	return model;
}

void bigc_model_PushInstancedAttribute(bigc_Model* model, const void* data, uint32_t dataSize, uint8_t attributeComponents, GLenum componentType, uint32_t offsetToValue)
{
	glBindVertexArray(model->vertexArrayHandle);

	GLint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(model->attributes);
	glVertexAttribPointer(model->attributes, attributeComponents, componentType, GL_FALSE, offsetToValue, (void*)0);
	glVertexAttribDivisor(model->attributes, 1);

	model->vertexBufferHandles[model->attributes] = instanceVBO;
	model->attributes++;
}

//04-03-2025 14:46 -> Probabilmente da riscrivere tutta questa funzione, usare il metodo di testing just-in-time sarebbe ottimale, poichè non so nemmeno dove l'errore si trova
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

	uint32_t uvCount = 0;

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

		else if(strcmp(word, "vt") == 0) //here UVs are almost always more than positions and/or normals, for that after i must duplicate the positions and normals until they match by count with UVs
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

			uvCount++;
		}

		else if(strcmp(word, "f") == 0)
		{
			indexCount += 3;
		}

		memset(lineString, 0, sizeof(lineString));
	}

	if(uvCount > vertexCount)
	{
		vertexCount = vertexCount; //now when i'll malloc, i'll actually allocate enough memory for me to duplicate what i need to duplicate (normals and positions most of the times)
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

	GLuint* normalIndexData = NULL;
	GLuint* uvIndexData = NULL;

	if(vertexAttributes & BIGC_VERTEX_NORMAL)
		normalIndexData = (GLuint*)malloc(indexCount * sizeof(GLuint));
	
	if(vertexAttributes & BIGC_VERTEX_UV)
		uvIndexData = (GLuint*)malloc(indexCount * sizeof(GLuint));

	uint32_t indexCounter = 0;

	if(indexData == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to malloc index data while loading an .obj");
		#endif
		return BIGC_BAD_MODEL;
	}

	uint32_t byteCounter = 0;
	fseek(filePointer, 0, SEEK_SET); //reset the file cursor to read it from the start
	//second pass
	//now actually fill the arrays
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
			sscanf(lineString, "%*s %f %f %f", (float*)(vertexData + byteCounter), (float*)(vertexData + byteCounter + sizeof(float)), (float*)(vertexData + byteCounter + sizeof(float) * 2));
			byteCounter += sizeof(vec3);
		}

		else if(strcmp(word, "vn") == 0)
		{
			vec3 normal;
			sscanf(lineString, "%*s %f %f %f", (float*)(vertexData + byteCounter), (float*)(vertexData + byteCounter + sizeof(float)), (float*)(vertexData + byteCounter + sizeof(float) * 2));
			byteCounter += sizeof(vec3);
		}

		else if(strcmp(word, "vt") == 0)
		{
			vec2 uv;
			sscanf(lineString, "%*s %f %f", (float*)(vertexData + byteCounter), (float*)(vertexData + byteCounter + sizeof(float)));
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
				sscanf(lineString, "%*s %u/%u/%u %u/%u/%u %u/%u/%u",
					&indexData[indexCounter], &uvIndexData[indexCounter], &normalIndexData[indexCounter],
					&indexData[indexCounter + 1], &uvIndexData[indexCounter + 1], &normalIndexData[indexCounter + 1],
					&indexData[indexCounter + 2], &uvIndexData[indexCounter + 2], &normalIndexData[indexCounter + 2]);

				*(indexData + indexCounter) -= 1;
				*(indexData + indexCounter + 1) -= 1;
				*(indexData + indexCounter + 2) -= 1;

				*(normalIndexData + indexCounter) -= 1;
				*(normalIndexData + indexCounter + 1) -= 1;
				*(normalIndexData + indexCounter + 2) -= 1;
			
				*(uvIndexData + indexCounter) -= 1;
				*(uvIndexData + indexCounter + 1) -= 1;
				*(uvIndexData + indexCounter + 2) -= 1;
			}

			//Positions and UV's
			else if(vertexAttributes & BIGC_VERTEX_UV)
			{
				sscanf(lineString, "%*s %u/%u %u/%u %u/%u",
					&indexData[indexCounter], &uvIndexData[indexCounter],
					&indexData[indexCounter + 1], &uvIndexData[indexCounter + 1],
					&indexData[indexCounter + 2], &uvIndexData[indexCounter + 2]);
				
				*(indexData + indexCounter) -= 1;
				*(indexData + indexCounter + 1) -= 1;
				*(indexData + indexCounter + 2) -= 1;

				*(uvIndexData + indexCounter) -= 1;
				*(uvIndexData + indexCounter + 1) -= 1;
				*(uvIndexData + indexCounter + 2) -= 1;
			}

			//Positions and normals
			else if(vertexAttributes & BIGC_VERTEX_NORMAL)
			{
				sscanf(lineString, "%*s %u//%u %u//%u %u//%u",
					&indexData[indexCounter], &normalIndexData[indexCounter],
					&indexData[indexCounter + 1], &normalIndexData[indexCounter + 1],
					&indexData[indexCounter + 2], &normalIndexData[indexCounter + 2]);

				*(indexData + indexCounter) -= 1;
				*(indexData + indexCounter + 1) -= 1;
				*(indexData + indexCounter + 2) -= 1;

				*(normalIndexData + indexCounter) -= 1;
				*(normalIndexData + indexCounter + 1) -= 1;
				*(normalIndexData + indexCounter + 2) -= 1;
			}

			//Only positions
			else
			{
				sscanf(lineString, "%*s %u %u %u", &indexData[indexCounter], &indexData[indexCounter + 1], &indexData[indexCounter + 2]);
				*(indexData + indexCounter) -= 1;
				*(indexData + indexCounter + 1) -= 1;
				*(indexData + indexCounter + 2) -= 1;
			}

			indexCounter += 3;
		}

		memset(lineString, 0, sizeof(lineString));
	}

	fclose(filePointer);
	
	float* normalOriginalData = NULL;
	float* uvOriginalData = NULL;

	if(vertexAttributes & BIGC_VERTEX_NORMAL)
	{
		normalOriginalData = (float*)malloc(sizeof(vec3) * vertexCount);
		memcpy(normalOriginalData, vertexData + vertexLayout.bytesToNextAttributes[0], sizeof(vec3) * vertexCount); //here i'm supposing that normals come right after positions, uvs might also come after positions so i should take this into account
	}

	if(vertexAttributes & BIGC_VERTEX_UV)
	{
		uvOriginalData = (float*)malloc(sizeof(vec2) * vertexCount);
		memcpy(uvOriginalData, vertexData + vertexLayout.bytesToNextAttributes[1], sizeof(vec2) * vertexCount); //here i'm supposing that normals come right after positions, uvs might also come after positions so i should take this into account
	}


	if(vertexLayout.attributes > 1)
	{
		//rearrange normals and texture coordinates from .obj file
		for(uint32_t vertex = 0; vertex < vertexCount; vertex++)
		{
			//parse through indices until we find index of number "vertex"
			for(uint32_t index = 0; index < indexCount; index++)
			{
				if(*(indexData + index) == vertex)
				{
					//now swap basing ourselves with normalIndexData and uvIndexData
					if(vertexAttributes & BIGC_VERTEX_NORMAL)
					{
						//here i'm supposing that normals come right after positions, uvs might also come after positions so i should take this into account
						memcpy(vertexData + vertexLayout.bytesToNextAttributes[0] + (vertex * vertexLayout.bytesToNextValues[1]),
							   normalOriginalData + normalIndexData[index] * vertexLayout.attributeComponents[1], vertexLayout.bytesToNextValues[1]);
					}

					if(vertexAttributes & BIGC_VERTEX_UV)
					{
						//here i'm supposing that normals come right after positions, uvs might also come after positions so i should take this into account
						memcpy(vertexData + vertexLayout.bytesToNextAttributes[1] + (vertex * vertexLayout.bytesToNextValues[2]),
							   uvOriginalData + uvIndexData[index] * vertexLayout.attributeComponents[2], vertexLayout.bytesToNextValues[2]);
					}

					break;
				}
			}
		}
	}

	bigc_Model model = bigc_model_SendToGPU(vertexData, vertexByteSize * vertexCount, &vertexLayout, indexData, indexCount * sizeof(GLuint));

	free(vertexData);
	free(normalOriginalData);
	free(uvOriginalData);
	free(indexData);
	free(normalIndexData);
	free(uvIndexData);

	return model;
}

void bigc_model_FreeFromGPU(bigc_Model* model)
{
	//Free everything about the model, if it is not already set at 0.
	if(model->vertexArrayHandle != 0)
		glDeleteVertexArrays(1, &model->vertexArrayHandle);

	for(int i = 0; i < BIGC_MAX_VBO_PER_MODEL; i++)
	{
		if(model->vertexBufferHandles[i] != 0)
		{
			glDeleteBuffers(1, &model->vertexBufferHandles[i]);	
		}
	}

	if(model->indexBufferHandle != 0)
		glDeleteBuffers(1, &model->indexBufferHandle);
}
