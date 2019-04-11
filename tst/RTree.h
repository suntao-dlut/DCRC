#ifndef RTREE_H
#define RTREE_H
#include "TimeSeries.h"
#include "RTreeNode.h"
#include "DQueue.h"
class RTree
{
    public:
        static int mode;
        RTree(int iDim);
        virtual ~RTree();
        RTreeNode* root = 0;
        static RTreeNode* nodeList[MAX_SIZE];
        static int nodePoint;
		static RTreeNode* getNewNode(int dim);
        int nDim;
        int nData;
        TimeSeries* resultList[MAX_SIZE];
        int resultCount = 0;

        void insertSeries(TimeSeries *tms, float lambda);
        void print();
		RQueue queue;
        void query(TimeSeries *tms, float lambda, float error, int &n1, int &n2);
    protected:
    private:
};

#endif // RTREE_H
