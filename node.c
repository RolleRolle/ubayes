#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "node.h"

// Global node list
static struct BNode BNETWORK[MAX_NODES] = {{{0}}};

/********NON-CRITICAL FUNCTIONS FOR CONVENIENCE ********/

unsigned int addBNodeParent(const char *name, fix *dist, int numVals, unsigned int node)
{
	unsigned int parent = addBNode(name, dist, numVals);
	addBParent(node, parent);
	return parent;
}

unsigned int setBNodeInfVector(unsigned int nodenum, fix *prob)
{
	unsigned int valuenum;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Copy out conditionals vector
	for(valuenum = 0; valuenum < BNETWORK[nodenum].numVals; valuenum++)
		BNETWORK[nodenum].conditionals[valuenum] = prob[valuenum];

	// Return size of vector out
	return valuenum;
}

unsigned int inferBNetwork(unsigned int nodenum)
{
	// Traverse the entire network from above a given node, performing Bayesian inference starting at nodes with leaf nodes as parents
	unsigned int parnum;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Go through each parent of the current node sequentially
	for(parnum = 0; parnum < (BNETWORK[nodenum].numParents); parnum++)
	{
		// If this node has parents, recurse to them and perform inference on them first
		inferBNetwork(BNETWORK[nodenum].parent[parnum]);
	} // End parent recursion loop

	// After all parents have had inference performed and have updated distributions, perform inference on this node
	inferBNode(nodenum);

	return parnum;
}

/********CRITICAL NETWORK CONSTRUCTION/DESTRUCTION********/

unsigned int addBNode(const char *name, fix *dist, int numVals)
{
	unsigned int nodenum = 0, valueCoords;

	// Find first available node number
	while(BNETWORK[nodenum].distributions != NULL)
		if(++nodenum >= MAX_NODES)
			return BERR;

	// Allocate memory for local probabilities
	BNETWORK[nodenum].distributions = calloc(numVals, sizeof(fix));
	if(BNETWORK[nodenum].distributions == NULL)
	{
		fprintf(stderr, "BNET: Could not allocate %lu bytes for new distribution \"%s\" at index %d!",
				numVals*sizeof(fix), name, nodenum);
		return BERR;
	}
	BNETWORK[nodenum].distMem = numVals*sizeof(fix);

	// Allocate memory for inferred probabilities
	BNETWORK[nodenum].conditionals = calloc(numVals, sizeof(fix));
	if(BNETWORK[nodenum].conditionals== NULL)
	{
		fprintf(stderr, "BNET: Could not allocate %lu bytes for new inferred vector \"%s\" at index %d!",
				numVals*sizeof(fix), name, nodenum);
		return BERR;
	}
	BNETWORK[nodenum].distMem = numVals*sizeof(fix);

	// Copy in state probabilities
	for(valueCoords = 0; valueCoords < numVals; valueCoords++)
	{
		BNETWORK[nodenum].distributions[valueCoords] = dist[valueCoords];
		BNETWORK[nodenum].conditionals[valueCoords] = dist[valueCoords];
	}

	// Set distribution sizes for local and complete distributions
	BNETWORK[nodenum].numVals = numVals;
	BNETWORK[nodenum].distSize = numVals;

	// Set name of node
	strncpy(BNETWORK[nodenum].name, name, NAME_LENGTH);

	#ifdef DEBUG
	printf("BNET: Added node %s as %d\n", name, nodenum);
	#endif // DEBUG

	return nodenum;
}

unsigned int freeBNode(unsigned int nodenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	#ifdef DEBUG
	printf("BNET: Freeing node %d\n", nodenum);
	#endif // DEBUG
	free(BNETWORK[nodenum].distributions);
	free(BNETWORK[nodenum].conditionals);
	BNETWORK[nodenum].distributions = NULL;
	BNETWORK[nodenum].conditionals = NULL;
	BNETWORK[nodenum].distMem = 0;
	memset(&BNETWORK[nodenum], 0, sizeof(struct BNode));

	// TODO: Clean up all the nodes depending on this one and compress node list to left,
	// which will require deallocation and a whole mess of other things...

	return nodenum;
}

