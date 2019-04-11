#include "DCRC.h"
#include "DCRC_Tree.h"
#include "malloc.h"
#include "dtw.h"
#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "debugleak.h"

#define min(x,y) ((x)<(y))? (x): (y)
#define max(x,y) ((x)>(y))? (x): (y)


float results[MAX_DIM][MAX_DIM][MAX_DIM];

DCRC::DCRC()
{
	nLong  = 0;
	nShort = 0;
}

DCRC::DCRC(int iLong, int iShort)
{
	init(iLong, iShort);
}

DCRC::~DCRC()
{
}


void DCRC::init(int iLong, int iShort) {
	nLong = iLong;
	nShort = iShort;
	shortCenter.length = nShort;
	shortCenter.dataId = -1;
	shortCenter.classId = -1;
	reset();
}
void DCRC::reset() {
	isEmpty = 1;
	for (int i = 0; i < nLong; i++) {
		for (int j = 0; j < nShort; j++) {
			shortRanges[i][j].flag = 0;
			shortRanges[i][j].rMin = INFINITY;
			shortRanges[i][j].rMax = -INFINITY;
		}
	}
	for (int i = 0; i < nShort; i++) {
		shortMBR[i].flag = 0;
		shortMBR[i].rMin = INFINITY;
		shortMBR[i].rMax = -INFINITY;
	}
	for (int i = 0; i < nLong; i++)
		for (int j = 0; j < nLong; j++)
			for (int k = 0; k < nShort; k++)
				results[i][j][k] = INFINITY;
}

void DCRC::setReference(TimeSeries * tms) {
	shortReference = tms->compress_AS(nShort);
	isEmpty = 0;
}


void DCRC::addSeries(TimeSeries * tms) {
	if (isEmpty) {
		shortReference = tms->compress_AS(nShort);
		isEmpty = 0;
	}
	DTW dtw;
	dtw.calculate(tms, &shortReference, 2, 1.0);

	assert(tms->length == nLong);
	assert(dtw.pathLen == nLong);
	for (int j = 0; j < dtw.pathLen; j++) {
		Matching *mat = &(dtw.path[j]);
		int a = mat->a;
		int b = mat->b;
		assert(a == nLong - j - 1);
		assert((b >= 0) && (b < nShort));
		float val = tms->data[a];
		shortRanges[a][b].flag = 1;
		shortRanges[a][b].rMin = min(val, shortRanges[a][b].rMin);
		shortRanges[a][b].rMax = max(val, shortRanges[a][b].rMax);
		shortMBR[b].rMin = min(val, shortMBR[b].rMin);
		shortMBR[b].rMax = max(val, shortMBR[b].rMax);
	}
	for (int i = 0; i < nShort; i++)
		shortCenter.data[i] = 0.5f * (shortMBR[i].rMin + shortMBR[i].rMax);
}

void DCRC::addDCRC(DCRC * dcrc) {
	if (isEmpty) {
		this->shortReference = (dcrc->shortCenter).compress_AS(nShort);
		isEmpty = 0;
	}
	DTW dtw;
	dtw.calculate(&(dcrc->shortCenter), &shortReference, 2, 1.0);
	assert(((dcrc->shortCenter).length == dcrc->nShort) && (dcrc->nShort == dtw.pathLen));
	int mapLS[MAX_DIM];
	for (int j = 0; j < dtw.pathLen; j++) {
		Matching *mat = &(dtw.path[j]);
		int a = mat->a;
		int b = mat->b;
		mapLS[a] = b;
	}
	assert(dcrc->nLong == this->nLong);
	for (int i = 0; i < dcrc->nLong; i++) {
		for (int j = 0; j < dcrc->nShort; j++) {
			if (dcrc->shortRanges[i][j].flag == 0) continue;
			int k = mapLS[j];
			float jMin = dcrc->shortRanges[i][j].rMin;
			float jMax = dcrc->shortRanges[i][j].rMax;
			this->shortRanges[i][k].flag = 1;
			this->shortRanges[i][k].rMin = min(jMin, this->shortRanges[i][k].rMin);
			this->shortRanges[i][k].rMax = max(jMax, this->shortRanges[i][k].rMax);
			this->shortMBR[k].rMin = min(jMin, this->shortMBR[k].rMin);
			this->shortMBR[k].rMax = max(jMax, this->shortMBR[k].rMax);
		}
	}
	for (int i = 0; i < nShort; i++)
		shortCenter.data[i] = 0.5f * (shortMBR[i].rMin + shortMBR[i].rMax);
}



inline float DCRC::getVolume(Range * rects, int iDim) {
	float ret = 1.0;
	for (int i = 0; i < iDim; i++) {
		//printf("%d~%f\n", i, (rects[i].rMax - rects[i].rMin));
		ret *= (float)pow((rects[i].rMax - rects[i].rMin), 1.0f / iDim);
	}
	return ret;
}


float DCRC::testVolInc(TimeSeries * tms) {
	Range testRects[MAX_DIM];
	for (int i = 0; i < nShort; i++)
		testRects[i] = shortMBR[i];
	float v0 = getVolume(testRects, nShort);
	DTW dtw;
	dtw.calculate(tms, &shortReference, 2, 1.0);
	for (int j = 0; j < dtw.pathLen; j++) {
		Matching *mat = &(dtw.path[j]);
		int a = mat->a;
		int b = mat->b;
		assert(a == tms->length - 1 - j);
		assert((b>=0)&&(b<nShort));
		float val = tms->data[a];
		if (val > testRects[b].rMax)
			testRects[b].rMax = val;
		if (val < testRects[b].rMin)
			testRects[b].rMin = val;
	}
	float v1 = getVolume(testRects, nShort);
	return v1 - v0;
}

