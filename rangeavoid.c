#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "node.h"
#include "graph.h"

int MakeGaussian(double *array, double scale, double stddev, double mean, int offset, int size)
{
	int counter;
    for(counter=0; counter<size; counter++)
	{
		array[counter] = (1/(stddev*sqrt(2*M_PI)))*exp(-pow(((double)(counter-offset)*scale/(double)size)-mean,2)/pow(2*stddev,2));
	}
	return counter;
}

#define blength 17

int main(int argc, char **argv)
{
	char bdot[32768]; unsigned int size, counter;
	fix fixarray[blength];
	fix fixarray_left[blength];
	fix fixarray_right[blength];
	unsigned int nn[6];
	double sensor_width = 30.0; //60 for half? and horizontal scale/2?
	int counter_offset = (round(sensor_width)*2-1)/blength;
	double left_sensor[] = {0.8, 0.2};
	double middle_sensor[] = {0.8, 0.2}; //{0.0, 1.35};
	double right_sensor[] = {0.8, 0.2};
	double fused_sensor[blength] = {0.0};
	double left_motor[] = {0.6, 0.4};
	double right_motor[] = {0.6, 0.4};
	double motor_bias[] = {0.5, 0.5};
	double bias_array[blength] = {0.9, 0.9, 0.9, 0.9, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4};

	printf("Byte Sizes: short=%ld int=%ld long=%ld fix=%ld fixrad=%ld BERR=%d\n",
		sizeof(short), sizeof(int), sizeof(long), sizeof(fix), sizeof(fixrad), BERR);

	MakeGaussian(fused_sensor, sensor_width, 3.0, 0.0, blength/2, blength);

	size = convertArray(fixarray, left_sensor, 0, sizeof(left_sensor)/sizeof(double));
	nn[0] = addBNode("LeftSensor", fixarray, size);
	size = convertArray(fixarray, middle_sensor, 0, sizeof(middle_sensor)/sizeof(double));
	nn[1] = addBNode("MiddleSensor", fixarray, size);
	size = convertArray(fixarray, right_sensor, 0, sizeof(right_sensor)/sizeof(double));
	nn[2] = addBNode("RightSensor", fixarray, size);
	size = convertArray(fixarray, fused_sensor, 0, sizeof(fused_sensor)/sizeof(double));
	nn[3] = addBNode("FusedSensor", fixarray, size);
	printArray(fixarray, sensor_width, size/2, size, "data/gaussian.csv");
	size = convertArray(fixarray, left_motor, 0, sizeof(left_motor)/sizeof(double));
	nn[4] = addBNode("LeftMotor", fixarray, size);
	size = convertArray(fixarray, right_motor, 0, sizeof(right_motor)/sizeof(double));
	nn[5] = addBNode("RightMotor", fixarray, size);

	addBParent(nn[3], nn[0]);
	addBParent(nn[3], nn[1]);
	addBParent(nn[3], nn[2]);
	addBParent(nn[4], nn[3]);
	addBParent(nn[5], nn[3]);

	//MakeGaussian(fused_sensor, sensor_width, 0.0, 0.0, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	//convertArray(fixarray, fused_sensor, 0, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 0, 0, 0);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, -sensor_offset, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, -counter_offset, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 1, 0, 0);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, -2*sensor_offset, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, -2*counter_offset, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 1, 1, 0);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, 0.0, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, 0, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 0, 1, 0);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, sensor_offset, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, counter_offset, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 0, 1, 1);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, 2*sensor_offset, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, 2*counter_offset, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 0, 0, 1);

	//MakeGaussian(fused_sensor, sensor_width, 0.0, 0.0, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, 0, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 1, 0, 1);

	//MakeGaussian(fused_sensor, sensor_width, 3.0, 0.0, blength/2, blength);
	memset(fixarray, FIX_ZERO, blength*sizeof(fix));
	convertArray(fixarray, fused_sensor, 0, sizeof(fused_sensor)/sizeof(double));
	setBNodeProbVectorCoords(nn[3], fixarray, 1, 1, 1);

	for(counter = 0; counter < blength; counter++)
	{
		motor_bias[0] = bias_array[counter]; motor_bias[1] = 1.0-motor_bias[0];
		convertArray(fixarray, motor_bias, 0, sizeof(motor_bias)/sizeof(double));
		setBNodeProbVectorCoords(nn[4], fixarray, counter);
		setBNodeProbVectorCoords(nn[5], fixarray, blength-counter-1);
	}

	inferBNode(nn[3]);

	size = getBNodeInfVector(nn[3], fixarray);
	//for(counter = 0; counter < floor(size/2); counter++)
	//	fixarray[size-1-counter] = fixarray[counter];
	//fixarray[8] += DOUBLE_TO_FIX(0.002);
	//setBNodeInfVector(nn[3], fixarray);
	printArray(fixarray, sensor_width, size/2, size, "data/inferred.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 0, 0, 0);
	printArray(fixarray, sensor_width, size/2, size, "data/none.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 1, 0, 0);
	printArray(fixarray, sensor_width, size/2, size, "data/left.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 1, 1, 0);
	printArray(fixarray, sensor_width, size/2, size, "data/leftmid.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 0, 1, 0);
	printArray(fixarray, sensor_width, size/2, size, "data/mid.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 0, 1, 1);
	printArray(fixarray, sensor_width, size/2, size, "data/rightmid.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 0, 0, 1);
	printArray(fixarray, sensor_width, size/2, size, "data/right.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 1, 0, 1);
	printArray(fixarray, sensor_width, size/2, size, "data/split.csv");

	size = getBNodeProbVectorCoords(nn[3], fixarray, 1, 1, 1);
	printArray(fixarray, sensor_width, size/2, size, "data/all.csv");

	inferBNode(nn[4]);
	inferBNode(nn[5]);

	size = getBNodeInfVector(nn[4], fixarray);
	normalizeArray(fixarray, 2);
	printValue(fixarray[0], sensor_width, size/2, blength, "data/left_inferred_forward.csv");
	printValue(fixarray[1], sensor_width, size/2, blength, "data/left_inferred_reverse.csv");
	size = getBNodeInfVector(nn[5], fixarray);
	normalizeArray(fixarray, 2);
	printValue(fixarray[0], sensor_width, size/2, blength, "data/right_inferred_forward.csv");
	printValue(fixarray[1], sensor_width, size/2, blength, "data/right_inferred_reverse.csv");

	for(counter = 0; counter < blength; counter++)
	{
		fixarray_left[counter] = getBNodeProbElementCoords(nn[4], 0, counter);
		fixarray_right[counter] = getBNodeProbElementCoords(nn[5], 0, counter);
	}
	printArray(fixarray_left, sensor_width, size/2, blength, "data/left_motor_forward.csv");
	printArray(fixarray_right, sensor_width, size/2, blength, "data/right_motor_forward.csv");
	for(counter = 0; counter < blength; counter++)
	{
		fixarray_left[counter] = getBNodeProbElementCoords(nn[4], 1, counter);
		fixarray_right[counter] = getBNodeProbElementCoords(nn[5], 1, counter);
	}
	printArray(fixarray_left, sensor_width, size/2, blength, "data/left_motor_reverse.csv");
	printArray(fixarray_right, sensor_width, size/2, blength, "data/right_motor_reverse.csv");

	printBNetwork();

	generateDotGraph(bdot, blength);
	ExportDotGraph(bdot, "graph.dot");
	DisplayDotGraph(bdot);

	freeBNetwork();

	return 0;
}
