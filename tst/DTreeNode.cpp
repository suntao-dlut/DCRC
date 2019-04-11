#include "DTreeNode.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"
#include "debugleak.h"
#include "assert.h"
#include "DCRC_Tree.h"
#include "Database.h"

DTreeNode::DTreeNode() {
	parent = 0;
	nLong = 0;
	nShort = 0;
}
void DTreeNode::init(int iLong, int iShort) {
	parent = 0;
	nLong = iLong;
	nShort = iShort;
	data.init(iLong, iShort);
}

DTreeNode::DTreeNode(int iLong, int iShort) {
	init(iLong, iShort);
}

DTreeNode::~DTreeNode() {
}

void DTreeNode::reset() {
    nodeNumber = 0;
    tmsNumber = 0;
    data.reset();
}

void DTreeNode::setReference(TimeSeries *tms) {
    data.setReference(tms);
}
void DTreeNode::addSeries2File(TimeSeries* tms) {
	char fileName[100] = { 0 };
	sprintf_s(fileName, "%s\\cluster\\%d.dat", DB_PATH, index);
	FILE* fp = fopen(fileName, "ab+");
	if (fp) {
		assert(tms->length == MAX_DIM);
		fwrite(tms->data, sizeof(float), tms->length, fp);
		fclose(fp);
	}
}


DTreeNode * DTreeNode::addSeries(TimeSeries *tms) {
	assert(hasLeaves);
	data.addSeries(tms);
	addSeries2File(tms);
	printf("%d ", this->index);

	//************************* version 1.1
	if (tmsNumber < M_s) {
		childSeries[tmsNumber] = tms;
		tmsNumber++;
	}
	
	if (root) {
		if ((root->level < MAX_LEVEL)) {
			DTreeNode* temp = (tmsNumber == M_s) ? splitSeries() : 0;
			return temp;
		}
		else {
			printf("-");
		}
	}
	
	return 0; //version 1.1
	//************************* version 1.1
}

DTreeNode * DTreeNode::addNode(DTreeNode *node) {
    if (nodeNumber == M_d) return 0;
	assert((nodeNumber >= 0) && (nodeNumber < M_d));
	assert(!hasLeaves);
	data.addDCRC(&(node -> data));
    childNodes[nodeNumber ++] = node;
    node -> parent = this;
    return (nodeNumber == M_d)? splitNodes() : 0;
}

typedef struct {
    TimeSeries * tms;
    float volume;
} SeriesVolume;

DTreeNode * DTreeNode::splitSeries() {
    float maxVol = - INFINITY;
    float minVol = INFINITY;
    int argMin = -1;
    int argMax = -1;

    for (int i = 0; i < M_s; i ++) {
        float tmpVol = data.testVolume(childSeries[i]);
        if (tmpVol < minVol) {
            argMin = i;
            minVol = tmpVol;
        }
        if (tmpVol > maxVol) {
            argMax = i;
            maxVol = tmpVol;
        }
    }
    if ((argMin == -1) || (argMax == -1)) return 0;
    if (argMin == argMax) {
        argMin = 0;
        argMax = M_s - 1;
    }

    SeriesVolume svList[M_s];
    for (int i = 0; i < M_s; i ++) {
        svList[i].tms = childSeries[i];
    }
    TimeSeries * tmsMax = childSeries[argMax];
    TimeSeries * tmsMin = childSeries[argMin];
    this -> reset();
    this -> addSeries(tmsMin);
	this->setReference(tmsMin);
	DTreeNode *node = DCRC_Tree::getNewNode(nLong, nShort);
	DCRC_Tree::totalLeafNumber++;
    node -> hasLeaves = true;
    //if (maxVol < MIN_VOL)
    //    node -> setReference(tmsMin);
	node->reset();
	node -> setReference(tmsMax);
    node -> addSeries(tmsMax);

    for (int i = 0; i < M_s; i ++) {
        float volume0 = (this -> data).testVolume(svList[i].tms);
        float volume1 = (node -> data).testVolume(svList[i].tms);
        svList[i].volume = volume0 - volume1;
    }

    for (int i = 0; i < M_s - 1; i ++) {//ÅÅÐò
        int k = -1;
        float maxDiff = -INFINITY;
        for (int j = i; j < M_s; j ++) {
            float tmpDiff = (float)fabs(svList[j].volume);
            if (tmpDiff > maxDiff) {
                k = j;
                maxDiff = tmpDiff;
            }
        }
        if (k > i) {
            SeriesVolume t = svList[i];
            svList[i] = svList[k];
            svList[k] = t;
        }
    }

    for (int i = 0; i < M_s; i ++) {
        SeriesVolume sv = svList[i];
        if ((sv.tms == tmsMin) || (sv.tms == tmsMax)) continue;
        if (this -> tmsNumber == M_s / 2) {
            node -> addSeries(sv.tms);
        }
        else if (node -> tmsNumber == M_s / 2) {
            this -> addSeries(sv.tms);
        }
        else {
            if (sv.volume < 0) this -> addSeries(sv.tms);
            else node -> addSeries(sv.tms);
        }
    }
    return node;
}

