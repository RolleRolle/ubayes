#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "node.h"
#include "graph.h"

int main(int argc, char **argv)
{
	char bdot[32768];  unsigned int blength = 256;
	unsigned int nn[3];
	char name1[] = "TEST1",  name2[] = "TEST2",  name3[] = "TEST3";
	fix dist1[] = {DOUBLE_TO_FIX(0.4), DOUBLE_TO_FIX(0.6)};
	fix dist2[] = {DOUBLE_TO_FIX(0.6), DOUBLE_TO_FIX(0.4)};
	fix dist3[] = {DOUBLE_TO_FIX(0.5), DOUBLE_TO_FIX(0.5)};

	printf("Byte Sizes: short=%ld int=%ld long=%ld fix=%ld fixrad=%ld BERR=%d\n",
		sizeof(short), sizeof(int), sizeof(long), sizeof(fix), sizeof(fixrad), BERR);

	nn[0] = addBNode(name1, dist1, 2);
	nn[1] = addBNode(name2, dist2, 2);
	nn[2] = addBNode(name3, dist3, 2);

	nn[0] = findBNode(name1);
	nn[1] = findBNode(name2);
	nn[2] = findBNode(name3);

	addBParent(nn[2], nn[0]);
	addBParent(nn[2], nn[1]);

	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.9), 0, 0, 0);
	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.1), 1, 0, 0);
	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.8), 0, 1, 0);
	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.2), 1, 1, 0);
	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.7), 0, 0, 1);
	setBNodeProbElementCoords(nn[2], DOUBLE_TO_FIX(0.3), 1, 0, 1);

	inferBNode(nn[2]);

	printBNetwork();

	generateDotGraph(bdot, blength);
	DisplayDotGraph(bdot);

	freeBNetwork();

	return 0;
}
