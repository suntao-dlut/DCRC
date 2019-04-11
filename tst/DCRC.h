#ifndef DCRC_H
#define DCRC_H
#include "TimeSeries.h"
typedef struct {
    int flag;
    float rMin;
    float rMax;
} Range;

class DCRC
{
    public:
		DCRC();
        DCRC(int iLong, int iShort);
        virtual ~DCRC();
        bool debugFlag = false;
        char debugInfo[200];

        int index = -1;
        int nLong = 0, nShort;
        int isEmpty = 1;
        Range shortRanges[MAX_DIM][MAX_DIM];
        Range shortMBR[MAX_DIM];
        TimeSeries shortCenter, shortReference;
        void calculate(TimeSeries ** tmsSet, int iSet);


        void init(int iLong, int iShort);
        void reset();
        void setReference(TimeSeries * tms);
        void addSeries(TimeSeries * tms);
        void addDCRC(DCRC * dcrc);
        float testVolInc(TimeSeries * tms);
        float testVolume(TimeSeries * tms);
        float testVolume(DCRC *dcrc);

        float query(TimeSeries *tms, float lambda);
        void print();
    protected:
    private:
        inline  float getValue(int ip, int jp, int kp);
        inline float getVolume(Range * rects, int iDim);
};
#endif // DCRC_H
