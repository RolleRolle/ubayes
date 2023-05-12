#include "../umath/fixmath.h"

#define MAX_NODES	256
#define NAME_LENGTH	32
#define MAX_PARENTS	16
#define BERR	UINT_MAX

struct BNode
{
	// The name of this node
	char name[NAME_LENGTH];

	// Number of states in this node alone
	unsigned int numVals;

	// Number of nodes that affect this one
	int numParents;

	// Total Size of distribution = distSize*parent1_size*parent2_size*...
	unsigned int distSize;

	// Memory allocated to distribution, in bytes, as a sanity check (!)
	unsigned long distMem;

	// Names of distribution values
	char *valNames[NAME_LENGTH];

	// Nodes that affect this one
	unsigned int parent[MAX_PARENTS];

	// Function pointer for representing continuous distributions
	fix (*distFunction) (fix argument);

	// Conditional (Inferred) Probability Distribution array stating the probability of each value given parents
	fix *conditionals;

	// Probability Distribution array indexed as n-dimensional matrix
	fix *distributions;
};

unsigned int addBNode(const char *name, fix *dist, int numVals);
unsigned int addBNodeParent(const char *name, fix *dist, int numVals, unsigned int node);
//unsigned int AddBFunction
unsigned int addBParent(unsigned int nodenum, unsigned int parent);
//unsigned int freeBNode(unsigned int nodenum); //Do not use until dependency cleanup implemented
unsigned int freeBNetwork(void);

unsigned int findBNode(const char *name);
unsigned int inferBNode(unsigned int nodenum);
unsigned int inferBNetwork(unsigned int nodenum);
void printBNetwork(void);

unsigned int setBNodeInfVector(unsigned int nodenum, fix *prob);
unsigned int setBNodeProbElement(unsigned int nodenum, fix prob, unsigned int *valueCoords);
unsigned int setBNodeProbElementCoords(unsigned int nodenum, fix prob, ...);
unsigned int setBNodeProbVector(unsigned int nodenum, fix *prob, unsigned int *valueCoords);
unsigned int setBNodeProbVectorCoords(unsigned int nodenum, fix *prob, ...);

unsigned int getBNodeName(unsigned int nodenum, char *name);
unsigned int getBNodeParents(unsigned int nodenum, unsigned int *parents);
unsigned int getBNodeParentNumValues(unsigned int nodenum, unsigned int *valueMaxes);
unsigned int getIndex(unsigned int dimensions, unsigned int *dimCoords, unsigned int *maxCoords);
unsigned int getBNodeNumParents(unsigned int nodenum);
unsigned int getBNodeNumValues(unsigned int nodenum);
unsigned int getBNodeSize(unsigned int nodenum);
unsigned int getBNodeMem(unsigned int nodenum);
fix getBNodeProbElement(unsigned int nodenum, unsigned int *valueCoords);
fix getBNodeProbElementCoords(unsigned int nodenum, ...);
unsigned int getBNodeProbVector(unsigned int nodenum, fix *prob, unsigned int *valueCoords);
unsigned int getBNodeProbVectorCoords(unsigned int nodenum, fix *prob, ...);
unsigned int getBNodeProbText(unsigned int nodenum, char *textbuffer);
unsigned int getBNodeDistText(unsigned int nodenum, char *textbuffer);
fix getBNodeInfElement(unsigned int nodenum, unsigned int valuenum);
unsigned int getBNodeInfVector(unsigned int nodenum, fix *prob);