float DCRC::testVolume(TimeSeries * tms) {
	Range testRects[MAX_DIM];
	for (int i = 0; i < nShort; i++) {
		testRects[i].flag = 0;
		testRects[i].rMax = shortReference.data[i];
		testRects[i].rMin = shortReference.data[i];
	}
	DTW dtw;
	dtw.calculate(tms, &shortReference, 2, 1.0);
	for (int j = 0; j < dtw.pathLen; j++) {
		Matching *mat = &(dtw.path[j]);
		int a = mat->a;
		int b = mat->b;
		assert(a == tms->length - 1 - j);
		assert((b >= 0) && (b<nShort));
		float val = tms->data[a];
		if (val > testRects[b].rMax)
			testRects[b].rMax = val;
		if (val < testRects[b].rMin)
			testRects[b].rMin = val;
	}
	return getVolume(testRects, nShort);
}

float DCRC::testVolume(DCRC *dcrc) {
	Range testRects[MAX_DIM];
	for (int i = 0; i < nShort; i++) {
		testRects[i].flag = 0;
		testRects[i].rMax = shortReference.data[i];
		testRects[i].rMin = shortReference.data[i];
	}
	DTW dtw;
	dtw.calculate(&(dcrc->shortCenter), &shortReference, 2, 1.0);
	assert((dcrc->shortCenter).length == dcrc->nShort);
	int mapLS[MAX_DIM];
	for (int j = 0; j < dtw.pathLen; j++) {
		Matching *mat = &(dtw.path[j]);
		int a = mat->a;
		int b = mat->b;
		assert(a == dcrc->nShort - 1 - j);
		assert((b >= 0) && (b < nShort));
		mapLS[a] = b;
	}
	assert(dcrc->nLong == nLong);
	for (int i = 0; i < dcrc->nLong; i++) {
		for (int j = 0; j < dcrc->nShort; j++) {
			if (dcrc->shortRanges[i][j].flag == 0) continue;
			int k = mapLS[j];
			float jMin = dcrc->shortRanges[i][j].rMin;
			float jMax = dcrc->shortRanges[i][j].rMax;
			testRects[k].rMin = min(jMin, testRects[k].rMin);
			testRects[k].rMax = max(jMax, testRects[k].rMax);
		}
	}
	return getVolume(testRects, nShort);
}

void DCRC::calculate(TimeSeries ** tmsSet, int iSet) {
	reset();
	for (int i = 0; i < iSet; i++) {
		addSeries(tmsSet[i]);
	}
}
inline float DCRC::getValue(int ip, int jp, int kp) {
	if ((ip == 0) && (jp == 0) && (kp == 0)) return 0.0;
	if ((ip == 0) || (jp == 0) || (kp == 0)) return INFINITY;
	assert((ip > 0) && (ip <= nLong) && (jp > 0) && (jp <= nLong) && (kp > 0) && (kp <= nShort));
	return results[ip - 1][jp - 1][kp - 1];
}

float DCRC::query(TimeSeries *tms, float lambda) {
	if (tms->length != nLong) return INFINITY;
	for (int j = 1; j <= nLong; j++) {
		for (int k = 1; k <= nShort; k++) {
			if (shortRanges[j - 1][k - 1].flag == 0) continue;
			int i0 = 1;
			int i1 = nLong;
			int radius = (int)(nLong * lambda);
			i0 = j - radius;
			i1 = j + radius;
			if (i0 < 1) i0 = 1;
			if (i1 > nLong) i1 = nLong;

			for (int i = i0; i <= i1; i++) {
				float x = tms->data[i - 1];
				float a = shortRanges[j - 1][k - 1].rMin;
				float b = shortRanges[j - 1][k - 1].rMax;
				float delta = 0.0;
				if (x < a) delta = x - a;
				if (x > b) delta = x - b;
				delta *= delta;
				float e1 = getValue(i - 1, j - 1, k);
				float e2 = getValue(i - 1, j - 1, k - 1);
				float e3 = getValue(i - 1, j, k);
				float e4 = getValue(i, j - 1, k);
				float e5 = getValue(i, j - 1, k - 1);
				float e = e1;
				if (e2 < e) e = e2;
				if (e3 < e) e = e3;
				if (e4 < e) e = e4;
				if (e5 < e) e = e5;
				assert((i > 0) && (i <= nLong) && (j > 0) && (j <= nLong) && (k > 0) && (k <= nShort));
				results[i - 1][j - 1][k - 1] = e + delta;
			}
		}
	}
	return (float)sqrt(results[nLong - 1][nLong - 1][nShort - 1]);
}

void DCRC::print() {
	printf("\n\nlong = %d, short = %d", nLong, nShort);
	for (int i = 0; i < nLong; i++) {
		printf("\n%d: ", i);
		for (int j = 0; j < nShort; j++) {
			if (shortRanges[i][j].flag == 1) {
				printf("[%d, %0.2f,%0.2f] ", j, shortRanges[i][j].rMin, shortRanges[i][j].rMax);
			}
		}
	}
}
