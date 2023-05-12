#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "node.h"
#include "logic.h"

unsigned int normalize(fix *dist, unsigned int size)
{
	unsigned int counter;
	fix sum = FIX_ZERO;
    for(counter = 0; counter < size; counter++)
	{
    	sum += dist[counter];
	}
    if(sum == FIX_ZERO)
    	return 0;
    for(counter = 0; counter < size; counter++)
	{
    	dist[counter] = fixdiv(dist[counter],sum);
	}
	return counter;
}

fix normal(double value, double stddev, double mean)
{
	return DOUBLE_TO_FIX( exp( -pow((((double)value-mean)/stddev), 2.0) / 2.0 ) );
}

fix gaussian(double value, double stddev, double mean)
{
	return DOUBLE_TO_FIX( ( 1/(stddev * sqrt(2.0 * M_PI)) ) * exp( -pow((((double)value-mean)/stddev), 2.0) / 2.0 ) );
}

unsigned int makeGaussian(fix *dist, double stddev, double mean, unsigned int size)
{
	unsigned int counter;
    for(counter = 0; counter < size; counter++)
	{
		dist[counter] = DOUBLE_TO_FIX( ( 1/(stddev * sqrt(2.0 * M_PI)) ) * exp( -pow((((double)counter-mean)/stddev), 2.0) / 2.0 ) );
	}
	return counter;
}

unsigned int makeLogic(enum logic operation, unsigned int nodenum)
{
	// Construct a Boolean logic operation for Bernoulli random variables as a conditional probability distribution in a node
	const unsigned int numValues = 2; //Bernoulli RV with two values: P(true) and P(false)
	unsigned int numParents;
	if((numParents = getBNodeNumParents(nodenum)) == BERR) return BERR; // Checks for valid distribution in node
	unsigned int parnum, valueCoords[numParents+2], valueMaxes[numParents+1];
	// NOTE: for N-dimensional coordinates in valueCoords and getIndex, dimension 0 is always the local states of the node itself
	// Dimensions 1...N are indexed for the states of the node's N-1 parents, which are enumerated 0...N-1
	// We check the dimension value after these (N+1 or parents+2) for overflow to find when all states have been seen
	fix prob[numValues];
	memset(valueCoords, 0,  (numParents+2)*sizeof(unsigned int));
	memset(valueMaxes, 0,  (numParents+2)*sizeof(unsigned int));
	getBNodeParentNumValues(nodenum, valueMaxes);

	while(valueCoords[numParents + 1] == 0)
	{
		// Set the "false" probability prob[0] for the conditional distribution depending on what operation is desired
		// This typically depends on whether the tensor index of valueCoords refers to a true (1) or false (0) parent value
		switch(operation)
		{
		case NOT: //based on the first parent only
			if(valueCoords[1] == 0)
				prob[0] = FIX_ZERO;
			else
				prob[0] = FIX_ONE;
			break;
		case AND: //false except if all parents are true
			prob[0] = FIX_ZERO;
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] == 0) prob[0] = FIX_ONE;
			break;
		case NAND: //inverse of AND
			prob[0] = FIX_ONE;
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] == 0) prob[0] = FIX_ZERO;
			break;
		case OR: //true except if all parents are false
			prob[0] = FIX_ONE;
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] == numValues-1) prob[0] = FIX_ZERO;
			break;
		case NOR: //inverse of OR
			prob[0] = FIX_ZERO;
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] == numValues-1) prob[0] = FIX_ONE;
			break;
		case XOR: //true except if all parents are false or all parents are true
			prob[0] = FIX_ONE;
			//check if there is a mix of true/false in coordinates
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] != valueCoords[1]) prob[0] = FIX_ZERO;
			break;
		case XNOR: //false except if all parents are false or all parents are true
			prob[0] = FIX_ZERO;
			//check if there is a mix of true/false in coordinates
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] != valueCoords[1]) prob[0] = FIX_ONE;
			break;
		case IMP: //true except if first input is true and the rest are false
			prob[0] = FIX_ONE;
			//check if there is a mix of true/false in coordinates
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] != valueCoords[1]) prob[0] = FIX_ZERO;
			break;
		case CIMP: //false except if first input is true and the rest are false
			prob[0] = FIX_ZERO;
			//check if there is a mix of true/false in coordinates
			for(parnum = 1; parnum < (numParents+1); parnum++)
				if(valueCoords[parnum] != valueCoords[1]) prob[0] = FIX_ONE;
			break;
		default: //Identity, based on the first parent only
			if(valueCoords[1] == 0)
				prob[0] = FIX_ONE;
			else
				prob[0] = FIX_ZERO;
		}

		// Set the conditional random variable probability of a Bernoulli RV, P(true) = 1-P(false)
		prob[numValues-1] = FIX_ONE - prob[0];
		setBNodeProbVector(nodenum, prob, valueCoords);

		// Print current set of "coordinates", the variable indices of the parents
		#ifdef DEBUG
		char name[NAME_LENGTH];
		getBNodeName(nodenum, name);
		printf("LOGIC: %s @(:", name);
		for(parnum = 1; parnum < numParents + 1; parnum++)
			printf(",%3d", valueCoords[parnum]);
		printf(") = {%1.3fF %1.3fT}\n", FIX_TO_DOUBLE(prob[0]), FIX_TO_DOUBLE(prob[1]));
		#endif

		// Go to next low-order coordinate for the parent nodes
		valueCoords[1]++;

		// Treat each dimension counter as a digit of a number with a radix of the associated maximum value
		for(parnum = 1; parnum < (numParents + 1); parnum++) {
			// If we are past the number of values that this random variable has then increment the next one
			if(valueCoords[parnum] >= valueMaxes[parnum]) {
				valueCoords[parnum] = 0;
				valueCoords[parnum+1]++;
			}
		} // End counter increment loop
	} // End matrix dimension loop

	return 0;
}

unsigned int makeLogicTree(enum logic operation, unsigned int rootNodeNum, char *rootName, fix *dist, unsigned int numInputs, unsigned int numLevels)
{
	int inputNum;
	const unsigned int numValues = 2; //Bernoulli RV with two values: P(true) and P(false)
	char nodeName[NAME_LENGTH] = {'\0'};

	// Leaf node (distribution only) test for recursion
	if(numLevels <= 0) return rootNodeNum;

	// Create a tree of nodes, each with a given number of inputs at which are subtrees
	for(inputNum = 0; inputNum < numInputs; inputNum++)
	{
		// Create node at the next level
		sprintf(nodeName, "%s_%d%d", rootName, numLevels, inputNum);
		unsigned int currentNode = addBNodeParent(nodeName, dist, numValues, rootNodeNum);
		// Recursively build next level depth-first
		makeLogicTree(operation, currentNode, nodeName, dist, numInputs, numLevels-1);

	}

	// If not a leaf node then implement a logic operation
	// NOTE: this must be done AFTER the parents have been added to the current node
	makeLogic(operation, rootNodeNum);

	return rootNodeNum;
}

unsigned int setLogic(unsigned int nodenum, fix probtrue)
{
	fix prob[2] = {FIX_ONE-probtrue, probtrue};

	// If the node has parents we don't want to change it's conditional distribution
	// However, if the node has no parents then if we don't change the conditional distribution it will be reverted to the conditional distribution when inference is done
	if(getBNodeNumParents(nodenum) == 0)
		setBNodeProbVectorCoords(nodenum, prob, 0);

	setBNodeInfVector(nodenum, prob);

	return nodenum;
}
