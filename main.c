// main.c
#include "database.h"
#include "crud_operations.h"

int main() {
    // 初始化 RowSet 用于存储从 CSV 文件加载的数据
    RowSet rs;
    initRowSet(&rs, 10); // 初始容量为 10

    // 解析 CSV 文件加载数据
    parseCSV("titanic_survivor.csv", &rs);

    // 解析表头文件获取列索引信息
    int* columnIndexes = NULL;
    int numColumns = 0;
    parseHeader("titanic_survivor.header", &columnIndexes, &numColumns);

    // 创建 B树索引
    BTreeIndex* btreeIndex = createBTreeIndex(0, &rs); // 假设第一个字段是索引列

    // 创建红黑树索引
    RBTreeIndex* rbtreeIndex = createRBTreeIndex();

    // 插入数据到 B树索引
    for (int i = 0; i < rs.numRows; i++) {
        int key = atoi(rs.rows[i].cells[0].value); // 假设第一列是我们要索引的键
        insertBTreeIndex(btreeIndex, key, &rs.rows[i]);
        insertRBTreeIndex(rbtreeIndex, key, &rs.rows[i]); // 同时插入红黑树索引
    }

    // 使用 B树索引搜索键值
    RowSet* resultBTree = searchBTreeIndex(btreeIndex, 1); // 搜索键值为 1 的记录

    // 使用红黑树索引搜索键值
    RowSet* resultRBTree = searchRBTreeIndex(rbtreeIndex, 1); // 搜索键值为 1 的记录

    // 打印 B树索引搜索结果
    printf("BTree Index Search Results:\n");
    for (int i = 0; i < resultBTree->numRows; i++) {
        Row* row = &resultBTree->rows[i];
        for (int j = 0; j < row->numCells; j++) {
            Cell* cell = &row->cells[j];
            printf("%s ", cell->value);
        }
        printf("\n");
    }

    // 打印红黑树索引搜索结果
    printf("RBTree Index Search Results:\n");
    for (int i = 0; i < resultRBTree->numRows; i++) {
        Row* row = &resultRBTree->rows[i];
        for (int j = 0; j < row->numCells; j++) {
            Cell* cell = &row->cells[j];
            printf("%s ", cell->value);
        }
        printf("\n");
    }

    // 清理资源
    freeRowSet(&rs);
    freeBTreeIndex(btreeIndex);
    freeRBTreeIndex(rbtreeIndex);
    free(columnIndexes);

    return 0;
}