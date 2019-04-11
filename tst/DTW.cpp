#include "DTW.h"
#include "TimeSeries.h"
#include "malloc.h"
#include "math.h"
#include "stdio.h"
#include "RTree.h"
#include "debugleak.h"
#include "assert.h"

DTW::DTW() {

	//ctor
}

DTW::~DTW() {

}
inline float DTW::getValue(int mode, int ia, int ja, int radius) {
	if ((ia == 0) && (ja == 0)) return 0.0;
	if ((ia == 0) || (ja == 0)) return INFINITY;

	if (abs(ia * n / m - ja) > radius)
		return INFINITY;
	return matrix[ia][ja].v;
}

float DTW::calculate(TimeSeries *ts1, TimeSeries *ts2, int mode, float lambda) {
	tms1 = ts1;
	tms2 = ts2;
	m = ts1->length;
	n = ts2->length;
	int radius = (int)(lambda * n + 0.1);

	for (int i = 1; i <= m; i++) {
		int j0, j1;

		j0 = i * n / m - radius;
		j1 = i * n / m + radius;

		if (j0 < 1) j0 = 1;
		if (j1 > n) j1 = n;


		for (int j = j0; j <= j1; j++) {
			float delta = ts1->data[i - 1] - ts2->data[j - 1];
			delta *= delta;
			float err1 = getValue(mode, i - 1, j - 1, radius) + delta;
			float err2 = getValue(mode, i - 1, j, radius) + delta;
			float err3 = getValue(mode, i, j - 1, radius) + delta;
			matrix[i][j].v = err1;
			matrix[i][j].a = i - 1;
			matrix[i][j].b = j - 1;
			if (err2 < matrix[i][j].v) {
				matrix[i][j].v = err2;
				matrix[i][j].a = i - 1;
				matrix[i][j].b = j;
			}
			if ((mode < 2) && (err3 < matrix[i][j].v)) {
				matrix[i][j].v = err3;
				matrix[i][j].a = i;
				matrix[i][j].b = j - 1;
			}
		}
	}
	pathLen = 0;

	Matching* mat = &matrix[m][n];
	int i = m, j = n;
	while (1) {
		assert(pathLen < m + n);
		path[pathLen].a = i - 1;
		path[pathLen].b = j - 1;
		path[pathLen].v = mat->v;
		pathLen++;
		i = mat->a;
		j = mat->b;
		if ((i == 0) && (j == 0)) break;
		assert((i > 0) && (j > 0)&&(i <= m) && (j <= n));
		mat = &matrix[i][j];
	}

	return (float)sqrt(path[0].v);
}

void DTW::printPath() {
	for (int i = 0; i < pathLen; i++) {
		Matching mat = path[i];
		printf("%d,%d\n", mat.a, mat.b);
	}
}
void DTW::printSeriesByPath() {
	TimeSeries temp;
	temp.length = tms1->length;
	for (int i = 0; i < pathLen; i++) {
		Matching mat = path[i];
		temp.data[mat.a] = tms2->data[mat.b];
	}
	tms1->print(1);
	temp.print(1);
}
