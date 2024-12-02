#include <model.h>
#include <bigc.h>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <stdio.h>

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

bigc_Model bigc_model_LoadFromDiskAsSingleModel(const char* filePath)
{
	//open file
	FILE* filePointer = fopen(filePath, "rb");
	if(filePointer == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to open a .gltf or .glb file, check the path");
		#endif
		return BIGC_BAD_MODEL;
	}

	fseek(filePointer, 0, SEEK_END); //this never works correctly
    long fileLength = ftell(filePointer);
    fseek(filePointer, 0, SEEK_SET);

    char* fileData = (char*)malloc(fileLength);
    if(fileData == NULL)
    {
    	#ifdef DEBUG
        BIGC_LOG_ERROR("failed to malloc when loading a model");
        fclose(filePointer);
        #endif
        return BIGC_BAD_MODEL;
    }

    fread(fileData, 1, fileLength, filePointer);
    fclose(filePointer);

    //here we have read the data
	cgltf_options gltfOptions = {0};
	cgltf_data* gltfData = NULL;
	cgltf_result gltfResult = cgltf_parse(&gltfOptions, fileData, fileLength, &gltfData);

	free(fileData); 

	if(gltfResult != cgltf_result_success)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("error while parsing a gltf");
		#endif
		return BIGC_BAD_MODEL;
	}

	gltfResult = cgltf_load_buffers(&gltfOptions, gltfData, filePath);
    
    if(gltfResult != cgltf_result_success)
    {
    	#ifdef DEBUG
    	BIGC_LOG_ERROR("error while loading the buffers from a gltf");
    	#endif
        cgltf_free(gltfData);
        return BIGC_BAD_MODEL;
    }

    // Facoltativo: valida la struttura del file GLTF
    gltfResult = cgltf_validate(gltfData);
    if(gltfResult != cgltf_result_success) 
  	{
    	#ifdef DEBUG
    	BIGC_LOG_ERROR("error while validating a gltf");
    	#endif
        cgltf_free(gltfData);
        return BIGC_BAD_MODEL;
    }

    //data is ready to fetch and use

    //for every mesh
    void* vertexData;
    uint32_t vertexCount = 0;
    uint8_t vertexDataSize = 0;
    void* indexData;
    uint32_t indexCount = 0;

    bigc_VertexDataLayout vertexLayout =
    {
    	1,
    	{3},
    	{GL_FLOAT},
    	{},
    	{}
    };

    for(int meshIndex = 0; meshIndex < gltfData->meshes_count; meshIndex++)
    {
    	//for every primitive (not sure what they are)
    	for(int primitiveIndex = 0; primitiveIndex < gltfData->meshes[meshIndex].primitives_count; primitiveIndex++)
    	{	
    		cgltf_primitive* primitive = &gltfData->meshes[meshIndex].primitives[primitiveIndex];
    		cgltf_attribute* positionAttribute;
    		cgltf_accessor* indexAccessor = primitive->indices;

    		indexData =
			indexAccessor->buffer_view->buffer->data + 
            indexAccessor->buffer_view->offset + 
            indexAccessor->offset;

            indexCount = indexAccessor->count;

    		for(int attributeIndex = 0; attributeIndex < primitive->attributes_count; attributeIndex++)
    		{
    			if(primitive->attributes[attributeIndex].type == cgltf_attribute_type_position) //if it is a position
    			{
    				positionAttribute = &primitive->attributes[attributeIndex];
    				cgltf_accessor* positionAccessor = positionAttribute->data;
    				cgltf_buffer_view* bufferView = positionAccessor->buffer_view;
					cgltf_buffer* buffer = bufferView->buffer;

					vertexData = buffer->data + bufferView->offset + positionAccessor->offset;

					vertexCount += positionAccessor->count;
					vertexDataSize += positionAccessor->count * sizeof(float);

					vertexLayout.bytesToNextValues[0] = sizeof(float) * 3;
	    		}
    		}
    	}
    }

    cgltf_free(gltfData); //free the data

	bigc_Model model = bigc_model_SendToGPU(vertexData, vertexDataSize, &vertexLayout, indexData, indexCount * sizeof(GLuint));
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
