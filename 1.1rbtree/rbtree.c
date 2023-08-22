#include <stdio.h>
#include <stdlib.h>

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