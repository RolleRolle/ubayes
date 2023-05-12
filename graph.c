#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "graph.h"

//NOTE: Graphing and output functions are not really robust and should be used with caution
unsigned int generateDotGraph(char *textbuffer, unsigned int maxlength)
{
	unsigned int nodenum, parnum, index = 0, numparents, parents[MAX_PARENTS], namelength, parnamelength;
	char namebuffer[NAME_LENGTH], parnamebuffer[NAME_LENGTH];

	// Print praph preamble
	index += sprintf(&textbuffer[index], "digraph bayesnetwork {");

	for(nodenum = 0; nodenum < MAX_NODES; nodenum++)
		if(getBNodeSize(nodenum) != BERR)
		{
			// Print node relationships
			memset(namebuffer, 0, NAME_LENGTH); memset(parnamebuffer, 0, NAME_LENGTH);  memset(parents, 0, MAX_PARENTS);
			numparents = getBNodeParents(nodenum, parents);
			namelength = getBNodeName(nodenum, namebuffer);
			for(parnum = 0; parnum < numparents; parnum++)
			{
				parnamelength = getBNodeName(parents[parnum], parnamebuffer);
				if(index > maxlength - namelength - parnamelength - 12) break;
				index += sprintf(&textbuffer[index], " \"%s\" -> \"%s\"; ", parnamebuffer, namebuffer);
			}

			// Print node distributions
			if(index > maxlength - namelength - getBNodeSize(nodenum)*6) break; //TODO: correct this estimate!
			index += sprintf(&textbuffer[index], "{ \"%s\" [ label=\"", namebuffer);
			index += getBNodeProbText(nodenum, &textbuffer[index]); //for full distribution use getBNodeDistText
			index += sprintf(&textbuffer[index], "\"] }");

		} // End node selection loop

	index += sprintf(&textbuffer[index], "}");
	//puts(textbuffer);

	return index;

}

unsigned int ExportDotGraph(char *textbuffer, char *filename)
{
	FILE *fp = fopen(filename, "w");
	if(fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing!\n", filename);
		return 0;
	}
	//fwrite(textbuffer, strlen(textbuffer), strlen(textbuffer)/sizeof(textbuffer[0]), fp);
	fprintf(fp, "%s", textbuffer);
	fclose(fp);

	return strlen(textbuffer);
}

unsigned int DisplayDotGraph(char *textbuffer)
{
	//echo "digraph { a -> b; a -> c; c -> e }" | dot -Tpng | xview /dev/stdin
	char commandbuffer[strlen(textbuffer)+64];
	//printf("echo \'%s\' | dot -Tpng | xview /dev/stdin &", textbuffer);
	sprintf(commandbuffer, "echo \'%s\' | dot -Tpng | xview /dev/stdin &", textbuffer);
	if(system(commandbuffer) < 0)
		fprintf(stderr, "Could not run command \"%s\"\n", commandbuffer);

	return strlen(commandbuffer);
}

int normalizeArray(fix *array, int size)
{
	int counter;
	fix accumulator = FIX_ZERO;
    for(counter = 0; counter < size; counter++)
		accumulator = accumulator + array[counter];
	if(accumulator == FIX_ZERO) return 0;
	for(counter = 0; counter < size; counter++)
		array[counter] = fixdiv(array[counter], accumulator);
	return counter;
}

unsigned int convertArray(fix *output, double *input, int offset, unsigned int size)
{
	unsigned int counter, target;
    for(counter = 0; counter < size; counter++) {
		target = counter + offset;
		if(target >= 0 && target < size)
			output[target] = DOUBLE_TO_FIX(input[counter]);
	}
    return (unsigned int)size;
}

unsigned int printArray(fix *array, double scale, int offset, int size, char *filename)
{
	int counter;
	FILE *fp;
	if((fp = fopen(filename, "w")) < 0) { fprintf(stderr, "Could not open file %s!", filename); return 0; }
    for(counter = 0; counter < size; counter++)
		fprintf(fp, "%g,\t%g\n", (double)(counter-offset)*scale/(double)size, FIX_TO_DOUBLE(array[counter]));

	if(fclose(fp) < 0) { fprintf(stderr, "Could not close file %s!", filename); return 0; }
    return (unsigned int)size;
}

unsigned int printValue(fix value, double scale, int offset, int size, char *filename)
{
	int counter;
	FILE *fp;
	if((fp = fopen(filename, "w")) < 0) { fprintf(stderr, "Could not open file %s!", filename); return 0; }
    for(counter = 0; counter < size; counter++)
		fprintf(fp, "%g,\t%g\n", (double)(counter-offset)*scale/(double)size, FIX_TO_DOUBLE(value));

	if(fclose(fp) < 0) { fprintf(stderr, "Could not close file %s!", filename); return 0; }
    return (unsigned int)size;
}
