#ifndef DQUEUE_H
#define DQUEUE_H
#include "DTreeNode.h"
#include "RTreeNode.h"
class DQueue
{
    public:
        DQueue();
        virtual ~DQueue();
        int head = 0;
        int tail = 0;
        DTreeNode* data[MAX_SIZE];
        void push(DTreeNode *node);
        DTreeNode * pop();
    protected:
    private:
};
class RQueue
{
    public:
        RQueue();
        virtual ~RQueue();
        int head = 0;
        int tail = 0;
        RTreeNode * data[MAX_SIZE];
        void push(RTreeNode *node);
        RTreeNode * pop();
    protected:
    private:
};

#endif // DQUEUE_H
