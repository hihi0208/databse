#ifndef CRUD_OPERATIONS_H
#define CRUD_OPERATIONS_H

#include "database.h"

// BTree Index Operations
BTreeIndex* createBTreeIndex(int keyColumn, RowSet* rs);
void insertBTreeIndex(BTreeIndex* index, int key, RowSet* value);
RowSet* searchBTreeIndex(BTreeIndex* index, int key);
void freeBTreeIndex(BTreeIndex* index);

// RBTree Index Operations
RBTreeIndex* createRBTreeIndex();
void insertRBTreeIndex(RBTreeIndex* index, int key, RowSet* value);
RowSet* searchRBTreeIndex(RBTreeIndex* index, int key);
void freeRBTreeIndex(RBTreeIndex* index);

// CRUD Operations
RowSet* selectAll(RowSet* rs);
RowSet* selectLimit(RowSet* rs, int limit);
RowSet* selectProject(RowSet* rs, int* columnIndexes, int numColumns);
RowSet* selectFilterWhere(RowSet* rs, int columnIndex, const char* value);
RowSet* selectSort(RowSet* rs, int columnIndex, int ascending);
RowSet* aggregateSum(RowSet* rs, int columnIndex);
RowSet* aggregateGroupBy(RowSet* rs, int groupByColumnIndex, int aggregateColumnIndex, const char* aggregateFunction);

// Indexed CRUD Operations
RowSet* selectFilterWhereBTreeIndex(RowSet* rs, int columnIndex, const char* value, BTreeIndex* btreeIndex);
RowSet* selectFilterWhereRBTreeIndex(RowSet* rs, int columnIndex, const char* value, RBTreeIndex* rbtreeIndex);

#endif // CRUD_OPERATIONS_H