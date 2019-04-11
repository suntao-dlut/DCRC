#include "KEOGH.h"
#include "malloc.h"
#include "math.h"
#include "stdio.h"
#include "RTree.h"
#include "debugleak.h"
#include "assert.h"
KEOGH::KEOGH()
{
    //ctor
}

KEOGH::~KEOGH()
{
}
void KEOGH::getRangesFromSeries(TimeSeries * tms, float lambda, Range* rngs) {
	nDim = tms->length;
	assert(nDim <= MAX_DIM);
	int r = (int)(lambda * nDim);
	int n = nDim;
	for (int i = 0; i < nDim; i++) {
		rngs[i].flag = false;
		rngs[i].rMax = -INFINITY;
		rngs[i].rMin = INFINITY;
	}
	for (int j = 0; j < nDim; j++) {
		if (RTree::mode == 0) {
			int k0, k1;
			k0 = j - r;
			if (k0 < 0) k0 = 0;
			k1 = j + r;
			if (k1 > nDim) k1 = nDim;
			for (int k = k0; k <= k1; k++) {
				float x = tms->data[k];
				if (x < rngs[j].rMin) rngs[j].rMin = x;
				if (x > rngs[j].rMax) rngs[j].rMax = x;
			}
		}
		else if (RTree::mode == 1) {
			int p = j + 1;
			int k0, k1;
			if ((p > 0) && (p <= r + 1)) {
				k0 = 1;
				k1 = p;
				for (int k = k0; k <= k1; k++) {
					if (tms->data[k - 1] < rngs[j].rMin) rngs[j].rMin = tms->data[k - 1];
					if (tms->data[k - 1] > rngs[j].rMax) rngs[j].rMax = tms->data[k - 1];
				}
			}
			if ((p > r + 1) && (p <= n / 2)) {
				k0 = p - r;
				k1 = p;
				for (int k = k0; k <= k1; k++) {
					if (tms->data[k - 1] < rngs[j].rMin) rngs[j].rMin = tms->data[k - 1];
					if (tms->data[k - 1] > rngs[j].rMax) rngs[j].rMax = tms->data[k - 1];
				}
			}
			if ((p > n / 2) && (p <= n - r - 1)) {
				k0 = p;
				k1 = p + r;
				for (int k = k0; k <= k1; k++) {
					if (tms->data[k - 1] < rngs[j].rMin) rngs[j].rMin = tms->data[k - 1];
					if (tms->data[k - 1] > rngs[j].rMax) rngs[j].rMax = tms->data[k - 1];
				}
			}
			if ((p > n - r - 1) && (p <= n)) {
				k0 = p;
				k1 = n;
				for (int k = k0; k <= k1; k++) {
					if (tms->data[k - 1] < rngs[j].rMin) rngs[j].rMin = tms->data[k - 1];
					if (tms->data[k - 1] > rngs[j].rMax) rngs[j].rMax = tms->data[k - 1];
				}
			}
		}
	}
}


float KEOGH::calculate(TimeSeries * tms, Range * rngs) {
	float sum = 0.0;
	int n = tms->length;
	for (int i = 0; i < n; i++) {
		float v0 = rngs[i].rMin;
		float v1 = rngs[i].rMax;
		float delta = 0;
		float x = tms->data[i];
		if (x < v0) delta = x - v0;
		if (x > v1) delta = x - v1;
		delta *= delta;
		sum += delta;
	}
	return (float)sqrt(sum);

}


