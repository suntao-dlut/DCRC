#include "RTree.h"
#include "malloc.h"
#include "KEOGH.h"
#include "DQueue.h"
#include "stdio.h"
#include "malloc.h"
#include "math.h"
#include "debugleak.h"
#include "assert.h"

int RTree::mode = 0;
RTreeNode* RTree::nodeList[MAX_SIZE];
int RTree::nodePoint;

RTree::RTree(int iDim)
{
    nDim = iDim;
	for (int i = 0; i < MAX_SIZE; i ++)
		nodeList[i] = new RTreeNode();
	root = getNewNode(iDim);
    root -> parent = 0;
    root -> hasLeaves = true;
    nData = 0;
}

RTree::~RTree()
{
    for (int i = 0; i < MAX_SIZE; i ++) {
		if (nodeList[i]) delete nodeList[i];
		nodeList[i] = 0;
    }
}
RTreeNode* RTree::getNewNode(int dim) {
	assert(nodePoint < MAX_SIZE);
	RTreeNode* node = nodeList[nodePoint];
	node->init(dim);
	node->index = nodePoint;
	nodePoint++;
	return node;
}

void RTree::insertSeries(TimeSeries *tms, float lambda) {
    RTreeNode *tmsNode = getNewNode(nDim);
    tmsNode -> index = tms -> index;
    int r = (int)(lambda * nDim);
    int n = nDim;
    for (int i = 0; i < nDim; i ++) {
        tmsNode -> mbr[i].flag = 0;
        tmsNode -> mbr[i].rMin = tms -> data[i];
        tmsNode -> mbr[i].rMax = tms -> data[i];
        if (RTree::mode == 1) {
            int p = i + 1;
            int j0, j1;
            if (p <= r + 1) {
                j0 = 1;
                j1 = p;
            }
            else if (p <= n / 2) {
                j0 = p - r;
                j1 = p;
            }
            else if (p <= n - r - 1) {
                j0 = p;
                j1 = p + r;
            }
            else {
                j0 = p;
                j1 = n;
            }
            if (j0 < 1) j0 = 1;
            if (j1 > n) j1 = n;
            for (int j = j0; j <= j1; j ++) {
                if (tms -> data[j - 1] < tmsNode -> mbr[i].rMin)
                    tmsNode -> mbr[i].rMin = tms -> data[j - 1];
                if (tms -> data[j - 1] > tmsNode -> mbr[i].rMax)
                    tmsNode -> mbr[i].rMax = tms -> data[j - 1];
            }
        }
    }
    tmsNode -> tms = tms;
    nData ++;
    RTreeNode *current = root;
    while (! current -> hasLeaves) {
        float minInc = INFINITY;
        RTreeNode *argNode = 0;
        for (int i = 0; i < current -> nodeNumber; i ++) {
            RTreeNode * tmpNode = current -> childNodes[i];
            float tmpInc = tmpNode -> testVolInc(tmsNode);
            if (tmpInc < minInc) {
                minInc = tmpInc;
                argNode = tmpNode;
            }
        }
        if (argNode == 0) return;
        current = argNode;
    }
    RTreeNode *brother = current -> addNode(tmsNode);
    RTreeNode *parent = 0;

    while (1) {
        parent = current -> parent;
        if (parent) {
            if (brother) {
				assert(nodePoint < MAX_SIZE);
                brother -> hasLeaves = current -> hasLeaves;
                brother = parent -> addNode(brother);
            }
            else
                parent -> updateNodes();
            current = parent;
        }
        else {
            if (brother) {
                brother -> hasLeaves = current -> hasLeaves;
                root = getNewNode(nDim);
                root -> addNode(current);
                root -> addNode(brother);
            }
            break;
        }
    }
}
void RTree::print() {
    for (int i = 0; i < nodePoint; i ++) {
        nodeList[i] -> print();
    }
}
void RTree::query(TimeSeries *tms, float lambda, float error, int &n1, int &n2) {
    resultCount = 0;
    KEOGH keogh;
    queue.push(root);
    RTreeNode * node = 0;
    int nodeNum = 0;
    int leafNum = 0;
    while (node = queue.pop()) {
        nodeNum ++;
        keogh.setMBR(node -> mbr, nDim);
        float dist = keogh.query(tms, lambda);
        if (dist > error) continue;

        if (node -> hasLeaves) {
            leafNum += node -> nodeNumber;
            for (int i = 0; i < node -> nodeNumber; i ++) {
                resultList[resultCount] = node -> childNodes[i] -> tms;
                resultCount ++;
            }
        }
        else {
            for (int i = 0; i < node -> nodeNumber; i ++) {
                RTreeNode *temp = node -> childNodes[i];
				queue.push(temp);
            }
        }
    }
    n1 = nodeNum;
    n2 = leafNum;
}

