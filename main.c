#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

char *load_png_file(const char *filename, int *width, int *height) {
    unsigned char *image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return NULL;
    }
    return (image);
}

void floodFill(unsigned char *image, int x, int y, int newColor1, int newColor2, int newColor3, int oldColor, int width,
               int height) {
    int dx[] = {-1, 0, 1, 0};
    int dy[] = {0, 1, 0, -1};

    int *stackX = malloc(width * height * sizeof(int));
    int *stackY = malloc(width * height * sizeof(int));
    long top = 0;

    stackX[top] = x;
    stackY[top] = y;
    top++;

    while (top > 0) {
        top--;
        int px = stackX[top];
        int py = stackY[top];

        if (px < 0 || px >= width || py < 0 || py >= height)
            continue;

        if (image[(py * width + px) * 4] > oldColor)
            continue;

        image[(py * width + px) * 4] = newColor1;
        image[(py * width + px) * 4 + 1] = newColor2;
        image[(py * width + px) * 4 + 2] = newColor3;

        for (int i = 0; i < 4; i++) {
            int nx = px + dx[i];
            int ny = py + dy[i];
            if (nx > 0 && nx < width && ny > 0 && ny < height && image[(ny * width + nx) * 4] <= oldColor) {
                stackX[top] = nx;
                stackY[top] = ny;
                top++;
            }
        }
    }
    free(stackX);
    free(stackY);
}


void colorComponents(unsigned char *image, int width, int height, int epsilon) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (image[4 * (y * width + x)] < epsilon) {
                floodFill(image, x, y, rand() % (255 - epsilon * 2) + epsilon * 2, \
                rand() % (255 - epsilon * 2) + epsilon * 2, \
                rand() % (255 - epsilon * 2) + epsilon * 2, epsilon, width, height);

            }
        }
    }
    char *output_filename = "output.png";
    lodepng_encode32_file(output_filename, image, width, height);
}


int main() {
    int width = 0, height = 0;
    char *filename = "input.png";
    char *image = load_png_file(filename, &width, &height);
    unsigned char *result = malloc(width * height * 4 * sizeof(unsigned char));

    int gx[3][3] = {{-1, 0, 1},
                    {-2, 0, 2},
                    {-1, 0, 1}};
    int gy[3][3] = {{1,  2,  1},
                    {0,  0,  0},
                    {-1, -2, -1}};


    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int index = ((y + dy) * width + (x + dx)) * 4;
                    int gray = (image[index] + image[index + 1] + image[index + 2]) / 3;
                    sumX += gx[dy + 1][dx + 1] * gray;
                    sumY += gy[dy + 1][dx + 1] * gray;
                }
            }
            int magnitude = sqrt(sumX * sumX + sumY * sumY);
            if (magnitude > 255) magnitude = 255;
            if (magnitude < 0) magnitude = 0;

            int resultIndex = (y * width + x) * 4;
            result[resultIndex] = (unsigned char) magnitude;
            result[resultIndex + 1] = (unsigned char) magnitude;
            result[resultIndex + 2] = (unsigned char) magnitude;
            result[resultIndex + 3] = image[resultIndex + 3];
        }
    }

    for (int i = 0; i < width * height * 4; i++) {
        image[i] = result[i];
    }

    colorComponents(image, width, height, 40);

    free(result);
    free(image);
    return 0;
}

