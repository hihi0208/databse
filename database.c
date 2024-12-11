// database.c
#include "database.h"
//初始化RowSet结构，分配初始容量的内存空间
void initRowSet(RowSet* rs, int initialCapacity) {
    rs->rows = malloc(sizeof(Row) * initialCapacity);
    rs->numRows = 0;
    rs->capacity = initialCapacity;
}
// 释放RowSet结构及其包含的所有行和单元格的内存空间
void freeRowSet(RowSet* rs) {
    for (int i = 0; i < rs->numRows; i++) {
        for (int j = 0; j < rs->rows[i].numCells; j++) {
            free(rs->rows[i].cells[j].value);
        }
        free(rs->rows[i].cells);
    }
    free(rs->rows);
}
//向RowSet结构中添加一个新的行
void addRow(RowSet* rs) {
    if (rs->numRows >= rs->capacity) {
        rs->capacity *= 2;
        rs->rows = realloc(rs->rows, sizeof(Row) * rs->capacity);
    }
    Row row;
    row.cells = malloc(sizeof(Cell) * 1);  
    row.numCells = 0;
    row.capacity = 1;
    rs->rows[rs->numRows++] = row;
}
//向指定行的RowSet结构中添加一个新的单元格
void addCell(RowSet* rs, int rowIndex, DataType type, const char* value) {
    Row* row = &rs->rows[rowIndex];
    if (row->numCells >= row->capacity) {
        row->capacity *= 2;
        row->cells = realloc(row->cells, sizeof(Cell) * row->capacity);
    }
    Cell cell;
    cell.type = type;
    strncpy(cell.value, value, MAX_CELL_VALUE);
    row->cells[row->numCells++] = cell;
}
//解析CSV文件，并将数据添加到RowSet结构中
void parseCSV(const char* filename, RowSet* rs) {
    FILE* file = fopen(filename, "r");
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        addRow(rs);
        int index = 0;
        char* token = strtok(line, ",");
        while (token) {
            addCell(rs, rs->numRows - 1, STRING_TYPE, token);
            token = strtok(NULL, ",");
        }
    }
    fclose(file);
}
// 解析CSV文件的头部，获取列索引
void parseHeader(const char* filename, int** columnIndexes, int* numColumns) {
    FILE* file = fopen(filename, "r");
    char line[1024];
    if (fgets(line, sizeof(line), file)) {
        int count = 0;
        char* token = strtok(line, ",");
        while (token) {
            count++;
            token = strtok(NULL, ",");
        }
        *numColumns = count;
        *columnIndexes = malloc(sizeof(int) * count);
        rewind(file);
        if (fgets(line, sizeof(line), file)) {
            token = strtok(line, ",");
            for (int i = 0; i < *numColumns; i++) {
                (*columnIndexes)[i] = atoi(token);
                token = strtok(NULL, ",");
            }
        }
    }
    fclose(file);
}
//B树
BTreeNode* createNode() {
    BTreeNode* node = malloc(sizeof(BTreeNode));
    node->keyCount = 0;
    for (int i = 0; i < MAX_KEYS + 1; i++) {
        node->children[i] = NULL;
    }
    for (int i = 0; i < MAX_KEYS; i++) {
        node->values[i] = NULL;
    }
    return node;
}

void splitNode(BTreeNode* node, BTreeNode* rightNode, int key) {
    int i = node->keyCount - 1;
    while (i >= 0 && node->keys[i] > key) {
        rightNode->keys[i + 1] = node->keys[i];
        rightNode->values[i + 1] = node->values[i];
        node->keys[i] = 0;
        node->values[i] = NULL;
        i--;
    }
    rightNode->keys[0] = key;
    rightNode->values[0] = rightNode->values[1];
    node->keys[i + 1] = 0;
    node->values[i + 1] = NULL;
    rightNode->keyCount++;
    node->keyCount++;
}

BTreeNode* insertNode(BTreeNode* node, int key, RowSet* value) {
    if (node->keyCount == MAX_KEYS) {
        BTreeNode* rightNode = createNode();
        splitNode(node, rightNode, key);
        if (key > node->keys[node->keyCount - 1]) {
            return insertNode(rightNode, key, value);
        } else {
            return insertNode(node->children[node->keyCount], key, value);
        }
    }

    int i = node->keyCount - 1;
    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        node->values[i + 1] = node->values[i];
        i--;
    }
    node->keys[i + 1] = key;
    node->values[i + 1] = value;
    node->keyCount++;
    return NULL;
}

BTreeIndex* createBTreeIndex(int keyColumn, RowSet* rs) {
    BTreeIndex* index = malloc(sizeof(BTreeIndex));
    index->root = createNode();
    for (int i = 0; i < rs->numRows; i++) {
        int key = atoi(rs->rows[i].cells[keyColumn].value);
        insertNode(index->root, key, &rs->rows[i]);
    }
    return index;
}

