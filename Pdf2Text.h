#ifndef Pdf2Text_h__
#define Pdf2Text_h__

#include <stdio.h>
#include <ctype.h>
#include <string.h>

//Keep this many previous recent characters for back reference:
#define oldchar 15

size_t FindStringInBuffer (char* buffer, char* search, size_t buffersize);
float ExtractNumber(const char* search, int lastcharoffset);
bool seen2(const char* search, char* recent);
void ProcessOutput(FILE* file, char* output, size_t len);
void Pdf2Text(const char* pdf, const char* text);

#endif // Pdf2Text_h__