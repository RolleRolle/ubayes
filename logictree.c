#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "node.h"
#include "graph.h"
#include "logic.h"

#define BSIZE 2

#include <sys/time.h>
struct timeval timeStart, timeEnd;
struct timeval totalStart, totalEnd;
void timerStart(void)
{
	gettimeofday(&timeStart, NULL);
}
void timerEnd(const char *tag)
{
	gettimeofday(&timeEnd, NULL);
	printf("TIMER: %s took %fus\n", tag, ((double)(timeEnd.tv_sec - timeStart.tv_sec) * 1000000.0 + (double)(timeEnd.tv_usec - timeStart.tv_usec)));
}

int main(int argc, char **argv)
{
	char bdot[32768];
	char itc[NAME_LENGTH];
	unsigned int nn;

	fix bnull[BSIZE]  = {FIX_ZERO, FIX_ZERO};

	enum logic operation 	=  AND;
	char name [NAME_LENGTH]	= "AND";

	int numi = 100, nump = 2, numl = 3;

	fix input[BSIZE] = {DOUBLE_TO_FIX(0.5), DOUBLE_TO_FIX(0.5)};

	printf("Byte Sizes: short=%ld int=%ld long=%ld fix=%ld fixrad=%ld BERR=%d\n",
		sizeof(short), sizeof(int), sizeof(long), sizeof(fix), sizeof(fixrad), BERR);

	int numnodes = (int)((pow((double)nump, (double)(numl+1))-1.0)/(double)(nump-1));
	printf("Tree with %d levels of %d parents (%d nodes) x %d iterations (total %d inferences)\n", numl, nump, numnodes, numi, numi*numnodes);
	sprintf(itc, "inference x%d", numi*numnodes);

	timerStart();
	nn = addBNode(name, bnull, BSIZE);
	makeLogicTree(operation, nn, name, input, nump, numl);
	timerEnd("initialization");
	timerStart();
	for(int i = 0; i < numi; i++)
		inferBNetwork(nn);
	timerEnd(itc);

	printBNetwork();

	generateDotGraph(bdot, BSIZE);
	ExportDotGraph(bdot, "graph.dot");
	DisplayDotGraph(bdot);

	freeBNetwork();

	return 0;
}
