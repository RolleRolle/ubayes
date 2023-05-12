#include "../umath/fixmath.h"

unsigned int generateDotGraph(char *buffer, unsigned int maxlength);
unsigned int ExportDotGraph(char *textbuffer, char *filename);
unsigned int DisplayDotGraph(char *textbuffer);

int normalizeArray(fix *array, int size);
unsigned int convertArray(fix *output, double *input, int offset, unsigned int size);
unsigned int printArray(fix *array, double scale, int offset, int size, char *filename);
unsigned int printValue(fix value, double scale, int offset, int size, char *filename);
