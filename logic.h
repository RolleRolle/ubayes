enum logic { ID, NOT, AND, NAND, OR, NOR, XOR, XNOR, IMP, CIMP };

unsigned int normalize(fix *dist, unsigned int size);
fix normal(double value, double stddev, double mean);
fix gaussian(double value, double stddev, double mean);
unsigned int makeGaussian(fix *dist, double stddev, double mean, unsigned int size);
unsigned int makeLogic(enum logic operation, unsigned int nodenum);
unsigned int makeLogicTree(enum logic operation, unsigned int rootNodeNum, char *rootName, fix *dist, unsigned int numInputs, unsigned int numLevels);
unsigned int setLogic(unsigned int nodenum, fix probtrue);
