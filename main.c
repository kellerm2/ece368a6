#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// declare fxns
void insert_head(list_node* list, Node* node);
void push(list_stack* stack, Node* node);
void is_empty(list_node* list);
void delete_head(list_stack* stack);
Node* pop(list_stack* stack);
Node* read_in(FILE* file);
void preorder(Node* node);

typedef struct Node {
    int label;
    int width;
    int height;
    char cut;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct list_node {
    Node* node;            
    struct list_node* next;
} list_node;

typedef struct list_stack {
    struct list_node* list;
} list_stack;


void insert_head(list_node* list, Node* node) {
    list_node* new = (list_node*) malloc(sizeof(list_node));
    new->node = node; // the new node is the node passed in
    new->next = stack->list; // new node points to stack list
    stack->list = new; // new is pushed to stack
}

void push(list_stack* stack, Node* node) {
    insert_head(stack->list, node);    
}

void is_empty(list_node* list) {
    if (list == NULL) return 1;
    return 0;
}

void delete_head(list_stack* stack) {
    list_node* temp = stack->list;
    stack->list = temp->next;
    free(temp);
}

Node* pop(list_stack* stack) {
    assert(!is_empty(*(stack->list)));
    Node* node = stack->list->node;
    delete_head(stack);
    return node;
}

Node* read_in(FILE* file) {
    char line[50];
    list_stack stack = NULL;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'V' || line[0] == 'H') { // node is a cutline
            char cut = line[0];
            Node* right = pop(&stack);
            Node* left = pop(&stack);
            Node* cutNode = create_cut(cut, left, right);
            push(&stack, cutNode);
        }
        else { // node is a package
            int label;
            int width; 
            int height;
            sscanf(line, "%d(%d,%d)", &label, &width, &height);
            Node* pkg = create_pkg(label, width, height);
            push(&stack, pkg);
        }
    }
    return pop(&stack);
}

void preorder(Node* node) {
    if (node == NULL) return;
    if (node->cut == NULL) printf("%d(%d,&d)\n", node->label, node->width, node->height);
    else printf("%c\n", node->cut);
    preorder(node->left);
    preorder(node->right);
}

int main(int argc, char* argv[]) {
    if (argc != 5) return 0; // args ./a6, in, out1, out2, out3
    char* in_file = argv[1];
    FILE* file = (FILE*) fopen(in_file, "r"); // open txt file
    assert(file != NULL);

    Node* node = read_in(file);

    return 0;
}