#include "DQueue.h"
#include "malloc.h"
#include "stdio.h"
#include "debugleak.h"
DQueue::DQueue()
{
    head = tail = 0;
}

DQueue::~DQueue()
{
}
void DQueue::push(DTreeNode *node) {
    if ((head + 1) % MAX_SIZE == tail) return;
    data[head] = node;
    head = (head + 1) % MAX_SIZE;
}
DTreeNode * DQueue::pop() {
    if (head == tail) return 0;
    DTreeNode *node = data[tail];
    tail = (tail + 1) % MAX_SIZE;
    return node;
}

RQueue::RQueue()
{
    head = tail = 0;
}

RQueue::~RQueue()
{
}
void RQueue::push(RTreeNode *node) {
    if ((head + 1) % MAX_SIZE == tail) {
        printf("error: The queue is full! head = %d, tail = %d, nSize = %d\n", head, tail, MAX_SIZE);
        return;
    }
    data[head] = node;
    head = (head + 1) % MAX_SIZE;

}
RTreeNode * RQueue::pop() {
    if (head == tail) return 0;
    RTreeNode *node = data[tail];
    tail = (tail + 1) % MAX_SIZE;
    return node;
}
