#pragma once

#include "mat4.h"

#include <math.h>

#ifndef _MATRIX_H_
#define _MATRIX_H_

static mat4f makeMat4(float s)
{
	mat4f matrix = 
	{ 
		.m =
		{
			{s, 0.0f, 0.0f, 0.0f},
			{0.0f, s, 0.0f, 0.0f},
			{0.0f, 0.0f, s, 0.0f},
			{0.0f, 0.0f, 0.0f, s}
		}
	};
	return matrix;
}

static mat4f Mat4f_MultiplyMatrix(mat4f m1, mat4f m2)
{
    mat4f matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return matrix;
}

static mat4f makeTranslationMatrix(float x, float y, float z)
{
    mat4f matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

static mat4f translateMatrix(float tx, float ty, float tz, mat4f matrix)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    matrix.m[0][3] = tx;
    matrix.m[1][3] = ty;
    matrix.m[2][3] = tz;

    return matrix;
}

static mat4f getPerspectiveFovMat(float fFovDegrees, float height, float width, float fNear, float fFar)
{
	float fAspectRatio = height / width;
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	mat4f matrix = { 0 };
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

static mat4f inverseMat4f(mat4f matrix) 
{
    mat4f inverse = { 0 };
    float temp[4][4], adj[4][4], det = 0;

    // Compute determinant using Laplace expansion
    for (int i = 0; i < 4; i++) {
        int subi = 0;
        for (int j = 1; j < 4; j++) {
            int subj = 0;
            for (int k = 0; k < 4; k++) {
                if (k == i) continue;
                temp[subi][subj++] = matrix.m[j][k];
            }
            subi++;
        }
        float subdet = temp[0][0] * temp[1][1] * temp[2][2] + temp[0][1] * temp[1][2] * temp[2][0] + temp[0][2] * temp[1][0] * temp[2][1] -
            temp[0][2] * temp[1][1] * temp[2][0] - temp[0][1] * temp[1][0] * temp[2][2] - temp[0][0] * temp[1][2] * temp[2][1];
        det += (i % 2 == 0 ? 1 : -1) * matrix.m[0][i] * subdet;
    }

    if (det == 0) 
    {
    }

    // Compute adjugate matrix
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int subi = 0;
            for (int x = 0; x < 4; x++) {
                if (x == i) continue;
                int subj = 0;
                for (int y = 0; y < 4; y++) {
                    if (y == j) continue;
                    temp[subi][subj++] = matrix.m[x][y];
                }
                subi++;
            }
            float subdet = temp[0][0] * temp[1][1] * temp[2][2] + temp[0][1] * temp[1][2] * temp[2][0] + temp[0][2] * temp[1][0] * temp[2][1] -
                temp[0][2] * temp[1][1] * temp[2][0] - temp[0][1] * temp[1][0] * temp[2][2] - temp[0][0] * temp[1][2] * temp[2][1];
            adj[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * subdet;
        }
    }

    // Compute inverse by dividing adjugate by determinant
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            inverse.m[i][j] = adj[i][j] / det;
        }
    }
    return inverse;
}


#endif // _MATRIX_H_