// crud_operations.c
#include "crud_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全表读取
RowSet* selectAll(RowSet* rs) {
    return rs;  // 直接返回原始 RowSet
}

// 分页语句(LIMIT)
RowSet* selectLimit(RowSet* rs, int limit) {
    if (rs->numRows <= limit) {
        return rs;  // 数据行数小于等于限制数，直接返回
    }
    RowSet* limitedRs = malloc(sizeof(RowSet));
    limitedRs->rows = malloc(sizeof(Row) * limit);
    limitedRs->numRows = limit;
    limitedRs->capacity = limit;
    for (int i = 0; i < limit; i++) {
        limitedRs->rows[i] = rs->rows[i];
    }
    return limitedRs;  // 返回限制后的 RowSet
}

// 投影操作(PROJECTION)
RowSet* selectProject(RowSet* rs, int* columnIndexes, int numColumns) {
    RowSet* projectedRs = malloc(sizeof(RowSet));
    initRowSet(projectedRs, rs->numRows);
    for (int i = 0; i < rs->numRows; i++) {
        addRow(projectedRs);
        for (int j = 0; j < numColumns; j++) {
            int columnIndex = columnIndexes[j];
            Cell cell = rs->rows[i].cells[columnIndex];
            addCell(projectedRs, projectedRs->numRows - 1, cell.type, cell.value);
        }
    }
    return projectedRs;
}

// 条件选择语句(WHERE)
RowSet* selectFilterWhere(RowSet* rs, int columnIndex, const char* value) {
    RowSet* filteredRs = malloc(sizeof(RowSet));
    initRowSet(filteredRs, 0);
    for (int i = 0; i < rs->numRows; i++) {
        if (strcmp(rs->rows[i].cells[columnIndex].value, value) == 0) {
            addRow(filteredRs);
            for (int j = 0; j < rs->rows[i].numCells; j++) {
                Cell cell = rs->rows[i].cells[j];
                addCell(filteredRs, filteredRs->numRows - 1, cell.type, cell.value);
            }
        }
    }
    return filteredRs;
}

// 排序操作(ORDER BY)
RowSet* selectSort(RowSet* rs, int columnIndex, int ascending) {
    // 简单的冒泡排序实现
    for (int i = 0; i < rs->numRows - 1; i++) {
        for (int j = 0; j < rs->numRows - i - 1; j++) {
            if ((ascending && strcmp(rs->rows[j].cells[columnIndex].value, rs->rows[j + 1].cells[columnIndex].value) > 0) ||
                (!ascending && strcmp(rs->rows[j].cells[columnIndex].value, rs->rows[j + 1].cells[columnIndex].value) < 0)) {
                Row temp = rs->rows[j];
                rs->rows[j] = rs->rows[j + 1];
                rs->rows[j + 1] = temp;
            }
        }
    }
    return rs;  // 返回排序后的 RowSet
}

// 聚合操作(AGGREGATION)
RowSet* aggregateSum(RowSet* rs, int columnIndex) {
    RowSet* sumRs = malloc(sizeof(RowSet));
    initRowSet(sumRs, 1);
    int sum = 0;
    for (int i = 0; i < rs->numRows; i++) {
        sum += atoi(rs->rows[i].cells[columnIndex].value);
    }
    addRow(sumRs);
    Cell sumCell;
    sumCell.type = INT_TYPE;
    sprintf(sumCell.value, "%d", sum);
    addCell(sumRs, 0, sumCell.type, sumCell.value);
    return sumRs;
}

RowSet* aggregateGroupBy(RowSet* rs, int groupByColumnIndex, int aggregateColumnIndex, const char* aggregateFunction) {
    if (strcmp(aggregateFunction, "COUNT") != 0) {
        return NULL;  // 只支持 COUNT 聚合函数
    }
    RowSet* groupByRs = malloc(sizeof(RowSet));
    initRowSet(groupByRs, 0);
    char* groups[MAX_CELL_VALUE] = {0};
    int counts[MAX_CELL_VALUE] = {0};
    for (int i = 0; i < rs->numRows; i++) {
        char* groupKey = rs->rows[i].cells[groupByColumnIndex].value;
        int groupIndex = -1;
        for (int j = 0; j < MAX_CELL_VALUE; j++) {
            if (groups[j] && strcmp(groups[j], groupKey) == 0) {
                groupIndex = j;
                break;
            }
        }
        if (groupIndex == -1) {
            groupIndex = strlen(groups) / sizeof(char*);
            groups[groupIndex] = strdup(groupKey);
        }
        counts[groupIndex]++;
    }
    for (int i = 0; i < MAX_CELL_VALUE; i++) {
        if (groups[i]) {
            addRow(groupByRs);
            Cell groupCell, countCell;
            groupCell.type = STRING_TYPE;
            strcpy(groupCell.value, groups[i]);
            addCell(groupByRs, groupByRs->numRows - 1, groupCell.type, groupCell.value);
            countCell.type = INT_TYPE;
            sprintf(countCell.value, "%d", counts[i]);
            addCell(groupByRs, groupByRs->numRows - 1, countCell.type, countCell.value);
            free(groups[i]);
        }
    }
    return groupByRs;
}

// 使用B树索引进行查询
RowSet* selectFilterWhereBTreeIndex(RowSet* rs, int columnIndex, const char* value, BTreeIndex* btreeIndex) {
    // 假设value是整数类型的字符串
    int key = atoi(value);  // 将字符串转换为整数
    RowSet* result = searchBTreeIndex(btreeIndex, key);
    if (result != NULL) {
        return result; // 直接返回索引查询结果
    }
    return NULL; // 索引中未找到
}

// 使用红黑树索引进行查询
RowSet* selectFilterWhereRBTreeIndex(RowSet* rs, int columnIndex, const char* value, RBTreeIndex* rbtreeIndex) {
    // 假设value是整数类型的字符串
    int key = atoi(value);  // 将字符串转换为整数
    RowSet* result = searchRBTreeIndex(rbtreeIndex, key);
    if (result != NULL) {
        return result; // 直接返回索引查询结果
    }
    return NULL; // 索引中未找到
}