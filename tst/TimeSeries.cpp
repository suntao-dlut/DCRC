#include "TimeSeries.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"
#include "debugleak.h"
#include <assert.h>

TimeSeries::TimeSeries()
{
}

TimeSeries::~TimeSeries()
{
}

TimeSeries TimeSeries::dimTrans(int newLen) {
    TimeSeries ret;
    ret.dataId = dataId;
    ret.classId = classId;
    ret.length = newLen;
    if (newLen <= length) {
        for (int i = 0; i < newLen; i ++) {
            int j0 = i * length/ newLen;
            int j1 = (i + 1) * length / newLen - 1;
            ret.data[i] = 0;
            for (int j = j0; j <= j1; j ++) {
                ret.data[i] += data[j];
            }
            ret.data[i] /= (j1 - j0 + 1);
        }
    }
    else {
        for (int i = 0; i < newLen; i ++) {
            int j = i * length / newLen;
            if (j >= length - 1) j = length - 1;
            ret.data[i] = data[j];
        }
    }

    return ret;
}

TimeSeries TimeSeries::compress_AS(int newLen) {
    if (length <= newLen)
        return dimTrans(newLen);
    TimeSeries ret;

	try {
		ret.dataId = dataId;
		ret.classId = classId;
		ret.length = newLen;

		float avg[MAX_DIM][MAX_DIM], err[MAX_DIM][MAX_DIM], val[MAX_DIM][MAX_DIM];
		int idx[MAX_DIM][MAX_DIM];


		for (int i = 1; i <= length; i++) {
			for (int j = 1; j <= i; j++) {
				float sum = 0.0;
				for (int k = j; k <= i; k++) {
					sum += data[k - 1];
				}
				float tmpAvg = sum / (i - j + 1);
				avg[j - 1][i - 1] = tmpAvg;
				sum = 0.0;
				for (int k = j; k <= i; k++) {
					sum += (data[k - 1] - tmpAvg) * (data[k - 1] - tmpAvg);
				}
				err[j - 1][i - 1] = sum;
			}
		}
		for (int i = 1; i <= length; i++) {
			for (int j = 1; j <= newLen; j++) {
				if (j > i) continue;
				int t = -1;
				val[j - 1][i - 1] = 0.0;
				idx[j - 1][i - 1] = -1;
				float minErr = INFINITY;
				for (int k = j; k <= i; k++) {
					float err0 = 0.0;
					float err1 = err[k - 1][i - 1];
					if (k - 1 == 0) {
						err0 = 0.0;
					}
					else if (j - 1 == 0) {
						err0 = INFINITY;
					}
					else {
						err0 = val[j - 2][k - 2];
					}
					float err2 = err0 + err1;
					if (err2 < minErr) {
						minErr = err2;
						t = k;
					}
				}
				if (t == -1) continue;
				val[j - 1][i - 1] = minErr;
				idx[j - 1][i - 1] = t;
			}
		}
		int i = length;
		for (int j = newLen; j >= 1; j--) {
			int p = i;
			assert(i - 1 >= 0);
			i = idx[j - 1][i - 1];
			assert(p - 1 >= 0);
			ret.data[j - 1] = avg[i - 1][p - 1];
			i--;
		}
	}
	catch (...) {
		printf("Exception in compress_AS! \n");
	}
	
    return ret;

}


void TimeSeries::print(float scale) {
    printf("\nlength = %d\n", length);
    for (int i = 0; i < (int)(length * scale); i ++) {
        int j = (int)(i/scale);
        if (j >= length - 1) j = length - 1;
        printf("%f\n", data[j]);
    }
    printf("\n");
}
