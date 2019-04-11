#ifndef DATABASE_H
#define DATABASE_H
#include "TimeSeries.h"
#define DB_PATH "C:\\Users\\suntao\\Desktop\\DCRC_v1.1"
#define TEST_COUNT 200
typedef struct {
    char dataName[100];
    int classCount;
    int seriesLength;

    int trainCount;
    int testCount;

    TimeSeries *testData;
    TimeSeries *trainData;
} Dataset;

class Database
{
    public:
        Database();
        virtual ~Database();
		int nLong;
		void setDim(int iLong);
		void buff2Series(float* buff, int len, TimeSeries *pts);
		void read();
        TimeSeries** getInnerClass(int dataId, int classId, int dimension, int *nSet);
        TimeSeries** getTestData(int dimension);
        Dataset datasets[20];
    protected:
    private:
        void readByIndex(int index);
        char * readFloat(char * str, float *val);

};



#endif // DATABASE_H
