#include "stdio.h"
#include <stdlib.h> 
#include "string.h"
#include "Database.h"
#include "malloc.h"
#include "debugleak.h"
#include "assert.h"
using namespace std;

Database::Database()
{
	Dataset list[] = {
		{ "synthetic_control", 6, 60, 300, 300 },
		{ "Gun_Point", 2, 150, 50, 150 },
		{ "CBF", 3, 128, 30, 900 },
		{ "FaceAll", 14, 131, 560, 1690 },
		{ "OSULeaf", 6, 427, 200, 242 },
		{ "SwedishLeaf", 15, 128, 500, 625 },
		{ "50Words", 50, 270, 450, 455 },
		{ "Trace", 4, 275, 100, 100 },
		{ "Two_Patterns", 4, 128, 1000, 4000 },
		{ "Wafer", 2, 152, 1000, 6164 }, //6174!
		{ "FaceFour", 4, 350, 24, 88 },
		{ "Lighting2", 2, 637, 60, 61 },
		{ "Lighting7", 7, 319, 70, 73 },
		{ "ECG200", 2, 96, 100, 100 },
		{ "Adiac", 37, 176, 390, 391 },
		{ "Yoga", 2, 426, 300, 3000 },
		{ "Fish", 7, 463, 175, 175 },
		{ "Plane", 7, 144, 105, 105 },
		{ "Car", 4, 577, 60, 60 },
		{ "Beef", 5, 470, 30, 30 }
	};
	for (int i = 0; i < 20; i++) {
		datasets[i] = list[i];
	}
	nLong = 0;
}
void Database::setDim(int iLong) {
	nLong = iLong;
}

char *Database::readFloat(char *str, float *val) {
	char * p = str;
	while (1) {
		if (((*p) == ' ') || ((*p) == ',') || ((*p) == 0xa) || ((*p) == 0xd) || ((*p) == 0)) break;
		p++;
	}
	while (1) {
		if ((*p) == 0) break;
		if (((*p) == ' ') || ((*p) == ',') || ((*p) == 0xa) || ((*p) == 0xd)) {
			(*p) = 0;
			p++;
		}
		else
			break;
	}
	if (strcmp(str, "") == 0) {
		printf("error float!");
	}
	(*val) = (float)atof(str);
	return p;
}

void Database::buff2Series(float* buff, int len, TimeSeries *pts) {
	if (nLong <= len) {
		for (int i = 0; i < nLong; i++) {
			int j0 = i * len / nLong;
			int j1 = (i + 1) * len / nLong - 1;
			pts->data[i] = 0;
			for (int j = j0; j <= j1; j++) {
				pts->data[i] += buff[j];
			}
			pts->data[i] /= (j1 - j0 + 1);
		}
	}
	else {
		for (int i = 0; i < nLong; i++) {
			int j = i * len / nLong;
			if (j >= nLong - 1) j = len - 1;
			pts->data[i] = buff[j];
		}
	}
}


void Database::readByIndex(int index) {
	Dataset* dset = &datasets[index];
	char fileName[100] = { 0 };
	char row[10000];
	FILE *fp = 0;

	sprintf_s(fileName, "%s\\db\\%s\\%s_TEST", DB_PATH, dset->dataName, dset->dataName);
	printf("fileName = %s\n", fileName);
	fp = fopen(fileName, "rt");
	if (fp == 0) {
		printf("The file \"%s\" does not exist!\n", fileName);
		return;
	}

	dset->testData = (TimeSeries *)malloc(sizeof(TimeSeries) * dset->testCount);
	for (int i = 0; i < dset->testCount; i++) {
		if (feof(fp)) {
			printf("The file \"%s\" is error at row %d!\n", fileName, i);
			delete dset->testData;
			return;
		}
		fgets(row, 10000, fp);
		TimeSeries *pts = &(dset->testData[i]);
		char * p = row;
		float temp = 0;
		p = readFloat(p, &temp);
		pts->dataId = index;
		pts->classId = int(temp + 0.1);
		pts->length = nLong;

		float *buff = (float *)malloc(sizeof(float) * dset->seriesLength);
		for (int j = 0; j < dset->seriesLength; j++) {
			p = readFloat(p, &temp);
			buff[j] = temp;
		}
		buff2Series(buff, dset->seriesLength, pts);
		delete buff;
	}
	fclose(fp);

	sprintf_s(fileName, "%s\\db\\%s\\%s_TRAIN", DB_PATH, dset->dataName, dset->dataName);
	fp = fopen(fileName, "rt");
	if (fp == 0) {
		printf("The file \"%s\" does not exist!\n", fileName);
		return;
	}

	dset->trainData = (TimeSeries *)malloc(sizeof(TimeSeries) * dset->trainCount);
	for (int i = 0; i < dset->trainCount; i++) {
		if (feof(fp)) {
			printf("The file \"%s\" is error at row %d!\n", fileName, i);
			delete dset->trainData;
			return;
		}
		fgets(row, 10000, fp);
		TimeSeries *pts = &(dset->trainData[i]);
		char * p = row;
		float temp = 0;
		p = readFloat(p, &temp);
		pts->dataId = index;
		pts->classId = int(temp + 0.001);
		pts->length = nLong;

		float *buff = (float *)malloc(sizeof(float) * dset->seriesLength);
		for (int j = 0; j < dset->seriesLength; j++) {
			p = readFloat(p, &temp);
			buff[j] = temp;
		}
		buff2Series(buff, dset->seriesLength, pts);
		delete buff;
	}
	fclose(fp);
}

void Database::read() {
	assert(nLong > 0);
	for (int i = 0; i < 20; i++) {
		readByIndex(i);
	}
}

TimeSeries** Database::getInnerClass(int dataId, int classId, int dimension, int *nSet) {
	int cnt = 0;
	Dataset dSet = datasets[dataId];
	for (int i = 0; i < dSet.trainCount; i++) {
		TimeSeries *ts = &dSet.trainData[i];
		if (ts->classId == classId) cnt++;
	}
	if (cnt == 0) {
		(*nSet) = 0;
		return 0;
	}
	TimeSeries** ret = (TimeSeries **)malloc(sizeof(TimeSeries *) * cnt);
	int j = 0;
	for (int i = 0; i < dSet.trainCount; i++) {
		TimeSeries *ts = &dSet.trainData[i];
		if (ts->classId == classId) {
			ret[j] = ts;
			j++;
		}
	}
	(*nSet) = cnt;
	return ret;
}

TimeSeries** Database::getTestData(int dimension) {
	TimeSeries** ret = (TimeSeries **)malloc(sizeof(TimeSeries *) * TEST_COUNT);
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < (TEST_COUNT / 20); j++) {
			ret[i * 10 + j] = &datasets[i].testData[j];
		}
	}
	return ret;
}



Database::~Database()
{
	//dtor
}
