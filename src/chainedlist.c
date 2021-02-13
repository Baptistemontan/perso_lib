#include "../headers/chainedlist.h"


void* cl_add(cl_node** head, void* value) {
    cl_node *list = malloc(sizeof(cl_node));
    list->next = *head;
    list->val = value;
    *head = list;
    return value;
}

void* cl_addEnd(cl_node** head, void* value) {
    while (*head != NULL) {
        head = &((*head)->next);
    }
    cl_add(head,value);
    return value;
}

void* cl_remove(cl_node** list) {
    if(*list == NULL) return NULL;
    cl_node *next = (*list)->next;
    void* tmp = (*list)->val;
    free(*list);
    *list = next;
    return tmp;
}

void* cl_removeIndex(cl_node** list, size_t index) {
    if(*list == NULL) return NULL;
    if(index != 0) return cl_removeIndex(&((*list)->next), index - 1);
    void* tmp = (*list)->val;
    cl_remove(list);
    return tmp;
}

void* cl_get(const cl_node* head, size_t index) {
    if(head == NULL) return NULL;
    if(index != 0) return cl_get(head->next, index - 1);
    return head->val;
}

void cl_forEach(cl_node* list,cl_forEach_fn todo_fn) {
    size_t i = 0;
    while (list != NULL) {
        todo_fn(list->val,i);
        list = list->next;
        i++;
    }
}

void cl_free(cl_node** list, cl_free_fn free_fn) {
    while ((*list) != NULL) {
        void* tmp = cl_remove(list);
        if(free_fn != NULL) free_fn(tmp);
    }
}

int cl_findIndex(const cl_node* head, const void* value, cl_comp_fn comp_fn) {
    int i = 0;
    while(head != NULL) {
        if(comp_fn == NULL) {
            if(head->val == value) return i;
        } else if(comp_fn(head->val, value) == 0) return i;
        head = head->next;
        i++;
    }
    return -1;
}

static void cl_private_swap(cl_node* a, cl_node* b, cl_node* c) {
    void* temp = a->val;
    a->val = c->val;
    c->val = b->val;
    b->val = temp;
}


void cl_qsort_interval(cl_node* start, cl_node* end, __compar_fn_t comp_fn) {
    if(start == end || start->next == end) return;
    cl_node *chaine = start->next, *pivot = start;
    while (chaine != end)
    {
        if(comp_fn(chaine->val,pivot->val) < 0) {
            cl_private_swap(chaine,pivot,pivot->next);
            pivot = pivot->next;
        }
        chaine = chaine->next;
    }
    cl_qsort_interval(start,pivot,comp_fn);
    cl_qsort_interval(pivot->next,end,comp_fn);
}