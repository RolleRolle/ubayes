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
	unsigned int nn[MAX_NODES];
	fix bnull[BSIZE]  = {FIX_ZERO, FIX_ZERO};
	fix btrue[BSIZE]  = {FIX_ZERO, FIX_ONE};
	fix bfalse[BSIZE] = {FIX_ONE, FIX_ZERO};

	fix *inputA = btrue;
	fix *inputB = bfalse;
	fix *inputC = btrue;

	printf("Byte Sizes: short=%ld int=%ld long=%ld fix=%ld fixrad=%ld BERR=%d\n",
		sizeof(short), sizeof(int), sizeof(long), sizeof(fix), sizeof(fixrad), BERR);

	//ID
	nn[1] = addBNode("ID", bnull, BSIZE);
	nn[0] = addBNodeParent("ID_input", inputA, BSIZE, nn[1]);
	makeLogic(ID, nn[1]);
	inferBNode(nn[1]);

	//NOT
	nn[3] = addBNode("NOT", bnull, BSIZE);
	nn[2] = addBNodeParent("NOT_input", inputA, BSIZE, nn[3]);
	makeLogic(NOT, nn[3]);
	inferBNode(nn[3]);

	//AND
	nn[6] = addBNode("AND", bnull, BSIZE);
	nn[4] = addBNodeParent("AND_inputA", inputA, BSIZE, nn[6]);
	nn[5] = addBNodeParent("AND_inputB", inputB, BSIZE, nn[6]);
	nn[22] = addBNodeParent("AND_inputC", inputC, BSIZE, nn[6]);
	makeLogic(AND, nn[6]);
	inferBNode(nn[6]);

	//NAND
	nn[9] = addBNode("NAND", bnull, BSIZE);
	nn[7] = addBNodeParent("NAND_inputA", inputA, BSIZE, nn[9]);
	nn[8] = addBNodeParent("NAND_inputB", inputB, BSIZE, nn[9]);
	nn[23] = addBNodeParent("NAND_inputC", inputC, BSIZE, nn[9]);
	makeLogic(NAND, nn[9]);
	inferBNode(nn[9]);

	//OR
	nn[12] = addBNode("OR", bnull, BSIZE);
	nn[10] = addBNodeParent("OR_inputA", inputA, BSIZE, nn[12]);
	nn[11] = addBNodeParent("OR_inputB", inputB, BSIZE, nn[12]);
	nn[24] = addBNodeParent("OR_inputC", inputC, BSIZE, nn[12]);
	makeLogic(OR, nn[12]);
	inferBNode(nn[12]);

	//NOR
	nn[15] = addBNode("NOR", bnull, BSIZE);
	nn[13] = addBNodeParent("NOR_inputA", inputA, BSIZE, nn[15]);
	nn[14] = addBNodeParent("NOR_inputB", inputB, BSIZE, nn[15]);
	nn[25] = addBNodeParent("NOR_inputC", inputC, BSIZE, nn[15]);
	makeLogic(NOR, nn[15]);
	inferBNode(nn[15]);

	//XOR
	nn[18] = addBNode("XOR", bnull, BSIZE);
	nn[16] = addBNodeParent("XOR_inputA", inputA, BSIZE, nn[18]);
	nn[17] = addBNodeParent("XOR_inputB", inputB, BSIZE, nn[18]);
	nn[26] = addBNodeParent("XOR_inputC", inputC, BSIZE, nn[18]);
	makeLogic(XOR, nn[18]);
	inferBNode(nn[18]);

	//XNOR
	nn[21] = addBNode("XNOR", bnull, BSIZE);
	nn[19] = addBNodeParent("XNOR_inputA", inputA, BSIZE, nn[21]);
	nn[20] = addBNodeParent("XNOR_inputB", inputB, BSIZE, nn[21]);
	nn[27] = addBNodeParent("XNOR_inputC", inputC, BSIZE, nn[21]);
	makeLogic(XNOR, nn[21]);
	inferBNode(nn[21]);

	printBNetwork();

	generateDotGraph(bdot, BSIZE);
	ExportDotGraph(bdot, "graph.dot");
	DisplayDotGraph(bdot);

	freeBNetwork();

	return 0;
}
