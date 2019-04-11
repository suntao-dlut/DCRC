#ifndef KEOGH_H
#define KEOGH_H
#include "DCRC.h"

class KEOGH
{
    public:
        KEOGH();
        virtual ~KEOGH();
        int nDim = 0;
        Range ranges[MAX_DIM];
        Range qRanges[MAX_DIM];
		void getRangesFromSeries(TimeSeries * tms, float lambda, Range* rngs);
		float calculate(TimeSeries * tms, Range * rngs);
		void calculate(TimeSeries ** tmsSet, int nSet, float lambda);
		void setMBR(Range *mbr, int iDim);
        float query(TimeSeries *tms, float lambda);
    protected:
    private:
};

#endif // KEOGH_H