unsigned int freeBNetwork(void)
{
	unsigned int nodenum;

	for(nodenum = 0; nodenum < MAX_NODES; nodenum++)
		if(BNETWORK[nodenum].distributions != NULL)
			freeBNode(nodenum);

	return 0;
}

unsigned int addBParent(unsigned int nodenum, unsigned int parent)
{
	unsigned int ind, indNew, newParent, newSize = BNETWORK[nodenum].numVals;
	fix *newDist;

	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
	{
		fprintf(stderr, "BNET: Could not allocate new node %d as maximum number of nodes is %d!", nodenum, MAX_NODES);
		return BERR;
	}

	// Add parent to node's list
	BNETWORK[nodenum].parent[BNETWORK[nodenum].numParents++] = parent;

	#ifdef DEBUG
	printf("BNET: Added parent %d (%s) to node %d (%s)\n", parent, BNETWORK[parent].name, nodenum, BNETWORK[nodenum].name);
	#endif // DEBUG

	// Calculate new size of joint distribution
	for(ind = 0; ind < BNETWORK[nodenum].numParents; ind++)
	{
		newParent = BNETWORK[nodenum].parent[ind];
		newSize *= BNETWORK[newParent].numVals;
		#ifdef DEBUG
		printf("BNET: Calculating new distribution size, total %d from %d\n", newSize, newParent);
		#endif // DEBUG
	}

	// Reallocate memory to allow a distribution for each of the parent's values
	newDist = realloc(BNETWORK[nodenum].distributions, newSize*sizeof(fix));
	if(newDist == NULL)
	{
		fprintf(stderr, "BNET: Could not reallocate %lu bytes to %lu bytes for new parent \"%s\" of node \"%s\" at index %d!",
				BNETWORK[nodenum].distMem, newSize*sizeof(fix), BNETWORK[parent].name, BNETWORK[nodenum].name, nodenum);

		// Remove new parent from list
		BNETWORK[nodenum].parent[BNETWORK[nodenum].numParents--] = 0;

		// Memory block for distribution should be still intact
		return BERR;
	}

	// Reallocation succeeded, update memory use tracking
	BNETWORK[nodenum].distributions = newDist;
	BNETWORK[nodenum].distMem = newSize*sizeof(fix);

	// Copy original state probabilities into each possible value of the new parent
	for(ind = 0; ind < BNETWORK[nodenum].distSize; ind++)
	{
		for(indNew = ind + BNETWORK[nodenum].distSize; indNew < newSize; indNew += BNETWORK[nodenum].distSize)
		{
			#ifdef DEBUG
			printf("BNET: Initializing new distribution to %f from index %d to index %d\n", FIX_TO_DOUBLE(BNETWORK[nodenum].distributions[indNew]), ind, indNew);
			#endif // DEBUG
			BNETWORK[nodenum].distributions[indNew] = BNETWORK[nodenum].distributions[ind];
		}
	}

	// Update total size of distribution
	BNETWORK[nodenum].distSize = newSize;

	return nodenum;
}

/********ACCESSOR METHODS********/

struct BNode *getBNetworkArrayBase(void)
{
	return BNETWORK;
}

unsigned int getIndex(unsigned int dimensions, unsigned int *dimCoords, unsigned int *maxCoords)
{
	unsigned int dim, coord, offset, index = 0;

	// \sum_{n=1}^{L+1} \left( m_n \prod_{l=1}^{n-1} M_l \right)
	for(dim = 0; dim < dimensions; dim++) {
		offset = 1;
		for(coord = 0; coord < dim; coord++) {
			offset *= maxCoords[coord];
		}
		index += dimCoords[dim] * offset;
	}
	return index;
}

