#include "../headers/tree.h"

tree_node* tree_buildPre(void** preorder, void** inorder, size_t size){
    if(size == 0) return NULL;
    tree_node *head = malloc(sizeof(tree_node));
    head->val = preorder[0];
    int mid = 0;
    while(inorder[mid] != preorder[0]) mid++;
    head->left = tree_buildPre(preorder + 1, inorder, mid);
    head->right = tree_buildPre(preorder + mid + 1, inorder + mid + 1, size - mid - 1);
    return head;
}

void tree_printPre(const tree_node* root, tree_print_fn print_fn) {
    if(root == NULL) return;
    print_fn(root->val);
    tree_printPre(root->left, print_fn);
    tree_printPre(root->right, print_fn);
}

void tree_printIn(const tree_node* root, tree_print_fn print_fn) {
    if(root == NULL) return;
    tree_printIn(root->left, print_fn);
    print_fn(root->val);
    tree_printIn(root->right, print_fn);
}

void tree_printPost(const tree_node* root, tree_print_fn print_fn) {
    if(root == NULL) return;
    tree_printPost(root->left, print_fn);
    tree_printPost(root->right, print_fn);
    print_fn(root->val);
}

static void** tree_private_getPre(const tree_node* root, void** preorder, size_t* size) {
    if(root == NULL) return preorder;
    void** tmp = malloc(sizeof(void*) * (*size + 1));
    memcpy(tmp, preorder, (*size) * sizeof(void*));
    free(preorder);
    tmp[*size] = root->val;
    (*size)++;
    tmp = tree_private_getPre(root->left, tmp, size);
    return tree_private_getPre(root->right, tmp, size);
}

void** tree_getPre(const tree_node* root, size_t* size) {
    *size = 0;
    return tree_private_getPre(root,NULL,size);
}

static void** tree_private_getIn(const tree_node* root, void** inorder, size_t* size) {
    if(root == NULL) return inorder;
    void** tmp = malloc(sizeof(void*) * (*size + 1));
    memcpy(tmp, inorder, (*size) * sizeof(void*));
    free(inorder);
    tmp = tree_private_getIn(root->left, tmp, size);
    tmp[*size] = root->val;
    (*size)++;
    return tree_private_getIn(root->right, tmp, size);
}

void** tree_getIn(const tree_node* root, size_t* size) {
    *size = 0;
    return tree_private_getIn(root,NULL,size);
}

static void** tree_private_getPost(const tree_node* root, void** postorder, size_t* size) {
    if(root == NULL) return postorder;
    void** tmp = malloc(sizeof(void*) * (*size + 1));
    memcpy(tmp, postorder, (*size) * sizeof(void*));
    free(postorder);
    tmp = tree_private_getPost(root->left, tmp, size);
    tmp = tree_private_getPost(root->right, tmp, size);
    tmp[*size] = root->val;
    (*size)++;
    return tmp;
}

void** tree_getPost(const tree_node* root, size_t* size) {
    *size = 0;
    return tree_private_getPost(root,NULL,size);
}

void* tree_getIndex(const tree_node* root, size_t index) {
    if(root == NULL) return NULL;
    if(index == 1) return root->val;
    int test = log(index) / log(2);
    if(index >= (pow(2,test) + pow(2,test + 1)) / 2) {
        return tree_getIndex(root->right,index - pow(2,test));
    } 
    return tree_getIndex(root->left,index - pow(2,test - 1));
}

void insertIndex(tree_node* root, size_t index, void* val) {
    if(root == NULL) return;
    if(index == 2) {
        root->left = malloc(sizeof(tree_node));
        root->left->val = val;
        return;
    }
    if(index == 3) {
        root->right = malloc(sizeof(tree_node));
        root->right->val = val;
        return;
    }
    size_t test = log(index) / log(2);
    if(index >= (pow(2,test) + pow(2,test + 1)) / 2) {
        insertIndex(root->right,index - pow(2,test),val);
    } else {
        insertIndex(root->left,index - pow(2,test - 1),val);
    }
}

size_t tree_isBalanced(const tree_node* root) {
    if(root == NULL) return 1;
    size_t left = tree_isBalanced(root->left), right = tree_isBalanced(root->right);
    if(left == 0 || right == 0 || left - right < -1 || left - right > 1) return 0;
    return (left > right ? left : right) + 1;
}



void tree_private_printLevel(tree_node** stack, size_t size, tree_print_fn print_fn) {
    size_t count = 0;
    tree_node** newStack = malloc(sizeof(tree_node*) * size * 2);
    for(int i = 0; i < size;i++) {
        if(stack[i] == NULL) {
            print_fn(NULL);
        } else {
            if(stack[i]->right != NULL || stack[i]->left != NULL) count++;
            print_fn(stack[i]->val);
            newStack[i * 2] = stack[i]->left;
            newStack[i * 2 + 1] = stack[i]->right;
        }
        if(i == size - 1 && count == 0) break;
    }
    free(stack);
    if(count == 0) {
        free(newStack);
        return;
    }
    tree_private_printLevel(newStack, size * 2, print_fn);
}

void tree_printLevel(tree_node* root, tree_print_fn print_fn) {
    tree_node** stack = malloc(sizeof(tree_node*));
    *stack = root;
    tree_private_printLevel(stack, 1, print_fn);
}

tree_node* tree_createBalanced(void** arr, size_t size) {
    if(size == 0) return NULL;
    tree_node* root = malloc(sizeof(tree_node));
    size_t mid = size / 2;
    root->val = arr[mid];
    root->left = tree_createBalanced(arr,mid);
    root->right = tree_createBalanced(arr + mid + 1, size - mid - 1);
    return root;
}

tree_node* tree_balance(const tree_node* root, __compar_fn_t compare) {
    size_t size;
    void** all = tree_getPre(root,&size);
    qsort(all, size, sizeof(void*), compare);
    tree_node* tmp = tree_createBalanced(all, size);
    free(all);
    return tmp;
}

size_t tree_isSearch(const tree_node* root) {
    if(root == NULL) return 1;
    if(root->left != NULL && root->left->val > root->val) return 0;
    if(root->right != NULL && root->right->val < root->val) return 0;
    return tree_isSearch(root->left) && tree_isSearch(root->right);
}

void tree_free(tree_node* root) {
    if(root == NULL) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}
