#pragma once

#include <math.h>
#include "mat4.h"

#ifndef _EULER_ANGLE_H_
#define _EULER_ANGLE_H_

static mat4f eulerToRotationMatrixXYZ(float x, float y, float z) 
{
    mat4f mat = { 0 };
    float cx = cos(x), sx = sin(x);
    float cy = cos(y), sy = sin(y);
    float cz = cos(z), sz = sin(z);

    // Compute rotation matrix
    mat.m[0][0] = cy * cz;
    mat.m[0][1] = -cy * sz;
    mat.m[0][2] = sy;
    mat.m[0][3] = 0.0f;

    mat.m[1][0] = sx * sy * cz + cx * sz;
    mat.m[1][1] = -sx * sy * sz + cx * cz;
    mat.m[1][2] = -sx * cy;
    mat.m[1][3] = 0.0f;

    mat.m[2][0] = -cx * sy * cz + sx * sz;
    mat.m[2][1] = cx * sy * sz + sx * cz;
    mat.m[2][2] = cx * cy;
    mat.m[2][3] = 0.0f;

    mat.m[3][0] = 0.0f;
    mat.m[3][1] = 0.0f;
    mat.m[3][2] = 0.0f;
    mat.m[3][3] = 1.0f;

    return mat;
}

#endif // _EULER_ANGLE_H_