void insertBTreeIndex(BTreeIndex* index, int key, RowSet* value) {
    if (insertNode(index->root, key, value) != NULL) {
        BTreeNode* newRoot = createNode();
        newRoot->children[0] = index->root;
        newRoot->children[1] = createNode();
        newRoot->keys[0] = key;
        newRoot->values[0] = value;
        newRoot->keyCount++;
        index->root = newRoot;
    }
}

RowSet* searchBTreeIndex(BTreeIndex* index, int key) {
    BTreeNode* node = index->root;
    while (node != NULL) {
        int i = node->keyCount - 1;
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        if (i == -1) {
            return NULL;
        }
        if (node->keys[i] == key) {
            return node->values[i];
        }
        node = node->children[i + 1];
    }
    return NULL;
}

void freeBTreeIndex(BTreeIndex* index) {
    if (index == NULL || index->root == NULL) return;

    // 递归释放左子树
    if (index->root->children[0] != NULL) {
        freeBTreeIndex((BTreeIndex*)index->root->children[0]);
    }

    // 递归释放右子树
    if (index->root->children[1] != NULL) {
        freeBTreeIndex((BTreeIndex*)index->root->children[1]);
    }

    // 释放当前节点的键和值
    for (int i = 0; i < index->root->keyCount; i++) {
        // 假设值是指向RowSet的指针，需要释放RowSet
        if (index->root->values[i] != NULL) {
            freeRowSet(index->root->values[i]);
            free(index->root->values[i]);
        }
    }

    // 释放当前节点
    free(index->root);
    index->root = NULL;
}

//红黑树
// 创建红黑树节点
RBTreeNode* createRBTreeNode(int key, RowSet* value) {
    RBTreeNode* node = (RBTreeNode*)malloc(sizeof(RBTreeNode));
    node->key = key;
    node->value = value;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = 'R';  // 新节点默认为红色
    return node;
}

// 创建红黑树索引
RBTreeIndex* createRBTreeIndex() {
    RBTreeIndex* index = (RBTreeIndex*)malloc(sizeof(RBTreeIndex));
    index->root = NULL;
    return index;
}
// 左旋
void leftRotate(RBTreeNode** root, RBTreeNode* x) {
    RBTreeNode* y = x->right;
    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        *root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

//右旋
void rightRotate(RBTreeNode** root, RBTreeNode* y) {
    RBTreeNode* x = y->left;
    y->left = x->right;
    if (x->right != NULL) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == NULL) {
        *root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
}

//插入红黑树节点
RBTreeNode* insertRBTreeNode(RBTreeNode* node, int key, RowSet* value) {
    if (node == NULL) {
        return createRBTreeNode(key, value);
    }
    if (key < node->key) {
        node->left = insertRBTreeNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertRBTreeNode(node->right, key, value);
    } else {
        // 键已存在，更新值
        freeRowSet(node->value);
        node->value = value;
        return node; // 返回当前节点
    }
    return node; // 返回未改变的节点
}
    // 调整红黑树属性
    void fixRedBlackTree(RBTreeNode* node, RBTreeNode* root) {
    RBTreeNode* grandparent = NULL;
    while (node->parent != NULL && node->parent->color == 'R') {
        grandparent = node->parent->parent;
        if (node->parent == grandparent->left) { // node的父节点是祖父节点的左孩子
            RBTreeNode* uncle = grandparent->right;
            if (uncle && uncle->color == 'R') { // 叔叔节点是红色
                node->parent->color = 'B';
                uncle->color = 'B';
                grandparent->color = 'R';
                node = grandparent;
            } else { // 叔叔节点是黑色
                if (node == node->parent->right) { // node是右孩子
                    node = node->parent;
                    leftRotate(root, node);
                }
                rightRotate(root, grandparent);
                char tempColor = node->parent->color;
                node->parent->color = grandparent->color;
                grandparent->color = tempColor;
                node = node->parent;
            }
        } else { // node的父节点是祖父节点的右孩子
            RBTreeNode* uncle = grandparent->left;
            if (uncle && uncle->color == 'R') { // 叔叔节点是红色
                node->parent->color = 'B';
                uncle->color = 'B';
                grandparent->color = 'R';
                node = grandparent;
            } else { // 叔叔节点是黑色
                if (node == node->parent->left) { // node是左孩子
                    node = node->parent;
                    rightRotate(root, node);
                }
                leftRotate(root, grandparent);
                char tempColor = node->parent->color;
                node->parent->color = grandparent->color;
                grandparent->color = tempColor;
                node = node->parent;
            }
        }
        if (node == root) break; // 防止无限循环
    }
    root->color = 'B'; // 根节点设为黑色
}

// 搜索红黑树节点
RowSet* searchRBTreeNode(RBTreeNode* node, int key) {
    while (node != NULL) {
        if (node->key == key) {
            return node->value;
        } else if (key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return NULL; // 未找到
}

// 释放红黑树节点
void freeRBTreeNode(RBTreeNode* node) {
    if (node == NULL) return;
    freeRBTreeNode(node->left);
    freeRBTreeNode(node->right);
    freeRowSet(node->value); // 假设freeRowSet是释放RowSet的函数
    free(node);
}