void KEOGH::calculate(TimeSeries ** tmsSet, int nSet, float lambda) {
    TimeSeries *first = tmsSet[0];
    nDim = first->length;
	assert(nDim <= MAX_DIM);
    int r = (int)(lambda * nDim);
    int n = nDim;
    for (int i = 0; i < nDim; i ++) {
        ranges[i].flag = false;
        ranges[i].rMax = -INFINITY;
        ranges[i].rMin = INFINITY;
    }
    for (int i = 0; i < nSet; i ++) {
		TimeSeries* tms = tmsSet[i];
		for (int j = 0; j < nDim; j ++) {
			if (RTree::mode == 0) {
				int k0, k1;
				k0 = j - r;
				if (k0 < 0) k0 = 0;
				k1 = j + r;
				if (k1 > nDim) k1 = nDim;
				for (int k = k0; k <= k1; k++) {
					float x = tms->data[k];
					if (x < ranges[j].rMin) ranges[j].rMin = x;
					if (x > ranges[j].rMax) ranges[j].rMax = x;
				}
			}
			else if (RTree::mode == 1) {
				int p = j + 1;
                int k0, k1;
                if ((p > 0) && (p <= r + 1)) {
                    k0 = 1;
                    k1 = p;
                    for (int k = k0; k <= k1; k ++) {
                        if (tms -> data[k - 1] < ranges[j].rMin)
                            ranges[j].rMin = tms -> data[k - 1];
                        if (tms -> data[k - 1] > ranges[j].rMax)
                            ranges[j].rMax = tms -> data[k - 1];
                    }
                }
                if ((p > r + 1) && (p <= n/2)) {
                    k0 = p - r;
                    k1 = p;
                    for (int k = k0; k <= k1; k ++) {
                        if (tms -> data[k - 1] < ranges[j].rMin)
                            ranges[j].rMin = tms -> data[k - 1];
                        if (tms -> data[k - 1] > ranges[j].rMax)
                            ranges[j].rMax = tms -> data[k - 1];
                    }
                }
                if ((p > n/2) && ( p <= n - r -1)) {
                    k0 = p;
                    k1 = p + r;
                    for (int k = k0; k <= k1; k ++) {
                        if (tms -> data[k - 1] < ranges[j].rMin)
                            ranges[j].rMin = tms -> data[k - 1];
                        if (tms -> data[k - 1] > ranges[j].rMax)
                            ranges[j].rMax = tms -> data[k - 1];
                    }
                }
                if ((p > n - r -1) && (p <= n)) {
                    k0 = p;
                    k1 = n;
                    for (int k = k0; k <= k1; k ++) {
                        if (tms -> data[k - 1] < ranges[j].rMin)
                            ranges[j].rMin = tms -> data[k - 1];
                        if (tms -> data[k - 1] > ranges[j].rMax)
                            ranges[j].rMax = tms -> data[k - 1];
                    }
                }
            }
        }
    }
}

void  KEOGH::setMBR(Range *mbr, int iDim) {
    nDim = iDim;
    for (int i = 0; i < nDim; i ++) {
        ranges[i] = mbr[i];
    }
}


float KEOGH::query(TimeSeries *tms, float lambda) {
    for (int i = 0; i < nDim; i ++) {
        qRanges[i].flag = false;
        qRanges[i].rMax = -INFINITY;
        qRanges[i].rMin = INFINITY;
    }
    int r = (int)(lambda * nDim);
    int n = nDim;
    for (int i = 1; i <= nDim; i ++) {
        float x = tms -> data[i - 1];
        int p = i;
        int k0 = 1;
        int k1 = n;
        if (RTree::mode == 0) {
            k0 = p - r;
            k1 = p + r;
            if (k0 < 1) k0 = 1;
            if (k1 > n) k1 = n;
            for (int j = k0; j <= k1; j ++) {
                if (x < qRanges[j - 1].rMin) qRanges[j - 1].rMin = x;
                if (x > qRanges[j - 1].rMax) qRanges[j - 1].rMax = x;
            }
        }
        else {
            if ((p > 0) && (p <= r + 1)) {
                k0 = 1;
                k1 = p;
                for (int j = k0; j <= k1; j ++) {
                    if (x < qRanges[j - 1].rMin) qRanges[j - 1].rMin = x;
                    if (x > qRanges[j - 1].rMax) qRanges[j - 1].rMax = x;
                }
            }
            if ((p > r + 1) && (p <= n/2)) {
                k0 = p - r;
                k1 = p;
                for (int j = k0; j <= k1; j ++) {
                    if (x < qRanges[j - 1].rMin) qRanges[j - 1].rMin = x;
                    if (x > qRanges[j - 1].rMax) qRanges[j - 1].rMax = x;
                }
            }
            if ((p > n/2) && ( p <= n - r -1)) {
                k0 = p;
                k1 = p + r;
                for (int j = k0; j <= k1; j ++) {
                    if (x < qRanges[j - 1].rMin) qRanges[j - 1].rMin = x;
                    if (x > qRanges[j - 1].rMax) qRanges[j - 1].rMax = x;
                }
            }
            if ((p > n - r -1) && (p <= n)) {
                k0 = p;
                k1 = n;
                for (int j = k0; j <= k1; j ++) {
                    if (x < qRanges[j - 1].rMin) qRanges[j - 1].rMin = x;
                    if (x > qRanges[j - 1].rMax) qRanges[j - 1].rMax = x;
                }
            }
         }
    }

    float sum = 0.0;
    for (int i = 0; i < nDim; i ++) {
        float u0 = qRanges[i].rMin;
        float u1 = qRanges[i].rMax;
        float v0 = ranges[i].rMin;
        float v1 = ranges[i].rMax;
        float delta = 0;
        if (u0 > v1) delta = u0 - v1;
        if (v0 > u1) delta = v0 - u1;
        delta *= delta;
        sum += delta;
    }
    return (float)sqrt(sum);
}
