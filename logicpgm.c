#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "node.h"
#include "graph.h"
#include "logic.h"

#define BSIZE 2

int main(int argc, char **argv)
{
	char bdot[32768];
	unsigned int nn[MAX_NODES] = {0};
	fix bnull[BSIZE]  = {FIX_ZERO, FIX_ZERO};

	fix inputA[BSIZE] = {DOUBLE_TO_FIX(0.0), DOUBLE_TO_FIX(1.0)};
	fix inputB[BSIZE] = {DOUBLE_TO_FIX(0.0), DOUBLE_TO_FIX(1.0)};
	fix inputC[BSIZE] = {DOUBLE_TO_FIX(0.0), DOUBLE_TO_FIX(1.0)};
	fix inputD[BSIZE] = {DOUBLE_TO_FIX(0.0), DOUBLE_TO_FIX(1.0)};

	printf("Byte Sizes: short=%ld int=%ld long=%ld fix=%ld fixrad=%ld BERR=%d\n",
		sizeof(short), sizeof(int), sizeof(long), sizeof(fix), sizeof(fixrad), BERR);

	//AND2
	nn[1] = addBNode("AND2", bnull, BSIZE);
	addBNodeParent("AND2_inputA", inputA, BSIZE, nn[1]);
	addBNodeParent("AND2_inputB", inputB, BSIZE, nn[1]);
	makeLogic(AND, nn[1]);
	inferBNode(nn[1]);

	//AND3
	nn[2] = addBNode("AND3", bnull, BSIZE);
	addBNodeParent("AND3_inputA", inputA, BSIZE, nn[2]);
	addBNodeParent("AND3_inputB", inputB, BSIZE, nn[2]);
	addBNodeParent("AND3_inputC", inputC, BSIZE, nn[2]);
	makeLogic(AND, nn[2]);
	inferBNode(nn[2]);

	//AND4
	nn[3] = addBNode("AND4", bnull, BSIZE);
	addBNodeParent("AND4_inputA", inputA, BSIZE, nn[3]);
	addBNodeParent("AND4_inputB", inputB, BSIZE, nn[3]);
	addBNodeParent("AND4_inputC", inputC, BSIZE, nn[3]);
	addBNodeParent("AND4_inputD", inputD, BSIZE, nn[3]);
	makeLogic(AND, nn[3]);
	inferBNode(nn[3]);

	//OR2
	nn[4] = addBNode("OR2", bnull, BSIZE);
	addBNodeParent("OR2_inputA", inputA, BSIZE, nn[4]);
	addBNodeParent("OR2_inputB", inputB, BSIZE, nn[4]);
	makeLogic(OR, nn[4]);
	inferBNode(nn[4]);

	//OR3
	nn[5] = addBNode("OR3", bnull, BSIZE);
	addBNodeParent("OR3_inputA", inputA, BSIZE, nn[5]);
	addBNodeParent("OR3_inputB", inputB, BSIZE, nn[5]);
	addBNodeParent("OR3_inputC", inputC, BSIZE, nn[5]);
	makeLogic(OR, nn[5]);
	inferBNode(nn[5]);

	//OR4
	nn[6] = addBNode("OR4", bnull, BSIZE);
	addBNodeParent("OR4_inputA", inputA, BSIZE, nn[6]);
	addBNodeParent("OR4_inputB", inputB, BSIZE, nn[6]);
	addBNodeParent("OR4_inputC", inputC, BSIZE, nn[6]);
	addBNodeParent("OR4_inputD", inputD, BSIZE, nn[6]);
	makeLogic(OR, nn[6]);
	inferBNode(nn[6]);

	//XOR2
	nn[7] = addBNode("XOR2", bnull, BSIZE);
	addBNodeParent("XOR2_inputA", inputA, BSIZE, nn[7]);
	addBNodeParent("XOR2_inputB", inputB, BSIZE, nn[7]);
	makeLogic(XOR, nn[7]);
	inferBNode(nn[7]);

	//XOR3
	nn[8] = addBNode("XOR3", bnull, BSIZE);
	addBNodeParent("XOR3_inputA", inputA, BSIZE, nn[8]);
	addBNodeParent("XOR3_inputB", inputB, BSIZE, nn[8]);
	addBNodeParent("XOR3_inputC", inputC, BSIZE, nn[8]);
	makeLogic(XOR, nn[8]);
	inferBNode(nn[8]);

	//XOR4
	nn[9] = addBNode("XOR4", bnull, BSIZE);
	addBNodeParent("XOR4_inputA", inputA, BSIZE, nn[9]);
	addBNodeParent("XOR4_inputB", inputB, BSIZE, nn[9]);
	addBNodeParent("XOR4_inputC", inputC, BSIZE, nn[9]);
	addBNodeParent("XOR4_inputD", inputD, BSIZE, nn[9]);
	makeLogic(XOR, nn[9]);
	inferBNode(nn[9]);

	//IMP2
	nn[10] = addBNode("IMP2", bnull, BSIZE);
	addBNodeParent("IMP2_inputA", inputA, BSIZE, nn[10]);
	addBNodeParent("IMP2_inputB", inputB, BSIZE, nn[10]);
	makeLogic(IMP, nn[1]);
	inferBNode(nn[1]);

	//IMP3
	nn[11] = addBNode("IMP3", bnull, BSIZE);
	addBNodeParent("IMP3_inputA", inputA, BSIZE, nn[11]);
	addBNodeParent("IMP3_inputB", inputB, BSIZE, nn[11]);
	addBNodeParent("IMP3_inputC", inputC, BSIZE, nn[11]);
	makeLogic(IMP, nn[2]);
	inferBNode(nn[2]);

	//IMP4
	nn[12] = addBNode("IMP4", bnull, BSIZE);
	addBNodeParent("IMP4_inputA", inputA, BSIZE, nn[12]);
	addBNodeParent("IMP4_inputB", inputB, BSIZE, nn[12]);
	addBNodeParent("IMP4_inputC", inputC, BSIZE, nn[12]);
	addBNodeParent("IMP4_inputD", inputD, BSIZE, nn[12]);
	makeLogic(IMP, nn[3]);
	inferBNode(nn[3]);

	printBNetwork();

	generateDotGraph(bdot, BSIZE);
	ExportDotGraph(bdot, "graph.dot");
	DisplayDotGraph(bdot);

	freeBNetwork();

	return 0;
}
