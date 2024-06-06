#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *load_png_file(const char *filename, int *width, int *height) {
    unsigned char *image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return NULL;
    }
    return (image);
}

typedef struct Node {
    unsigned char r, g, b, a;
    struct Node *up, *down, *left, *right;
    int visited;
    int component;
} Node;

double color_difference(Node* x, Node* y) {
    return abs(x->r - y->r);
}

Node* create_graph(unsigned char *image, int *width, int *height) {

    Node* nodes = malloc(*width * *height * sizeof(Node));


    for (unsigned y = 0; y < *height; ++y) {
        for (unsigned x = 0; x < *width; ++x) {
            Node* node = &nodes[y * *width + x];
            unsigned char* pixel = &image[(y * *width + x) * 4];
            node->r = pixel[0];
            node->g = pixel[1];
            node->b = pixel[2];
            node->a = pixel[3];
            node->up = y > 0 ? &nodes[(y - 1) * *width + x] : NULL;
            node->down = y < *height - 1 ? &nodes[(y + 1) * *width + x] : NULL;
            node->left = x > 0 ? &nodes[y * *width + (x - 1)] : NULL;
            node->right = x < *width - 1 ? &nodes[y * *width + (x + 1)] : NULL;
            node->visited = 0;
            node->component = 0;
        }
    }

    return nodes;
}


typedef struct Stack {
    Node* data;
    struct Stack* next;
} Stack;

Stack* createStack(Node* data) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->data = data;
    stack->next = NULL;
    return stack;
}

void push(Stack** stack, Node* data) {
    Stack* newStack = createStack(data);
//    printf("f");
    newStack->next = *stack;
    *stack = newStack;
}

Node* pop(Stack** stack) {
    if (*stack == NULL) {
        return NULL;
    }
    Stack* temp = *stack;
    Node* node = temp->data;
    *stack = (*stack)->next;
    free(temp);
    return node;
}

void dfs(Node* node, double epsilon, int component) {
    Stack* stack = createStack(node);
    int cnt = 0;
    while (stack) {
        cnt++;
        Node* current = pop(&stack);
        if (current->visited) {
            continue;
        }
        current->visited = 1;
        current->component = component;
//        printf("%d\n", cnt);

        if (current->up && !current->up->visited && color_difference(current, current->up) < epsilon) {
            push(&stack, current->up);
        }
        if (current->down && !current->down->visited && color_difference(current, current->down) < epsilon) {

            push(&stack, current->down);
        }
        if (current->left && !current->left->visited && color_difference(current, current->left) < epsilon) {
            push(&stack, current->left);
        }
        if (current->right && !current->right->visited && color_difference(current, current->right) < epsilon) {
            push(&stack, current->right);
        }
    }
}


void find_components(Node* nodes, int width, int height, double epsilon) {
    int component = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Node* node = &nodes[y * width + x];
            if (!node->visited && node->r > 20) {
//                printf("%d %d %d\n", y, x, node->r);
                dfs(node, epsilon, component);
                component++;
            }
        }
    }
}

void color_components(Node* nodes, unsigned char *image, int width, int height) {
    srand(time(0));
    int max_component = 0;
    for (int i = 0; i < width * height; i++) {
        if (nodes[i].component > max_component) {
            max_component = nodes[i].component;
        }
    }
    printf("%d", max_component);

    unsigned char* colors = malloc((max_component + 1) * 3);
    for (int i = 0; i <= max_component; i++) {
        colors[i * 3 + 0] = rand() % 256;
        colors[i * 3 + 1] = rand() % 256;
        colors[i * 3 + 2] = rand() % 256;
    }

    for (int i = 0; i < width * height; i++) {
        Node* p = &nodes[i];
        if (p->r > 20) {
            image[4 * i + 0] = colors[p->component * 3 + 0];
            image[4 * i + 1] = colors[p->component * 3 + 1];
            image[4 * i + 2] = colors[p->component * 3 + 2];
            image[4 * i + 3] = 255;
        }
        else{
            image[4 * i + 0] = 0;
            image[4 * i + 1] = 0;
            image[4 * i + 2] = 0;
            image[4 * i + 3] = 255;
        }
    }

    free(colors);
}


void main_color_border(unsigned char* image, int w, int h, int epsilon) {
    Node* nodes = create_graph(image, &w, &h);
    find_components(nodes, w, h, epsilon);
    color_components(nodes, image, w, h);

    free(nodes);
}


int main() {
    int width = 0, height = 0;
    char *filename = "input.png";
    unsigned char *image = load_png_file(filename, &width, &height);

    for (int i = 0; i < width * height; i++) {
        unsigned char gr;
        gr = (image[4 * i + 1] + image[4 * i + 2] + image[4 * i + 3])/3 ;
        image[4*i + 1] = gr;
        image[4*i + 2] = gr;
        image[4*i + 3] = gr;
    }


    main_color_border(image, width, height, 3);
    char *output_filename = "output.png";
    lodepng_encode32_file(output_filename, image, width, height);

    free(image);
    return 0;
}
