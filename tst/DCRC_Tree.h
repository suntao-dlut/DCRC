#ifndef DCRC_TREE_H
#define DCRC_TREE_H
#include "DTreeNode.h"
#include "TimeSeries.h"
#include "DQueue.h"
class DCRC_Tree
{
    public:
        static int mode;
		static DTreeNode** nodeList;
		static int totalNodeNumber;
		static int totalLeafNumber;
		static int totalNumber;

		static DTreeNode* getNewNode(int iLong, int iShort);

		bool debugFlag = false;
		DCRC_Tree(int iLong, int iShort);
		virtual ~DCRC_Tree();

        static DTreeNode* root;
		TimeSeries* resultList[MAX_SIZE];
		int resultCount = 0;

		static int resNodeNum;
		static int resIndices[MAX_SIZE];

        int nLong, nShort;
        int nData = 0;
		double testSeries(TimeSeries *tms);
		DTreeNode* findSeries(TimeSeries *tms, bool updateFlag);
        void insertSeries(TimeSeries *tms);
		void print();
		DQueue queue;
        void query(TimeSeries *tms, float lambda, float error, int &n1, int &n2);
    protected:
    private:
};

#endif // DCRC_TREE_H
