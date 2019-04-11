#ifndef DTREENODE_H
#define DTREENODE_H
#include "stdio.h"
#include <stdlib.h> 
#include "DCRC.h"
#include "TimeSeries.h"

#define MAX_LEVEL 4 //version 1.1
#define M_d 10
#define M_s 10
#define DIM_STEP 4
#define NUMBER_WEIGHT 0.005
class DTreeNode
{
    public:
		DTreeNode();
		DTreeNode(int iLong, int iShort);
		void init(int iLong, int iShort);
		bool debugFlag = false;
        int nLong, nShort;
        int index;
		int level = 0; //version 1.1
        bool hasLeaves = false;
        virtual ~DTreeNode();
		
		DTreeNode* root = 0;
        DTreeNode* parent = 0;
        DCRC data;
        DTreeNode * childNodes[M_d];
        TimeSeries * childSeries[M_d];
        int nodeNumber = 0;
        int tmsNumber = 0;
		int totalNumber = 0;

		void addSeries2File(TimeSeries* tms);
		void setReference(TimeSeries *tms);
        DTreeNode* addSeries(TimeSeries *tms);
        DTreeNode* addNode(DTreeNode *node);
        DTreeNode * splitSeries();
        DTreeNode * splitNodes();
        void reset();
        void updateNodes();
        void print();
    protected:
    private:
};

#endif // DTREENODE_H
