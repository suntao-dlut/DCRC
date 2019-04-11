#include "RTreeNode.h"
#include "malloc.h"
#include "math.h"
#include "stdio.h"
#include "RTree.h"
#include "debugleak.h"
#define min(x,y) ((x)<(y))? (x): (y)
#define max(x,y) ((x)>(y))? (x): (y)

RTreeNode::RTreeNode() {

}
void RTreeNode::init(int dim) {
	nDim = dim;
	reset();
}

RTreeNode::RTreeNode(int dim)
{
	init(dim);
}

RTreeNode::~RTreeNode()
{
}
float RTreeNode::testVolume(RTreeNode *node) {
    return test2Volume(this, node);
}

float RTreeNode::test2Volume(RTreeNode *node1, RTreeNode *node2) {
    float vol = 1.0;
    Range rect[MAX_DIM];
    for (int i = 0; i < nDim; i ++) {
        rect[i].rMin = min(node1 -> mbr[i].rMin, node2 -> mbr[i].rMin);
        rect[i].rMax = max(node1 -> mbr[i].rMax, node2 -> mbr[i].rMax);
    }
    for (int i = 0; i < nDim; i ++) {
        vol *= (float)pow(rect[i].rMax - rect[i].rMin, 1.0/nDim);
    }
    return vol;
}

float RTreeNode::testVolInc(RTreeNode *node) {
    float v0 = 1.0;
    for (int i = 0; i < nDim; i ++) {
        v0 *= (float)pow(mbr[i].rMax - mbr[i].rMin, 1.0/nDim);
    }
    float v1 = 1.0;
    Range rect[MAX_DIM];
    for (int i = 0; i < nDim; i ++) {
        rect[i].rMin = min(mbr[i].rMin, node -> mbr[i].rMin);
        rect[i].rMax = max(mbr[i].rMax, node -> mbr[i].rMax);
    }
    for (int i = 0; i < nDim; i ++) {
        v1 *= (float)pow(rect[i].rMax - rect[i].rMin, 1.0/nDim);
    }
    return v1 - v0;
}

void RTreeNode::reset() {
    for (int i = 0; i < nDim; i ++) {
        mbr[i].flag = 0;
        mbr[i].rMin = INFINITY;
        mbr[i].rMax = -INFINITY;
    }
    nodeNumber = 0;
}

RTreeNode* RTreeNode::addNode(RTreeNode *node) {
    if (nodeNumber == M_r) return 0;
    for (int i = 0; i < nDim; i ++) {
        mbr[i].rMin = min(mbr[i].rMin, node -> mbr[i].rMin);
        mbr[i].rMax = max(mbr[i].rMax, node -> mbr[i].rMax);
    }
    childNodes[nodeNumber ++] = node;
    node -> parent = this;
    return (nodeNumber == M_r)? splitNodes() : 0;
}

typedef struct {
    RTreeNode * node;
    float volume;
} NodeVolume;

RTreeNode * RTreeNode::splitNodes(){
    float maxVol = - INFINITY;
    int arg1 = -1;
    int arg2 = -1;

    for (int i = 1; i < M_r; i ++) {
        for (int j = 0; j < i; j ++) {
            float tmpVol = test2Volume(childNodes[i], childNodes[j]);
            if (tmpVol > maxVol) {
                arg1 = j;
                arg2 = i;
            }
        }
    }
    if (arg1 == arg2) {
        arg1 = 0;
        arg2 = M_r - 1;
    }
    NodeVolume * nvList = (NodeVolume*)malloc(sizeof(NodeVolume) * (M_r));
    for (int i = 0; i < M_r; i ++) {
        nvList[i].node = childNodes[i];
    }

    RTreeNode * node1 = childNodes[arg1];
    RTreeNode * node2 = childNodes[arg2];
    this -> reset();
    this -> addNode(node1);
    RTreeNode *brother = RTree::getNewNode(nDim);
    brother -> addNode(node2);
    for (int i = 0; i < M_r; i ++) {
        float volume0 = this -> testVolume(nvList[i].node);
        float volume1 = brother -> testVolume(nvList[i].node);
        nvList[i].volume = volume0 - volume1;
    }


    for (int i = 0; i < M_r - 1; i ++) {
        int k = -1;
        float maxDiff = INFINITY;
        for (int j = i; j < M_r; j ++) {
            float tmpDiff = (float)fabs(nvList[j].volume);
            if (tmpDiff > maxDiff) {
                k = j;
                maxDiff = tmpDiff;
            }
        }
        if (k > i) {
            NodeVolume t = nvList[i];
            nvList[i] = nvList[k];
            nvList[k] = t;
        }
    }

    for (int i = 0; i < M_r; i ++) {
        NodeVolume nv = nvList[i];
        if ((nv.node == node1) || (nv.node == node2)) continue;
        if (this -> nodeNumber == M_r / 2) {
            brother -> addNode(nv.node);
        }
        else if (brother -> nodeNumber == M_r / 2) {
            this -> addNode(nv.node);
        }
        else {
            if (nv.volume < 0) this -> addNode(nv.node);
            else brother -> addNode(nv.node);
        }
    }
    return brother;
}

void RTreeNode::updateNodes() {
    for (int i = 0; i < nDim; i ++) {
        mbr[i].flag = 0;
        mbr[i].rMin = INFINITY;
        mbr[i].rMax = -INFINITY;
        for (int j = 0; j < nodeNumber; j ++) {
            mbr[i].rMin = min(mbr[i].rMin, childNodes[j] -> mbr[i].rMin);
            mbr[i].rMax = max(mbr[i].rMax, childNodes[j] -> mbr[i].rMax);
        }
    }
}
void RTreeNode::print() {
    int pIndex = (parent)? parent -> index : -1;
    printf("\n%s%d:\t%d\t[ ",  (hasLeaves? "*" : " "), index, pIndex);
    for (int i = 0; i < nodeNumber; i ++) {
        printf("%d ", childNodes[i] -> index);
    }
    printf("] ");
}