unsigned int getBNodeParentNumValues(unsigned int nodenum, unsigned int *valueMaxes)
{
	unsigned int parnum;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Determine number of values in all parents
	valueMaxes[0] = BNETWORK[nodenum].numVals;
	for(parnum = 0; parnum < BNETWORK[nodenum].numParents; parnum++)
		valueMaxes[parnum+1] = BNETWORK[BNETWORK[nodenum].parent[parnum]].numVals;

	return parnum;
}

unsigned int getBNodeName(unsigned int nodenum, char *name)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Get name of node
	strcpy(name, BNETWORK[nodenum].name);
	return strlen(BNETWORK[nodenum].name);
}

unsigned int getBNodeParents(unsigned int nodenum, unsigned int *parents)
{
	unsigned int parnum;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Get parents of node
	for(parnum = 0; parnum < BNETWORK[nodenum].numParents; parnum++)
		parents[parnum] = BNETWORK[nodenum].parent[parnum];
	return BNETWORK[nodenum].numParents;
}

unsigned int getBNodeNumParents(unsigned int nodenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	return BNETWORK[nodenum].numParents;
}

unsigned int getBNodeNumValues(unsigned int nodenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	return BNETWORK[nodenum].numVals;
}

unsigned int getBNodeSize(unsigned int nodenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	return BNETWORK[nodenum].distSize;
}

unsigned int getBNodeMem(unsigned int nodenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	return BNETWORK[nodenum].distMem;
}

unsigned int findBNode(const char *name)
{
	unsigned int nodenum;

	for(nodenum = 0; nodenum < MAX_NODES; nodenum++)
		if(BNETWORK[nodenum].distributions != NULL)
		{
			if(strcmp(BNETWORK[nodenum].name,name) == 0)
			{
				return nodenum;
			}
		}
	return BERR;
}

unsigned int setBProb(unsigned int nodenum, fix prob, unsigned int *valueCoords, unsigned int *valueMaxes)
{
	// Get linear index that corresponds to desired variables
	unsigned int index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	if(index > BNETWORK[nodenum].distSize)
		return BERR;

	// Assign probability to distribution at defined set of values
	BNETWORK[nodenum].distributions[index] = prob;
	return index;
}

fix getBProb(unsigned int nodenum, unsigned int *valueCoords, unsigned int *valueMaxes)
{
	// Get linear index that corresponds to desired variables
	unsigned int index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	if(index > BNETWORK[nodenum].distSize)
		return BERR;

	// Assign probability to distribution at defined set of values
	return BNETWORK[nodenum].distributions[index];
}

unsigned int setBNodeProbElement(unsigned int nodenum, fix prob, unsigned int *valueCoords)
{
	// Assumed: length of valueCoords is always number of parents + 1 for local values
	unsigned int valueMaxes[BNETWORK[nodenum].numParents+1];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Set probability of node at desired index
	return setBProb(nodenum, prob, valueCoords, valueMaxes);
}

fix getBNodeProbElement(unsigned int nodenum, unsigned int *valueCoords)
{
	// Assumed: length of valueCoords is always number of parents + 1 for local values
	unsigned int valueMaxes[BNETWORK[nodenum].numParents+1];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Set probability of node at desired index
	return getBProb(nodenum, valueCoords, valueMaxes);
}

unsigned int setBNodeProbElementCoords(unsigned int nodenum, fix prob, ...)
{
	// Number of "coordinates" is always number of parents + 1 for local values
	unsigned int parnum, num = BNETWORK[nodenum].numParents+1;
	unsigned int valueCoords[num], valueMaxes[num];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Initialize variable arguments for local distribution and each parent
	va_list valist;
	va_start(valist, prob);

	// Record all the arguments given in valist to a vector
	for (parnum = 0; parnum < num; parnum++) {
		valueCoords[parnum] = va_arg(valist, unsigned int);
	}

	// Free allocated memory for valist
	va_end(valist);

	// Set probability of node at desired index
	return setBProb(nodenum, prob, valueCoords, valueMaxes);
}

