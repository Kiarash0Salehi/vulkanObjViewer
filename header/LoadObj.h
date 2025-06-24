#pragma once

#include "model.h"

#ifndef _LOADOBJ_H_
#define _LOADOBJ_H_

#ifdef __cplusplus
extern "C"
{
#endif

	Model* LoadModel(const char* const path);
	void DestroyModel(Model* model);

#ifdef __cplusplus
}
#endif

#endif // _LOADOBJ_H_