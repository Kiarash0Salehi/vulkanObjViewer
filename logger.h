#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*
*
*time format types :
* %YYYY  : used to show years
* %MM    : used to show months
* %DD    : used to show days
* %hh    : used to show hours ( 24 hour time)
* %mm    : used to show minutes
* %ss    : used to show seconds
*
* For example :
*   [%YYYY:%MM:%DD]
* Or
*   {%YYYY:%MM:%DD %hh %mm %ss}
* Or
*   [%hh %mm %ss]
* Or
*   '%YYYY %MM %DD %hh %mm %ss'
* Or
*   [%YYYY/%MM/%DD %hh:%mm:%ss]
*/
int LogToFile(FILE* file, const char* timeFormat, const char* const message);

#ifdef __cplusplus
}
#endif