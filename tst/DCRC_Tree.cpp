#include "DCRC_Tree.h"
#include "DQueue.h"
#include "stdio.h"
#include "malloc.h"
#include "math.h"
#include <iostream>
#include <exception>
#include "debugleak.h"
#include "assert.h"

int DCRC_Tree::mode = 0;
DTreeNode** DCRC_Tree::nodeList = 0;
DTreeNode* DCRC_Tree::root = 0;
int DCRC_Tree::resNodeNum = 0;
int DCRC_Tree::resIndices[MAX_SIZE];

int DCRC_Tree::totalNodeNumber = 0;
int DCRC_Tree::totalLeafNumber = 0;
int DCRC_Tree::totalNumber = 0;

DCRC_Tree::DCRC_Tree(int iLong, int iShort)
{
    nLong = iLong;
    nShort = iShort;
	if (nodeList == 0) {
		nodeList = new DTreeNode*[MAX_NODE_COUNT];
		for (int i = 0; i < MAX_NODE_COUNT; i++)
			nodeList[i] = new DTreeNode();
	}
	printf("The nodeList is created successfully!\n");
	totalNodeNumber = 0;

	DCRC_Tree::root = getNewNode(iLong, iShort);
	DCRC_Tree::root -> hasLeaves = true;
	DCRC_Tree::root -> root = DCRC_Tree::root; // version 1.1
	totalLeafNumber = 0;
    nData = 0;
}

DCRC_Tree::~DCRC_Tree()
{
	for (int i = 0; i < MAX_NODE_COUNT; i++) {
		if (nodeList[i]) delete nodeList[i];
		nodeList[i] = 0;
	}
}
DTreeNode* DCRC_Tree::getNewNode(int iLong, int iShort) {
	assert(totalNodeNumber < MAX_NODE_COUNT);
	DTreeNode* node = nodeList[totalNodeNumber];
	node->init(iLong, iShort);
	node->index = node->data.index = totalNodeNumber;
	node->level = 0;
	node->root = DCRC_Tree::root;
	node->totalNumber = DCRC_Tree::totalNumber / (DCRC_Tree::totalNodeNumber + 1);
	totalNodeNumber++;
	return node;
}

double DCRC_Tree::testSeries(TimeSeries *tms) {
	double sum = 0;
	DTreeNode *current = DCRC_Tree::root;
	while (!current->hasLeaves) {
		float minInc = INFINITY;
		DTreeNode *argNode = 0;
		assert(current->nodeNumber <= M_d);
		for (int i = 0; i < current->nodeNumber; i++) {
			DTreeNode * tmpNode = current->childNodes[i];
			float tmpInc = (tmpNode->data).testVolInc(tms);
			if (tmpInc < minInc) {
				minInc = tmpInc;
				argNode = tmpNode;
			}
		}
		sum += minInc;
		if (argNode == 0) {
			return 0;
		}
		current = argNode;
	}
	return sum;
}

DTreeNode* DCRC_Tree::findSeries(TimeSeries *tms, bool updateFlag) {
	DTreeNode *current = DCRC_Tree::root;
	while (!current->hasLeaves) {
		float minInc = INFINITY;
		DTreeNode *argNode = 0;
		assert(current->nodeNumber <= M_d);
		for (int i = 0; i < current->nodeNumber; i++) {
			DTreeNode * tmpNode = current->childNodes[i];
			float v0 = (tmpNode->data).testVolInc(tms);
			float v1 = NUMBER_WEIGHT * tmpNode->totalNumber * DCRC_Tree::totalNodeNumber /(DCRC_Tree::totalNumber + 1);
			//printf("\n%d %f %f \n", ((v1 > v0)? 1 : 0), v0, v1);
			float tmpInc = v0 + v1;
			if (tmpInc < minInc) {
				minInc = tmpInc;
				argNode = tmpNode;
			}
		}
		if (argNode == 0) {
			return 0;
		}
		current = argNode;
		if (updateFlag) {
			current->totalNumber++;
			totalNumber++;
		}
	}
	return current;
}

void DCRC_Tree::insertSeries(TimeSeries *tms) {
	//printf((root->level >= MAX_LEVEL)? "-" : "+");
	//printf("[%d] ", root->level);
	DTreeNode *current = findSeries(tms, true);
	if (current == 0) {
		printf("No current!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n ");
		return;
	}

	nData++;
	assert(current->hasLeaves);

	DTreeNode *brother = current -> addSeries(tms);
    DTreeNode *parent = 0;

    while (1) {
        parent = current -> parent;
        if (parent) {
            if (brother)
                brother = parent -> addNode(brother);
            else
                parent -> updateNodes();
            current = parent;
        }
        else {
            if (brother) {
                int newShort = ((nShort >= DIM_STEP)? nShort - DIM_STEP : DIM_STEP);
				//int newShort = (nShort < 6)? nShort : nShort * 1 / 2;
				
				DCRC_Tree::root = DCRC_Tree::getNewNode(nLong, newShort);
				DCRC_Tree::root -> addNode(current);
				DCRC_Tree::root -> addNode(brother);
				current -> root = DCRC_Tree::root; //version 1.1
				brother -> root = DCRC_Tree::root; //version 1.1
				brother -> level = current -> level;   //version 1.1;
				DCRC_Tree::root -> level = current -> level + 1;  //version 1.1;
				printf("\nroot->level = %d\n", DCRC_Tree::root->level);
            }
            break;
        }
    }
}

void DCRC_Tree::query(TimeSeries *tms, float lambda, float error, int &n1, int &n2) {
    resultCount = 0;
	resNodeNum = 0;

    queue.push(DCRC_Tree::root);
    DTreeNode * node = 0;
    int nodeNum = 0;
    int leafNum = 0;
    while (node = queue.pop()) {
        float dist = (node -> data).query(tms, lambda);
        if (dist > error) continue;
        if (node -> hasLeaves) {
			nodeNum++;
			resIndices[resNodeNum++] = node->index;
            leafNum += node -> tmsNumber;
            for (int i = 0; i < node -> tmsNumber; i ++) {
                resultList[resultCount ++] = node -> childSeries[i];
            }
			//version 1.1
			//addResult(node -> index);
        }
        else {
            for (int i = 0; i < node -> nodeNumber; i ++) {
                DTreeNode *temp = node -> childNodes[i];
				queue.push(temp);
            }
        }
    }
    n1 = nodeNum;
    n2 = leafNum;
    printf("totalNumber = %d, nodeNum=%d, leafNum=%d\n", totalNodeNumber, nodeNum, leafNum);
}

void DCRC_Tree::print() {
    for (int i = 0; i < totalNodeNumber; i ++) {
        nodeList[i] -> print();
    }
}
