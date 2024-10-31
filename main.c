#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// define structs
typedef struct Node {
    int label;
    int width;
    int height;
    char cut;
    struct Node* left;
    struct Node* right;
    int xcorner;
    int ycorner;
} Node;

typedef struct Tree {
    struct Node* root;
} Tree;

typedef struct list_node {
    Node* node;            
    struct list_node* next;
} list_node;

typedef struct list_stack {
    struct list_node* top;
} list_stack;

// declare fxns
void insert_head(list_stack* stack, Node* node);
void push(list_stack* stack, Node* node);
int is_empty(list_stack* stack);
void delete_head(list_stack* stack);
Node* pop(list_stack* stack);
Tree* read_in(FILE* file);
void preorder(Node* node, FILE* file);
Node* create_pkg(int label, int width, int height);
Node* create_cut(char cut, Node* left, Node* right);
void free_stacklist(list_node* top);
void postorder(Node* node, FILE* file);
void get_corner(Node* node, int xc, int yc);
void print_corner(Node* node, FILE* file);
void free_postorder(Node* node);

// inserts node on top of the tree assembly stack, in junction with push
void insert_head(list_stack* stack, Node* node) {
    list_node* new = (list_node*) malloc(sizeof(list_node));
    new->node = node; // the new node is the node passed in
    new->next = stack->top; // new node points to current stack top
    stack->top = new; // new is pushed to stack top
}
// pushs a node onto the tree assembly stack
void push(list_stack* stack, Node* node) {
    insert_head(stack, node);    
}
// checks if the tree assembly stack is empty
int is_empty(list_stack* stack) {
    return (stack->top == NULL);
}
// deletes the head of the stack for assembly of the package/cut tree
void delete_head(list_stack* stack) {
    list_node* head = stack->top;
    stack->top = head->next;
    free(head);
}
// pops off node from tree assembly stack, used with is_empty and delete_head
Node* pop(list_stack* stack) {
    assert(!is_empty(stack));
    Node* node = stack->top->node;
    delete_head(stack);
    return node;
}
// creates a node type to designate a package with package characteristics
Node* create_pkg(int label, int width, int height) {
    Node* new = (Node*) malloc(sizeof(Node));
    assert(new != NULL);
    new->label = label;
    new->width = width;
    new->height = height;
    new->left = NULL; // leaf node
    new->right = NULL;
    new->cut = 0; // represents NUL char
    return new;
}
// creates a node type to designate a horizontal or vertical cutline
Node* create_cut(char cut, Node* left, Node* right) {
    Node* new = (Node*) malloc(sizeof(Node));
    assert(new != NULL);
    new->cut = cut;
    new->left = left; // internal node
    new->right = right;
    new->label = -1;
    new->width = -1;
    new->height = -1;
    return new;
}
// frees list nodes of the stack used to assemble the tree
void free_stacklist(list_node* top) {
    list_node* current = top;
    if (current == NULL) return;
        
    free(current);
    top = top->next;
    current = top;
}
// reads in the input file, creates node for packages and cuts, and uses a stack to assemble the postorder traversal into a tree
Tree* read_in(FILE* file) {
    char line[25];
    list_stack* stack = (list_stack*) malloc(sizeof(list_stack));
    assert(stack != NULL);
    stack->top = NULL;
    Tree* tree = (Tree*)malloc(sizeof(Tree));
    assert(tree != NULL);
    tree->root = NULL;

    while (fgets(line, sizeof(line), file)) { // reads a line till end of line char
        if (line[0] == 'H' || line[0] == 'V') { // node is a cutline
            char cut = line[0];
            Node* right = pop(stack); // right pkg is first to come off stack
            Node* left = pop(stack); // left pkg is 2nd to come off stack
            Node* cut_node = create_cut(cut, left, right); // make the cut node with children
            push(stack, cut_node); // push new cut node back onto stack
        }
        else { // node is a package
            int label;
            int width; 
            int height;
            sscanf(line, "%d(%d,%d)", &label, &width, &height);
            Node* pkg = create_pkg(label, width, height);
            push(stack, pkg); // push pkg onto stack
        }
    }

    tree->root = pop(stack); // first to pop is root of tree bc postorder
    free_stacklist(stack->top);
    free(stack);
    return tree;
}
// prints out the nodes of the tree in preorder traversal O(n)
void preorder(Node* node, FILE* file) {
    if (node == NULL) return;
        
    if (node->cut == 0) fprintf(file, "%d(%d,%d)\n", node->label, node->width, node->height);
    else fprintf(file, "%c\n", node->cut);
    preorder(node->left, file);
    preorder(node->right, file);
}
// figures out the size of the room for cuts based on the fact that V is made of dimensions of (child 1 width + child 2 width, max height of children)
// for H, it is made of dimensions of (max width of children, child 1 height + child 2 height)
// this information is conveyed in postorder travseral O(n)
void postorder(Node* node, FILE* file) {
    if (node == NULL) return;
    postorder(node->left, file);
    postorder(node->right, file);
        if (node->cut == 0) // it's a pkg
            fprintf(file, "%d(%d,%d)\n", node->label, node->width, node->height);
        else {
            int small_width = 0;
            int small_height = 0;
            switch (node->cut) {
            case 'V': // child nodes are side by side
                small_width = node->left->width + node->right->width; // width is sum of widths
                small_height = (node->left->height > node->right->height) ? node->left->height : node->right->height;
                break;
            case 'H': // child nodes on top of each other
                small_width = (node->left->width > node->right->width) ? node->left->width : node->right->width;
                small_height = node->left->height + node->right->height; // height is sum of heights
            }
            node->width = small_width; // update cut node's new width, height
            node->height = small_height;

            fprintf(file, "%c(%d,%d)\n", node->cut, node->width, node->height);
        }
    return;
}
// traverses the tree until it reaches origin corner (0,0) at the left leaf of the right most subtree
// then works back to assign a sibling node of parent V (0+width of child 1, 0), and parent H (former x coordinate, ycoord + other sibling's height)
// it also keeps track of the V and H corners using this same logic
void get_corner(Node* node, int xc, int yc) {
    if (node == NULL) return;

    if (node->cut == 'H') {
        get_corner(node->right, xc, yc);
        get_corner(node->left, xc, yc + node->right->height);
        node->xcorner = xc;
        node->ycorner = yc;
    }
    else if (node->cut == 'V') {
        get_corner(node->left, xc, yc);
        get_corner(node->right, xc + node->left->width, yc);
        node->xcorner = xc;
        node->ycorner = yc;
    }
    else {
        node->xcorner = xc;
        node->ycorner = yc;
        return;
    }
        // if (node->cut == 0) { // it's a pkg
        //     printf("%d((%d,&d)(%d,%d))\n", 
        //     node->label, node->width, node->height, xcorner, ycorner);
        // }
        // else {
        //     switch (node->cut) {
        //     case 'V':
        //         get_corner(node->left, file, xc, yc);
        //         get_corner(node->right, file, xc + node->left->width, yc);
        //         break;
        //     case 'H':
        //         get_corner(node->left, file, xc, yc + node->right->height);
        //         get_corner(node->right, file, xc, yc);
        //     }        
        // }    
    return;
}
// after corners are assigned to nodes, the tree is traversed in postorder and printed O(n)
void print_corner(Node* node, FILE* file) {
    if (node == NULL) return;
        
    print_corner(node->left, file);
    print_corner(node->right, file);
    if (node->cut == 0)
        fprintf(file, "%d((%d,%d)(%d,%d))\n", node->label, node->width, node->height,
        node->xcorner, node->ycorner);
}
// the nodes in the tree used to store packages and cuts is freed using postorder traversal
void free_postorder(Node* node) {
    if (node == NULL) return;
    free_postorder(node->left);
    free_postorder(node->right);
    free(node);
}

int main(int argc, char* argv[]) {
    if (argc != 5) return 0; // args ./a6, in, out1, out2, out3
    char* in_file = argv[1];
    FILE* file = (FILE*) fopen(in_file, "r"); // open txt file
    assert(file != NULL);

    Tree* tree = read_in(file);
    fclose(file);

    // output file 1
    FILE* out_1 = (FILE*) fopen(argv[2], "w");
    assert(out_1 != NULL);
    preorder(tree->root, out_1);
    fclose(out_1);

    // output file 2
    FILE* out_2 = (FILE*) fopen(argv[3], "w");
    assert(out_2 != NULL);
    postorder(tree->root, out_2);
    fclose(out_2);

    // output file 3
    FILE* out_3 = (FILE*) fopen(argv[4], "w");
    assert(out_3 != NULL);
    int xc = 0;
    int yc = 0;
    get_corner(tree->root, xc, yc);
    print_corner(tree->root, out_3);
    fclose(out_3);

    // free memory
    free_postorder(tree->root);
    free(tree);

    return 0;
}