fix getBNodeProbElementCoords(unsigned int nodenum, ...)
{
	// Number of "coordinates" is always number of parents + 1 for local values
	unsigned int parnum, num = BNETWORK[nodenum].numParents+1;
	unsigned int valueCoords[num], valueMaxes[num];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Initialize variable arguments for local distribution and each parent
	va_list valist;
	va_start(valist, nodenum);

	// Record all the arguments given in valist to a vector
	for (parnum = 0; parnum < num; parnum++) {
		valueCoords[parnum] = va_arg(valist, unsigned int);
	}

	// Free allocated memory for valist
	va_end(valist);

	// Set probability of node at desired index
	return getBProb(nodenum, valueCoords, valueMaxes);
}

unsigned int setBNodeProbVector(unsigned int nodenum, fix *prob, unsigned int *valueCoords)
{
	// Assumed: length of valueCoords is always number of parents + 1 for local values, but we set the whole local vector at once
	unsigned int parnum, valueMaxes[BNETWORK[nodenum].numParents+1], index;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Set distribution vector along local dimension at index of desired values
	index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	for(parnum = 0; parnum < BNETWORK[nodenum].numVals; parnum++)
		BNETWORK[nodenum].distributions[index+parnum] = prob[parnum];

	// Return size of vector out
	return parnum;
}

unsigned int getBNodeProbVector(unsigned int nodenum, fix *prob, unsigned int *valueCoords)
{
	// Assumed: length of valueCoords is always number of parents + 1 for local values, but we set the whole local vector at once
	unsigned int parnum, valueMaxes[BNETWORK[nodenum].numParents+1], index;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Copy out vector along local dimension at index of desired values
	index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	for(parnum = 0; parnum < BNETWORK[nodenum].numVals; parnum++)
		prob[parnum] = BNETWORK[nodenum].distributions[index+parnum];

	// Return size of vector out
	return parnum;
}

unsigned int setBNodeProbVectorCoords(unsigned int nodenum, fix *prob, ...)
{
	// Number of "coordinates" is always number of parents only, we set the whole local vector at once
	unsigned int parnum, num = BNETWORK[nodenum].numParents+1; unsigned int index;
	unsigned int valueCoords[num], valueMaxes[num];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Initialize variable arguments for local distribution and each parent
	va_list valist;
	va_start(valist, prob);

	// Record all the arguments given in valist to a vector
	for (parnum = 1; parnum < num; parnum++) {
		valueCoords[parnum] = va_arg(valist, unsigned int);
	}
	valueCoords[0] = 0; // For index of start of local vector

	// Free allocated memory for valist
	va_end(valist);

	// Set distribution vector along local dimension at index of desired values
	index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	for(parnum = 0; parnum < BNETWORK[nodenum].numVals; parnum++)
		BNETWORK[nodenum].distributions[index+parnum] = prob[parnum];

	// Return size of vector out
	return parnum;
}

unsigned int getBNodeProbVectorCoords(unsigned int nodenum, fix *prob, ...)
{
	// Number of "coordinates" is always number of parents only, we set the whole local vector at once
	unsigned int parnum, num = BNETWORK[nodenum].numParents+1; unsigned int index;
	unsigned int valueCoords[num], valueMaxes[num];
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Initialize variable arguments for local distribution and each parent
	va_list valist;
	va_start(valist, prob);

	// Record all the arguments given in valist to a vector
	for (parnum = 1; parnum < num; parnum++) {
		valueCoords[parnum] = va_arg(valist, unsigned int);
	}
	valueCoords[0] = 0; // For index of start of local vector

	// Free allocated memory for valist
	va_end(valist);

	// Copy out vector along local dimension at index of desired values
	index = getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes);
	for(parnum = 0; parnum < BNETWORK[nodenum].numVals; parnum++)
		prob[parnum] = BNETWORK[nodenum].distributions[index+parnum];

	// Return size of vector out
	return parnum;
}

