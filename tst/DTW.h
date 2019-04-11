#ifndef DTW_H
#define DTW_H
#include "TimeSeries.h"
typedef struct {
    int a, b;
    float v;
} Matching;
class DTW
{
    public:
        DTW();
        virtual ~DTW();
        TimeSeries * tms1=0;
        TimeSeries * tms2=0;
        int m = 0, n = 0, pathLen = 0;
        bool debugFlag = false;
        Matching matrix[MAX_DIM+1][MAX_DIM + 1];
        Matching path[2*MAX_DIM];
        float distance;
        float calculate(TimeSeries *ts1, TimeSeries *ts2, int mode, float lambda);
        void printPath();
        void printSeriesByPath();
    protected:
    private:
        inline float getValue(int mode, int ia, int ja, int radius);
};

#endif // DTW_H
