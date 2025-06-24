
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

#ifndef _MODEL_H_
#define _MODEL_H_

#ifdef __cplusplus
extern "C"
{
#endif
	typedef struct Model
	{

		// the reason behind this form of structures is that I don't wanted to use any external libraries like glm so that will be very easy to use 
		// also you can cast it to glm::vec3 and glm::vec2 like this :	
		// 
		// glm::vec3* vec = (glm::vec3*)module->vertices;
		// glm::vec2* uv  = (glm::vec2*)module->uvs;
		// 

		struct // vec3
		{
			float x;
			float y;
			float z;
		} *positions;
		struct // vec2
		{
			float x;
			float y;
		} *uvs;
		struct //vec3
		{
			float x;
			float y;
			float z;
		} *normals;

		unsigned int positionCount;
		unsigned int uvCount;
		unsigned int normalCount;

		unsigned int* positionIndices;
		unsigned int* uvIndices;
		unsigned int* normalIndices;

		unsigned int positionIndicesCount;
		unsigned int uvIndicesCount;
		unsigned int normalIndicesCount;
	} Model;

#ifdef __cplusplus
}
#endif

#endif // _MODEL_H_