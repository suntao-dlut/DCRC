#ifndef TIMESERIES_H
#define TIMESERIES_H
#define MAX_DIM 20
#define MAX_SIZE (1024 * 1024)
#define MAX_NODE_COUNT (64 * 1024)

class TimeSeries
{
    public:
        int length = 0;
        int dataId = -1;
        int classId = -1;
        int index = -1;
		float data[MAX_DIM] = { 0 };
        TimeSeries dimTrans(int newLen);
        TimeSeries compress_AS(int newLen);
        void print(float scale);
        TimeSeries();
        virtual ~TimeSeries();
    protected:
    private:
};

#endif // TIMESERIES_H
