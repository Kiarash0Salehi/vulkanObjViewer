#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>


int LogToFile(FILE* file, const char* timeFormat, const char* const message)
{
    char tFormat[128] = { 0 };
    int tFormat_index = 0;
    size_t i = 0;
    while (timeFormat[i] != '\0')
    {
        if (timeFormat[i] == '%')
        {
            // Handle custom format specifiers
            if (strncmp(&timeFormat[i + 1], "YYYY", 4) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'Y';
                i += 5; // Skip "YYYY"
            }
            else if (strncmp(&timeFormat[i + 1], "MM", 2) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'm';
                i += 3; // Skip "MM"
            }
            else if (strncmp(&timeFormat[i + 1], "DD", 2) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'd';
                i += 3; // Skip "DD"
            }
            else if (strncmp(&timeFormat[i + 1], "hh", 2) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'H';
                i += 3; // Skip "hh"
            }
            else if (strncmp(&timeFormat[i + 1], "mm", 2) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'M';
                i += 3; // Skip "mm"
            }
            else if (strncmp(&timeFormat[i + 1], "ss", 2) == 0)
            {
                tFormat[tFormat_index++] = '%';
                tFormat[tFormat_index++] = 'S';
                i += 3; // Skip "SS"
            }
            else
            {
                // Handle unknown specifiers
                tFormat[tFormat_index++] = timeFormat[i++];
            }
        }
        else
        {
            // Copy other characters as-is
            tFormat[tFormat_index++] = timeFormat[i++];
        }
    }


    tFormat[tFormat_index] = '\0'; // Null-terminate the format string

    // Get the current time
    time_t t;
    time(&t);
    struct tm* tm_local = localtime(&t);

    // Format the time
    char strLocalTime[256] = { 0 };
    strftime(strLocalTime, sizeof(strLocalTime), tFormat, tm_local);

    // Write to the file
    if (file)
    {
        fprintf(file, "%s%s\n", strLocalTime, message);
    }

    return 0;
}