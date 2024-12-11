#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CELL_VALUE 1024
#define MAX_KEYS 100  // 假设B树的最大键值数量

typedef enum {
    INT_TYPE,
    FLOAT_TYPE,
    STRING_TYPE
} DataType;

typedef struct {
    DataType type;
    char value[MAX_CELL_VALUE];
} Cell;

typedef struct Row {
    Cell* cells;
    int numCells;
    int capacity;
} Row;

typedef struct {
    Row* rows;
    int numRows;
    int capacity;
} RowSet;

// B树
typedef struct BTreeNode {
    int keyCount;
    int keys[MAX_KEYS];
    RowSet* values[MAX_KEYS];
    struct BTreeNode* children[MAX_KEYS + 1];  // 子节点指针
} BTreeNode;

typedef struct {
    BTreeNode* root;
} BTreeIndex;

typedef struct {
    RowSet* (*execute)(void*, RowSet*);
    void* data;
} Operator;

// 红黑树
typedef struct RBTreeNode {
    int key;
    RowSet* value;
    struct RBTreeNode* parent;
    struct RBTreeNode* left;
    struct RBTreeNode* right;
    char color;  // 'R' for red, 'B' for black
} RBTreeNode;

typedef struct {
    RBTreeNode* root;
} RBTreeIndex;

// 函数原型
void initRowSet(RowSet* rs, int initialCapacity);
void freeRowSet(RowSet* rs);
void addRow(RowSet* rs);
void addCell(RowSet* rs, int rowIndex, DataType type, const char* value);
void parseCSV(const char* filename, RowSet* rs);
void parseHeader(const char* filename, int** columnIndexes, int* numColumns);
BTreeNode* createNode();
void splitNode(BTreeNode* node, BTreeNode* rightNode, int key);
BTreeNode* insertNode(BTreeNode* node, int key, RowSet* value);
BTreeIndex* createBTreeIndex(int keyColumn, RowSet* rs);
void insertBTreeIndex(BTreeIndex* index, int key, RowSet* value);
RowSet* searchBTreeIndex(BTreeIndex* index, int key);
void freeBTreeIndex(BTreeIndex* index);
RBTreeNode* createRBTreeNode(int key, RowSet* value);
RBTreeIndex* createRBTreeIndex();
RBTreeNode* insertRBTreeNode(RBTreeNode* node, int key, RowSet* value);
void fixRedBlackTree(RBTreeNode* node, RBTreeNode* root);
RowSet* searchRBTreeNode(RBTreeNode* node, int key);
void freeRBTreeNode(RBTreeNode* node);
void leftRotate(RBTreeNode** root, RBTreeNode* x);
void rightRotate(RBTreeNode** root, RBTreeNode* y);
RowSet* limitOperator(void* limit, RowSet* rs);
RowSet* projectionOperator(void* projection, RowSet* rs);
RowSet* filterOperator(void* filter, RowSet* rs);
RowSet* sortOperator(void* sort, RowSet* rs);
RowSet* aggregationOperator(void* aggregation, RowSet* rs);

#endif // DATABASE_H