package com.thomsonreuters.upa.examples.consumerperf;

public class ConsumerStats
{
	volatile long updateCount;
	volatile double latencyAvg;
	volatile double latencyRunningSum;
	volatile long latencyCount;
	double latencyMin;
	double latencyMax;
	volatile long totalMsgs;
	double memoryAvg;
	long memoryCount;
	double cpuAvg;
	long cpuCount;
}
