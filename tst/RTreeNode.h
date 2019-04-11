#ifndef RTREENODE_H
#define RTREENODE_H
#include "DCRC.h"
#include "DTreeNode.h"
#define M_r 10

class RTreeNode
{
    public:
		RTreeNode();
        RTreeNode(int dim);
		void init(int dim);
        virtual ~RTreeNode();
        int nDim = 0;
        int index = -1;
        RTreeNode *parent = 0;
        Range mbr[MAX_DIM];
        TimeSeries *tms = 0;
        RTreeNode* childNodes[M_r];
        int nodeNumber;
        void reset();
        RTreeNode* addNode(RTreeNode *node);
        RTreeNode* splitNodes();
        bool hasLeaves = false;
        float testVolume(RTreeNode *node);
        float test2Volume(RTreeNode *node1, RTreeNode *node2);
        float testVolInc(RTreeNode *node);
        void updateNodes();
        void print();
    protected:
    private:
};

#endif // RTREENODE_H
