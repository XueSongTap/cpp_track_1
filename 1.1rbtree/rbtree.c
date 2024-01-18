#include <stdio.h>
#include <stdlib.h>
// 这部分代码定义了红黑树节点rbtree_node和红黑树rbtree的结构。每个节点包含颜色、父节点、左子节点、右子节点和键值。红黑树结构包含指向根节点和哨兵节点（nil节点，代表空（NULL）节点，通常是黑色的）的指针。

#define RED     0
#define BLACK   1

typedef int KEY_TYPE;
typedef struct _rbtree_node rbtree_node;

struct _rbtree_node {
    unsigned char color;
    rbtree_node *parent;
    rbtree_node *left;
    rbtree_node *right;
    KEY_TYPE key;
};


typedef struct rbtree {
    rbtree_node *root;
    rbtree_node *nil; // NULL
}rbtree;
// 左旋转和右旋转,旋转操作是红黑树调整结构以保持或恢复平衡的关键操作。左旋转和右旋转是对称的操作，都是将一个节点和它的一个子节点“旋转”以改变树的结构。
// rotate 
void rbtree_left_rotate(rbtree *T, rbtree_node *x) {
    // NULL --> T->nil
    if (x == T->nil) return ;
    // 1
    rbtree_node *y = x->right;

    x->right = y->left;
    if (y->left != T->nil) {
        y->left->parent = x;
    }

    // 2
    y->parent = x->parent;
    if (x->parent == T->nil) {
        T->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    // 3
    y->left = x;
    x->parent = y;

}

// x --> y, y -->x 
// left --> right, right --> left

void rbtree_right_rotate(rbtree *T, rbtree_node *y) {
    // NULL --> T->nil
    if (y == T->nil) return ;
    // 1
    rbtree_node *x = y->left;

    y->left = x->right;
    if (y->left != T->nil) {
        y->left->parent = x;
    }

    // 2
    y->parent = x->parent;
    if (x->parent == T->nil) {
        T->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    // 3
    y->left = x;
    x->parent = y;

}

// 插入和修复
// 这些函数实现红黑树键值的插入和插入后的修复。
// rbtree_insert_fixup函数则负责修复插入操作可能引起的红黑树性质的违反。

void rbtree_insert_fixup(rbtree *T, rbtree_node *z) {
    //
    // z->color == RED
    // z->parent->color == RED

    // z--> RED
    while (z->parent->color == RED) { // 

        if (z->parent == z->parent->parent->left) {
            rbtree_node *y = z->parent->parent->right;
            y = z->parent->parent->right; //

            if (y->color == RED) {

                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                y->color = BLACK;

                z = z->parent->parent; //
            } else { // 

                if (z = z->parent->right) {
                    z = z->parent;
                    rbtree_left_rotate(T, z);
                }

                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtree_right_rotate(T, z->parent->parent);

            }


        } 

    }


}
// rbtree_insert函数负责将新节点插入树中并维持二叉搜索树的属性。

void rbtree_insert(rbtree *T, rbtree_node *z) {

    rbtree_node *y = T->nil;
    rbtree_node *x = T->root;

    while (x != T->nil) {

        y = x;

        if (z->key < x->key) {
            x = x->left;
        } else if (z->key > x->key) {
            x = x->right;
        } else { //Exist

        }

    }

    z->parent = y;
    if (y == T->nil) {
        T->root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    // z --> 
    z->color = RED;
    z->left = T->nil;
    z->right = T->nil;

    // 

}
// 这个函数用来在红黑树中查找给定键值的节点。
rbtree_node* rbtree_search(rbtree* T, int key) {
    rbtree_node* current = T->root;

    while (current != T->nil && current->key != key) {
        if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return current;
}





/*
TODO

代码遗漏和问题
rbtree_insert_fixup函数的实现没有完整提供，这里显示的代码是不完整的。
rbtree_insert函数在插入节点后没有调用修复函数rbtree_insert_fixup，这是必需的，以保持红黑树的性质。
main函数中的遍历方法是不正确的，仅遍历了树的右侧节点，并没有正确地显示整棵树的所有键值。
main函数没有释放分配的内存，这在真实的应用程序中可能会引起内存泄露。
*/
int main() {
    rbtree_node *nil = (rbtree_node *)malloc(sizeof(rbtree_node));
    nil->color = BLACK;
    nil->parent = NULL;
    nil->left = NULL;
    nil->right = NULL;
    nil->key = 0;

    rbtree *T = (rbtree *)malloc(sizeof(rbtree));
    T->root = nil;
    T->nil = nil;

    // Example usage
    rbtree_node *node1 = (rbtree_node *)malloc(sizeof(rbtree_node));
    node1->key = 1;
    rbtree_insert(T, node1);

    rbtree_node *node2 = (rbtree_node *)malloc(sizeof(rbtree_node));
    node2->key = 2;
    rbtree_insert(T, node2);

    rbtree_node *node3 = (rbtree_node *)malloc(sizeof(rbtree_node));
    node3->key = 3;
    rbtree_insert(T, node3);

    // Display the keys in the tree
    printf("Keys in the tree: ");
    rbtree_node *current = T->root;
    while (current != T->nil) {
        printf("%d ", current->key);
        current = current->right;
    }
    printf("\n");

    int searchKey = 2;
    rbtree_node* searchNode = rbtree_search(T, searchKey);
    if (searchNode != T->nil) {
        printf("Node with key %d found!\n", searchKey);
    } else {
        printf("Node with key %d not found.\n", searchKey);
    }
}