fix getBNodeInfElement(unsigned int nodenum, unsigned int valuenum)
{
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Return element of conditionals vector
	return BNETWORK[nodenum].conditionals[valuenum];
}

unsigned int getBNodeInfVector(unsigned int nodenum, fix *prob)
{
	unsigned int valuenum;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;

	// Copy out conditionals vector
	for(valuenum = 0; valuenum < BNETWORK[nodenum].numVals; valuenum++)
		prob[valuenum] = BNETWORK[nodenum].conditionals[valuenum];

	// Return size of vector out
	return valuenum;
}

/********INFERENCE PROCESS********/

unsigned int inferBNode(unsigned int nodenum)
{
	// Perform Bayesian inference at a given node by multiplying each local distribution value with the corresponding values of its parents that result in that value
	unsigned int parnum, valueCoords[BNETWORK[nodenum].numParents+2], valueMaxes[BNETWORK[nodenum].numParents+2];
	// NOTE: for N-dimensional coordinates in valueCoords and getIndex, dimension 0 is always the local states of the node itself
	// Dimensions 1...N are indexed for the states of the node's N-1 parents, which are enumerated 0...N-1
	// We check the dimension value after these (N+1 or parents+2) for overflow to find when all states have been seen
	fix inferredValue;
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	memset(valueCoords, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	memset(valueMaxes, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Go through each value of the current node sequentially
	for(valueCoords[0] = 0; valueCoords[0] < BNETWORK[nodenum].numVals; valueCoords[0]++)
	{
		// Initialize inferred value to 0
		BNETWORK[nodenum].conditionals[valueCoords[0]] = FIX_ZERO;
		valueCoords[BNETWORK[nodenum].numParents+1] = 0; // Set overflow to 0 for consecutive values
		#ifdef DEBUG
		printf("BNET: Inference at node %d with %d parents, value number %d\n", nodenum, BNETWORK[nodenum].numParents, valueCoords[0]);
		#endif

		// Loop until we increment the top coordinate of the tensor
		while(valueCoords[BNETWORK[nodenum].numParents + 1] == 0)
		{
			// The probability of a given value occurring is the sum of all conditional probabilities of that value
			inferredValue = BNETWORK[nodenum].distributions[getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes)];
			#ifdef DEBUG
			printf(" %1.3f (%1.3f @ node %2d at [", FIX_TO_DOUBLE(BNETWORK[nodenum].distributions[getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes)]), FIX_TO_DOUBLE(inferredValue), nodenum);
			for(unsigned int i = 0; i < BNETWORK[nodenum].numParents+2; i++)
				printf("%3d,", valueCoords[i]);
			printf("])\n");
			#endif
			for(parnum = 0; parnum < (BNETWORK[nodenum].numParents); parnum++)
			{
				// Multiply node distribution value by each parent's inferred probability of the current value
				inferredValue = fixmult(inferredValue, BNETWORK[BNETWORK[nodenum].parent[parnum]].conditionals[valueCoords[parnum+1]]);
				#ifdef DEBUG
				printf("*%1.3f (%1.3f @ node %2d value %3d)\n", FIX_TO_DOUBLE(BNETWORK[BNETWORK[nodenum].parent[parnum]].conditionals[valueCoords[parnum+1]]), FIX_TO_DOUBLE(inferredValue), BNETWORK[nodenum].parent[parnum], valueCoords[parnum+1]);
				#endif // DEBUG
			}

			// Add product of node distribution value probability and parent probabilities for that value together
			BNETWORK[nodenum].conditionals[valueCoords[0]] += inferredValue;
			#ifdef DEBUG
			printf(" += %1.3f\n", FIX_TO_DOUBLE(BNETWORK[nodenum].conditionals[valueCoords[0]]));
			#endif // DEBUG

			// Go to next low-order coordinate for the parent nodes
			valueCoords[1]++;

			// Treat each dimension counter as a digit of a number with a radix of the associated maximum value
			for(parnum = 1; parnum < (BNETWORK[nodenum].numParents + 1); parnum++)
			{
				// If we are past the number of values that this random variable has then increment the next one
				if(valueCoords[parnum] >= valueMaxes[parnum]) {
					valueCoords[parnum] = 0;
					valueCoords[parnum+1]++;
				}
			} // End counter increment loop
		} // End matrix dimension loop
	} // End value iteration loop

	return 0;
}

