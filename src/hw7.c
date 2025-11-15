#include "hw7.h"
#define MAX 100
bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root) {
    if (root == NULL){
        bst_sf *node = malloc(sizeof(bst_sf));
        node->mat = mat;
        node->left_child = NULL;
        node->right_child = NULL;
        return node;
    }

    if (mat->name < root->mat->name){
        root->left_child = insert_bst_sf(mat, root->left_child);
    } else {
        root->right_child = insert_bst_sf(mat, root->right_child);
    }
    return root;
}

matrix_sf* find_bst_sf(char name, bst_sf *root) {
    if (root == NULL){
        return NULL;
    }
    if (name == root->mat->name){
        return root->mat;
    }
    if (name < root->mat->name){
        return find_bst_sf(name, root->left_child);
    }
    if (name > root->mat->name){
        return find_bst_sf(name, root->right_child);
    }
}

void free_bst_sf(bst_sf *root) {
    if (root == NULL){
        return;
    }
    free_bst_sf(root->left_child);
    free_bst_sf(root->right_child);

    free(root->mat);
    free(root);
}

matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    unsigned int rows = mat1->num_rows;
    unsigned int cols = mat1->num_cols;

    matrix_sf *m = malloc(sizeof(matrix_sf)+rows*cols*sizeof(int));

    m->name = '$';
    m->num_cols = cols;
    m->num_rows = rows;

    for (int i = 0; i < cols * rows; i++){
        m->values[i] = mat1->values[i] + mat2->values[i];
    }

    return m;
}

matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    unsigned int rows1 = mat1->num_rows;
    unsigned int cols1 = mat1->num_cols;
    unsigned int cols2 = mat2->num_cols;

    matrix_sf *m = malloc(sizeof(matrix_sf)+rows1*cols2*sizeof(int));

    m->name = '$';
    m->num_rows = rows1;
    m->num_cols = cols2;

    for (int i = 0; i < rows1; i++){
        for (int j = 0; j < cols2; j++){
            int ans = 0;
            for (int k = 0; k < cols1; k++){
                int x = mat1->values[i * cols1 + k];
                int y = mat2->values[k * cols2 + j];
                ans += x*y;
            }
            m->values[i * cols2 + j] = ans;
        }
    }

   return m;
}

matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
    unsigned int rows = mat->num_rows;
    unsigned int cols = mat->num_cols;

    matrix_sf *m = malloc(sizeof(matrix_sf)+rows*cols*sizeof(int));

    m->name = '$';
    m->num_cols = rows;
    m->num_rows = cols;

    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            m->values[j * rows + i] = mat->values[i * cols + j];
        }
    }

    return m;
}

matrix_sf* create_matrix_sf(char name, const char *expr) {
    unsigned int rows, cols; 

    const char *p = expr;
    sscanf(p, "%u %u", &rows, &cols);
    int total = rows * cols;

    matrix_sf *m = malloc(sizeof(matrix_sf)+total*sizeof(int));
    
    m->name = name;
    m->num_cols = cols;
    m->num_rows = rows;

    for (int i = 0; i < total; i++) {
        m->values[i] = 0;
    }

    while (*p && *p != '['){
        p++;
    }
    if (!*p){
        return m;
    }
    p++;
    int index = 0;
    while (*p && *p != ']' && index < total){
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == ';') {
            p++;
            continue;
        }
        int num = 0;
        int spaces = 0;
        if (sscanf(p, "%d%n", &num, &spaces) == 1 && spaces > 0){
            m->values[index++] = num;
            p += spaces;
        } else {
            p++;
        }

    }

    return m;
}

int precedence(char ch){
    switch(ch){
        case '\'': return 4;
        case '^': return 3;
        case '/':
        case '*': return 2;
        case '+':
        case '-': return 1;
        case '(':return 0;
        default: return -1;
    }
}

typedef struct{
    char ch[MAX];
    int top;
} stack;

void push(stack *s, char c){
    if (s->top < MAX - 1){
        s->ch[++(s->top)] = c;
    }
}

char pop(stack *s){
    if (s->top >= 0){
        return s->ch[(s->top)--];
    }
}

char peek(stack *s){
    if (s->top >= 0){
        return s->ch[s->top];
    }
}

int is_empty(stack *s){
    return s->top == -1;
}

char* infix2postfix_sf(char *infix) {
    char *postfix = malloc(strlen(infix) * 2 + 1);
    
    char *p = infix;
    stack opstack;
    opstack.top = -1;
    int i = 0;
    while (*p != '\0'){
        if (*p == ' '){
            p++;
            continue;
        }
        if (isupper(*p)){
            postfix[i++] = *p;
        }
        else if (*p == '('){
            push(&opstack, *p);
        }
        else if (*p == ')'){
            while (!is_empty(&opstack) && peek(&opstack) != '('){
                postfix[i++] = pop(&opstack);
            }
            pop(&opstack);
        }
        else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '^' || *p == '\''){
            while (!is_empty(&opstack) && precedence(peek(&opstack)) >= precedence(*p)){
                postfix[i++] = pop(&opstack);
            }
            push(&opstack, *p);
        }
        p++;
    }
    while (!is_empty(&opstack)){
        postfix[i++] = pop(&opstack);
    }
    postfix[i] = '\0';
    return postfix;
}

matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
    char *postfix = infix2postfix_sf(expr);
    matrix_sf* stack[MAX];
    int top = -1;

    for (int i = 0; postfix[i] != '\0'; i++){
        char ch = postfix[i];

        if ((isupper(ch))){
            matrix_sf* m = find_bst_sf(ch, root);
            stack[++top] = m;
        }
        else if (ch == '\''){
            matrix_sf* mat1 = stack[top--];
            matrix_sf* result1 = transpose_mat_sf(mat1);

            result1->name = '$';

            if (mat1->name == '$') free(mat1);

            stack[++top] = result1;
        }
        else if (ch == '+' || ch == '*'){
            matrix_sf* mat2 = stack[top--];
            matrix_sf* mat1 = stack[top--];
            matrix_sf* ans;

            if (ch == '*'){
                ans = mult_mats_sf(mat1, mat2);
            } else {
                ans = add_mats_sf(mat1, mat2);
            }

            if (mat1->name == '$') free(mat1);
            if (mat2->name == '$') free(mat2);

            ans->name = '$';
            stack[++top] = ans;
        }
    }

    matrix_sf* ans = stack[top--];
    ans->name = name;

    free(postfix);
    return ans;
}

matrix_sf *execute_script_sf(char *filename) {
    FILE *file = fopen(filename, "r");
    
    char *line = NULL;
    size_t length = 0;
    bst_sf *root = NULL;
    matrix_sf *final = NULL;

    while (getline(&line, &length, file) != -1){
        char name = line[0];
        char *equation = strchr(line, '=');
        char *expr = equation + 1;
        while (*expr == ' ' || *expr == '\t'){
            expr++;
        }

        matrix_sf *m;

        if (strchr(expr, '[')){
        m = create_matrix_sf(name, expr);
        } else {
            m = evaluate_expr_sf(name, expr, root);
        }

        root = insert_bst_sf(m, root);

        final = m;

    }

    free(line);

    matrix_sf *result = NULL;
    if (final != NULL) {
        result = copy_matrix(final->num_rows, final->num_cols, final->values);
        result->name = final->name;
    }

    free_bst_sf(root);
    fclose(file);

   return result;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
