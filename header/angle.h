#pragma once

#ifndef _ANGLE_H_
#define _ANGLE_H_

static float toRadians(float degrees)
{
	return degrees * 3.14159265f / 180.0f;
}

static float toDegrees(float radians)
{
	return radians * 180.0f / 3.14159265f;
}


#endif // _ANGLE_H