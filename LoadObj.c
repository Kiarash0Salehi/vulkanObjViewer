
/*
* Copyright 2025 Ki(y)arash Salehi
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
* OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "model.h"
#include "vec.h"
#include "LoadObj.h"

static void* resizeBuffer(void* buffer, size_t typeSize, size_t count)
{
    void* r = realloc(buffer, typeSize * count);
    if (!r) {
        free(buffer);
        return NULL;
    }
    return r;
}

Model* LoadModel(const char* const path) 
{

    if (strcmp(path + strlen(path) - 4, ".obj"))
    {
		printf("wrong file format. please pass a .obj file to the function\n");
        return -1;
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        printf("Failed to open file: %s\n", path);
        return -1;
    }

    Model* model = calloc(1, sizeof(Model));

    vec3* temp_vertices = NULL;
    vec2* temp_uvs = NULL;
    vec3* temp_normals = NULL;
    unsigned int* temp_vertexIndices = NULL;
    unsigned int* temp_uvIndices = NULL;
    unsigned int* temp_normalIndices = NULL;

    unsigned int vCount = 0, _uvCount = 0, nCount = 0, fCount = 0;
    char line[128];
    
    while (fgets(line, sizeof(line), file)) 
    {
        if (strncmp(line, "v ", 2) == 0) 
        {
            temp_vertices = resizeBuffer(temp_vertices, sizeof(vec3), vCount + 1);
            if (!temp_vertices) {
                fclose(file);
                return -1;
            }
            if (sscanf(line, "v %f %f %f", &temp_vertices[vCount].x, &temp_vertices[vCount].y, &temp_vertices[vCount].z) != 3) {
                fclose(file);
                return -1;
            }
            vCount++;
        }
        else if (strncmp(line, "vt ", 3) == 0) 
        {
            temp_uvs = resizeBuffer(temp_uvs, sizeof(vec2), _uvCount + 1);
            if (!temp_uvs) {
                fclose(file);
                return -1;
            }
            if (sscanf(line, "vt %f %f", &temp_uvs[_uvCount].x, &temp_uvs[_uvCount].y) != 2) {
                fclose(file);
                return -1;
            }
            _uvCount++;
        }
        else if (strncmp(line, "vn ", 3) == 0) 
        {
            temp_normals = resizeBuffer(temp_normals, sizeof(vec3), nCount + 1);
            if (!temp_normals) {
                fclose(file);
                return -1;
            }
            if (sscanf(line, "vn %f %f %f", &temp_normals[nCount].x, &temp_normals[nCount].y, &temp_normals[nCount].z) != 3) {
                fclose(file);
                return -1;
            }
            nCount++;
        }
        else if (strncmp(line, "f ", 2) == 0) 
        {
            unsigned int v[3], vt[3], vn[3];
            int matches = sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
                &v[0], &vt[0], &vn[0],
                &v[1], &vt[1], &vn[1],
                &v[2], &vt[2], &vn[2]);
            if (matches == 9) {
                temp_vertexIndices = resizeBuffer(temp_vertexIndices, sizeof(unsigned int), fCount + 3);
                temp_uvIndices = resizeBuffer(temp_uvIndices, sizeof(unsigned int), fCount + 3);
                temp_normalIndices = resizeBuffer(temp_normalIndices, sizeof(unsigned int), fCount + 3);
                if (!temp_vertexIndices || !temp_uvIndices || !temp_normalIndices) {
                    fclose(file);
                    return -1;
                }
                for (int i = 0; i < 3; i++) {
                    temp_vertexIndices[fCount] = v[i] - 1;
                    temp_uvIndices[fCount] = vt[i] - 1;
                    temp_normalIndices[fCount] = vn[i] - 1;
                    fCount++;
                }
            }
        }
    }
    fclose(file);

    model->positions = temp_vertices;
    model->uvs = temp_uvs;
    model->normals = temp_normals;
    model->positionIndices = temp_vertexIndices;
    model->uvIndices = temp_uvIndices;
    model->normalIndices = temp_normalIndices;
    model->positionCount = vCount;
    model->uvCount = _uvCount;
    model->normalCount = nCount;
    model->positionIndicesCount = fCount;
    model->uvIndicesCount = fCount;
    model->normalIndicesCount = fCount;

    return model;
}

void DestroyModel(Model* model)
{
    if (!model) return;
    free(model->positions);
    free(model->uvs);
    free(model->normals);
    free(model->positionIndices);
    free(model->uvIndices);
    free(model->normalIndices);
    free(model);
}
