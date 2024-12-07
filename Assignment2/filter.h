#include <stdint.h>
#include <malloc.h>

#define PIXEL_WIDTH 3

double f_smooth[3][3] = {
        {1.0/9.0, 1.0/9.0, 1.0/9.0},
        {1.0/9.0, 1.0/9.0, 1.0/9.0},
        {1.0/9.0, 1.0/9.0, 1.0/9.0}
};

double f_sharpen[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
};

double f_edge[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}
};

double f_emboss[3][3] = {
        {-2, -1, 0},
        {-1, 1, 1},
        {0, 1, 2}
};



void apply_filter(unsigned char *pixel_data, double filter[3][3], int width, int height, int padding_size, unsigned char *output) {
    for (int i = 1; i < width - 1; i++) {
        for (int j = 1; j < height - 1; j++) {
            double sum[3] = {0, 0, 0};
            for (int p = -1; p <= 1; p++) {
                for (int q = -1; q <= 1; q++) {
                    int offset = (i - p) * (width * PIXEL_WIDTH + padding_size) + (j - q) * PIXEL_WIDTH;
                    double value = filter[p + 1][q + 1];
                    sum[0] += pixel_data[offset] * value;
                    sum[1] += pixel_data[offset + 1] * value;
                    sum[2] += pixel_data[offset + 2] * value;
                }
            }


            for (int k = 0; k < 3; k++) {
                if (sum[k] < 0) {
                    sum[k] = 0;
                } else if (sum[k] > 255) {
                    sum[k] = 255;
                }
            }

            if (j == 1) {
                printf("sum: %f %f %f\n", sum[0], sum[1], sum[2]);
            }

            int offset = i * (width * PIXEL_WIDTH + padding_size) + j * PIXEL_WIDTH;
            output[offset] = (unsigned char) sum[0];
            output[offset + 1] = (unsigned char) sum[1];
            output[offset + 2] = (unsigned char) sum[2];
        }
    }
}




