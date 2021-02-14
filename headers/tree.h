#ifndef HEAD_TREE
#define HEAD_TREE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct tree_node {
    void* val;
    struct tree_node *left;
    struct tree_node *right;
} tree_node;

typedef void (*tree_forEach_fn)(void* value);

// return a tree build with the preorder and the inorder
tree_node* tree_buildPre(void** preorder, void** inorder, size_t size);

// return a tree build with the postorder and the inorder
tree_node* tree_buildPost(void** postorder, void** inorder, size_t size);

// give to todo_fn the value of every node in the preorder
void tree_forEachPre(tree_node* root, tree_forEach_fn todo_fn);

// give to todo_fn the value of every node in the inorder
void tree_forEachIn(tree_node* root, tree_forEach_fn todo_fn);

// give to todo_fn the value of every node in the postorder
void tree_forEachPost(tree_node* root, tree_forEach_fn todo_fn);

// return 0 if the tree is not balanced, return a size_t > 0 if balanced 
size_t tree_isBalanced(const tree_node* root);

// return an array of all the values in the preorder, size is the size of the returned array
// array is malloced, must be free
void** tree_getPre(const tree_node* root, size_t* size);

// return an array of all the values in the inorder, size is the size of the returned array
// array is malloced, must be free
void** tree_getIn(const tree_node* root, size_t* size);

// return an array of all the values in the postorder, size is the size of the returned array
// array is malloced, must be free
void** tree_getPost(const tree_node* root, size_t* size);

// return the value of the node at the given index
// (index in level order, start at 1)
void* tree_getIndex(const tree_node* root, size_t index);

// give to todo_fn the value of every node in the level order
void tree_printLevel(tree_node* root, tree_forEach_fn todo_fn);

// create a balanced tree with the given arr, assuming that the array is sorted
tree_node* tree_createBalanced(void** arr, size_t size);

// return a new tree wich is a balanced version of the given tree
// take a comparaison fonction
tree_node* tree_balance(const tree_node* root, __compar_fn_t compare);

// return 0 if the tree is NOT a search tree, return 1 if it is
size_t tree_isSearch(const tree_node* root);

// free the entire tree
void tree_free(tree_node* root);



#endif