typedef struct {
    DTreeNode *node;
    float volume;
} NodeVolume;

DTreeNode * DTreeNode::splitNodes() {
	float maxVol = - INFINITY;
    float minVol = INFINITY;
    int argMin = -1;
    int argMax = -1;

    for (int i = 0; i < M_d; i ++) {
        float tmpVol = data.testVolume(&(childNodes[i] -> data));
        if (tmpVol < minVol) {
            argMin = i;
            minVol = tmpVol;
        }
        if (tmpVol > maxVol) {
            argMax = i;
            maxVol = tmpVol;
        }
    }
    if ((argMin == -1) || (argMax == -1))
        return 0;
    if (argMin == argMax) {
        argMin = 0;
        argMax = M_d - 1;
    }

    NodeVolume svList[M_d];
    for (int i = 0; i < M_d; i ++) {
        svList[i].node = childNodes[i];
    }

    DTreeNode * nodeMax = childNodes[argMax];
    DTreeNode * nodeMin = childNodes[argMin];
    this -> reset();
	this->setReference(&((nodeMin->data).shortReference));
	this -> addNode(nodeMin);
    DTreeNode *node = DCRC_Tree::getNewNode(nLong, nShort);
    //if (maxVol < MIN_VOL)
    node -> setReference(&((nodeMax -> data).shortReference));
    node -> addNode(nodeMax);

    for (int i = 0; i < M_d; i ++) {
        float volume0 = (this -> data).testVolume(&(svList[i].node->data));
        float volume1 = (node -> data).testVolume(&(svList[i].node->data));
        svList[i].volume = volume0 - volume1;
    }

    for (int i = 0; i < M_d - 1; i ++) {
        int k = -1;
        float maxDiff = INFINITY;
        for (int j = i; j < M_d; j ++) {
            float tmpDiff = (float)fabs(svList[j].volume);
            if (tmpDiff > maxDiff) {
                k = j;
                maxDiff = tmpDiff;
            }
        }
        if (k > i) {
            NodeVolume t = svList[i];
            svList[i] = svList[k];
            svList[k] = t;
        }
    }

    for (int i = 0; i < M_d; i ++) {
        NodeVolume nv = svList[i];
        if ((nv.node == nodeMin) || (nv.node == nodeMax)) continue;
        if (this -> nodeNumber == M_d / 2) {
            node -> addNode(nv.node);
        }
        else if (node -> nodeNumber == M_d / 2) {
            this -> addNode(nv.node);
        }
        else {
            if (nv.volume < 0) this -> addNode(nv.node);
            else node -> addNode(nv.node);
        }
    }

    return node;
}

void DTreeNode::updateNodes() {
    data.reset();
    for (int i = 0; i < nodeNumber; i ++) {
        data.addDCRC(&(childNodes[i] -> data));
    }
}

void DTreeNode::print() {
    int pIndex = (parent)? parent -> index : -1;
    printf("\n%s%d:\t%d\t[ ",  (hasLeaves? "*" : " "), index, pIndex);
    for (int i = 0; i < nodeNumber; i ++) {
        printf("%d ", childNodes[i] -> index);
    }
    printf("]\t[ ");
    for (int i = 0; i < tmsNumber; i ++) {
        printf("%d ", childSeries[i] -> index);
    }
    printf("]");
}