/********PRINTING AND INFORMATION********/

unsigned int getBNodeProbText(unsigned int nodenum, char *textbuffer)
{
	unsigned int index = 0, valueCoords[BNETWORK[nodenum].numParents+2], valueMaxes[BNETWORK[nodenum].numParents+2];
	// NOTE: for N-dimensional coordinates in valueCoords and getIndex, dimension 0 is always the local states of the node itself
	// Dimensions 1...N are indexed for the states of the node's N-1 parents, which are enumerated 0...N-1
	// We check the dimension value after these (N+1 or parents+2) for overflow to find when all states have been seen
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	memset(valueCoords, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	memset(valueMaxes, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Print inferred values
	index += sprintf(&textbuffer[index], "%s\n~", BNETWORK[nodenum].name);
	for(valueCoords[0] = 0; valueCoords[0] < valueMaxes[0]; valueCoords[0]++)
		index += sprintf(&textbuffer[index], "%1.3f, ", FIX_TO_DOUBLE(BNETWORK[nodenum].conditionals[valueCoords[0]]));
	index += sprintf(&textbuffer[index], "\\n");

	return index;
}

unsigned int getBNodeDistText(unsigned int nodenum, char *textbuffer)
{
	unsigned int index = 0, parnum, valueCoords[BNETWORK[nodenum].numParents+2], valueMaxes[BNETWORK[nodenum].numParents+2];
	// NOTE: for N-dimensional coordinates in valueCoords and getIndex, dimension 0 is always the local states of the node itself
	// Dimensions 1...N are indexed for the states of the node's N-1 parents, which are enumerated 0...N-1
	// We check the dimension value after these (N+1 or parents+2) for overflow to find when all states have been seen
	if(nodenum > MAX_NODES || BNETWORK[nodenum].distributions == NULL)
		return BERR;
	memset(valueCoords, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	memset(valueMaxes, 0,  (BNETWORK[nodenum].numParents+2)*sizeof(unsigned int));
	getBNodeParentNumValues(nodenum, valueMaxes);

	// Go through each dimension of the matrix sequentially
	while(valueCoords[BNETWORK[nodenum].numParents + 1] == 0)
	{
		// Print current set of "coordinates", the variable indices of the parents
		index += sprintf(&textbuffer[index], "@(:");
		for(parnum = 1; parnum < (BNETWORK[nodenum].numParents + 1); parnum++)
			index += sprintf(&textbuffer[index], ",%3d", valueCoords[parnum]);
		index += sprintf(&textbuffer[index], ") = ");

		// Print values of local distribution given parent values
		for(valueCoords[0] = 0; valueCoords[0] < valueMaxes[0]; valueCoords[0]++)
		{
			index += sprintf(&textbuffer[index], "%1.3f", FIX_TO_DOUBLE(BNETWORK[nodenum].distributions[getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes)]));
			if(valueCoords[0] < valueMaxes[0] - 1) index += sprintf(&textbuffer[index], ", ");
		}
		index += sprintf(&textbuffer[index], "\\n");

			// Go to next low-order coordinate for the parent nodes
		valueCoords[1]++;

		// Treat each dimension counter as a digit of a number with a radix of the associated maximum value
		for(parnum = 1; parnum < (BNETWORK[nodenum].numParents + 1); parnum++) {
			// If we are past the number of values that this random variable has then increment the next one
			if(valueCoords[parnum] >= valueMaxes[parnum]) {
				valueCoords[parnum] = 0;
				valueCoords[parnum+1]++;
			}
		} // End counter increment loop
	} // End matrix dimension loop

	// Print inferred values
	index += sprintf(&textbuffer[index], "~");
	for(valueCoords[0] = 0; valueCoords[0] < valueMaxes[0]; valueCoords[0]++)
		index += sprintf(&textbuffer[index], "%1.3f, ", FIX_TO_DOUBLE(BNETWORK[nodenum].conditionals[valueCoords[0]]));
	index += sprintf(&textbuffer[index], "\\n");

	return index;
}

void printBNetwork(void)
{
	unsigned int nodenum, parnum, valueCoords[MAX_PARENTS+2] = {0}, valueMaxes[MAX_PARENTS+2] = {0};
	// NOTE: for N-dimensional coordinates in valueCoords and getIndex, dimension 0 is always the local states of the node itself
	// Dimensions 1...N are indexed for the states of the node's N-1 parents, which are enumerated 0...N-1
	// We check the dimension value after these (N+1 or parents+2) for overflow to find when all states have been seen

	// Print probability distribution, dimension 0 is the local distribution
	printf("\nBayesian Network:\n");
	for(nodenum = 0; nodenum < MAX_NODES; nodenum++)
		if(BNETWORK[nodenum].distributions != NULL)
		{
			// Print node information
			printf("NODE: %s, distribution size %d, values %d, parents %d:",
					BNETWORK[nodenum].name, BNETWORK[nodenum].distSize, BNETWORK[nodenum].numVals, BNETWORK[nodenum].numParents);
			for(parnum = 0; parnum < BNETWORK[nodenum].numParents; parnum++)
				printf("\t%s", BNETWORK[BNETWORK[nodenum].parent[parnum]].name);
			printf("\nDISTRIBUTION:\n");

			// Go through each dimension of the matrix sequentially
			memset(valueCoords, 0,  (MAX_PARENTS+2)*sizeof(unsigned int));
			memset(valueMaxes, 0,  (MAX_PARENTS+2)*sizeof(unsigned int));
			getBNodeParentNumValues(nodenum, valueMaxes);
			while(valueCoords[BNETWORK[nodenum].numParents + 1] == 0)
			{
				// Print current set of "coordinates", the variable indices of the parents
				printf("@(:");
				for(parnum = 1; parnum < (BNETWORK[nodenum].numParents + 1); parnum++)
					printf(",%3d", valueCoords[parnum]);
				printf(") = [");

				// Print values of local distribution given parent values
				for(valueCoords[0] = 0; valueCoords[0] < valueMaxes[0]; valueCoords[0]++)
				{
					printf("%1.3f", FIX_TO_DOUBLE(BNETWORK[nodenum].distributions[getIndex(BNETWORK[nodenum].numParents+1, valueCoords, valueMaxes)]));
					if(valueCoords[0] < valueMaxes[0] - 1) printf(", ");
				}
				printf("]\n");

				// Go to next low-order coordinate for the parent nodes
				valueCoords[1]++;

				// Treat each dimension counter as a digit of a number with a radix of the associated maximum value
				for(parnum = 1; parnum < (BNETWORK[nodenum].numParents + 1); parnum++) {
					// If we are past the number of values that this random variable has then increment the next one
					if(valueCoords[parnum] >= valueMaxes[parnum]) {
						valueCoords[parnum] = 0;
						valueCoords[parnum+1]++;
					}
				} // End counter increment loop
			} // End matrix dimension loop

			// Print inferred values
			printf("~[");
			for(valueCoords[0] = 0; valueCoords[0] < valueMaxes[0]; valueCoords[0]++)
				printf("%1.3f, ", FIX_TO_DOUBLE(BNETWORK[nodenum].conditionals[valueCoords[0]]));
			printf("]\n");
		} // End node selection loop

return;
}
