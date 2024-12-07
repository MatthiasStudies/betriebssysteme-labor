#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>

#define BMP_HEADER_SIZE 54
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



unsigned char* apply_kernel(const unsigned char *pixel_data, double kernel[3][3], int width, int height, int padding_size) {
    unsigned char *output = (unsigned char *) malloc(width * height * PIXEL_WIDTH);
    if (output == NULL) {
        printf("Error: Failed to allocate memory for output image\n");
        return NULL;
    }

    for (int i = 1; i < width - 1; i++) {
        for (int j = 1; j < height - 1; j++) {
            double sum[3] = {0, 0, 0};
            for (int p = -1; p <= 1; p++) {
                for (int q = -1; q <= 1; q++) {
                    int offset = (i - p) * (width * PIXEL_WIDTH + padding_size) + (j - q) * PIXEL_WIDTH;
                    double value = kernel[p + 1][q + 1];
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

            int offset = i * (width * PIXEL_WIDTH + padding_size) + j * PIXEL_WIDTH;
            output[offset] = (unsigned char) sum[0];
            output[offset + 1] = (unsigned char) sum[1];
            output[offset + 2] = (unsigned char) sum[2];
        }
    }

    return output;
}

typedef struct {
    unsigned char *header;
    unsigned char *pixel_data;
    int width;
    int height;
    int padding_size;
    uint32_t image_size;
} bmpImage;

bmpImage *read_bmp_image(char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("Error: Failed to open file\n");
        return NULL;
    }

    unsigned char *bmp_header = (unsigned char *) malloc(BMP_HEADER_SIZE);

    ssize_t bytes_read = read(fd, bmp_header, BMP_HEADER_SIZE);
    if (bytes_read != BMP_HEADER_SIZE) {
        printf("Error: Invalid bitmap header\n");
        return NULL;
    }

    if (bmp_header[0] != 'B' || bmp_header[1] != 'M') {
        printf("Error: It's not a bitmap image\n");
        return NULL;
    }

    uint32_t image_size = *(uint32_t *) &bmp_header[2];
    int32_t width = *(int32_t *) &bmp_header[18];
    int32_t height = *(int32_t *) &bmp_header[22];
    uint16_t bit_depth = *(uint16_t *) &bmp_header[28];
    int32_t compression = *(int32_t *) &bmp_header[30];

    if (compression != 0) {
        printf("Error: Only uncompressed bitmaps are supported\n");
        return NULL;
    }

    if (width < 3 || height < 3) {
        printf("Error: Invalid image size\n");
        return NULL;
    }

    if (bit_depth != 24) {
        printf("Error: Only 24-bit bitmaps are supported\n");
        return NULL;
    }

    unsigned char *pixel_data = (unsigned char *) malloc(image_size - BMP_HEADER_SIZE);
    if (pixel_data == NULL) {
        printf("Error: Failed to allocate memory for pixel data\n");
        return NULL;
    }

    bytes_read = read(fd, pixel_data, image_size - BMP_HEADER_SIZE);
    if (bytes_read != image_size - BMP_HEADER_SIZE) {
        printf("Error: Failed to read pixel data\n");
        return NULL;
    }

    int pixel_bytes_per_row = width * PIXEL_WIDTH;
    int total_bytes_per_row = (pixel_bytes_per_row + PIXEL_WIDTH) & ~PIXEL_WIDTH;
    int padding_size = total_bytes_per_row - pixel_bytes_per_row;

    bmpImage *bmp = (bmpImage *) malloc(sizeof(bmpImage));
    bmp->header = bmp_header;
    bmp->pixel_data = pixel_data;
    bmp->width = width;
    bmp->height = height;
    bmp->padding_size = padding_size;
    bmp->image_size = image_size;

    close(fd);

    return bmp;
}

void free_bmp_file(bmpImage *bmp) {
    free(bmp->header);
    free(bmp->pixel_data);
    free(bmp);
}

int run_filter(bmpImage *image, char *filter) {
    unsigned char *pixel_data = image->pixel_data;
    int width = image->width;
    int height = image->height;
    int padding_size = image->padding_size;
    unsigned char *output;

    if (strcmp(filter, "smooth") == 0) {
        output = apply_kernel(pixel_data, f_smooth, width, height, padding_size);
    } else if (strcmp(filter, "sharp") == 0) {
        output = apply_kernel(pixel_data, f_sharpen, width, height, padding_size);
    } else if (strcmp(filter, "edge") == 0) {
        output = apply_kernel(pixel_data, f_edge, width, height, padding_size);
    } else if (strcmp(filter, "emboss") == 0) {
        output = apply_kernel(pixel_data, f_emboss, width, height, padding_size);
    } else {
        printf("Error: Unknown filter\n");
        return -1;
    }

    printf("Filter %s applied successfully\n", filter);

    free(image->pixel_data);
    image->pixel_data = output;

    return 0;
}

int crop_image(bmpImage* image) {
    int width = image->width;
    int height = image->height;
    int padding_size = image->padding_size;
    unsigned char* bmp_header = image->header;

    int cropped_width = width - 2;
    int cropped_height = height - 2;
    int cropped_pixel_bytes_per_row = cropped_width * PIXEL_WIDTH;
    int cropped_padding_size = ((cropped_pixel_bytes_per_row + PIXEL_WIDTH) & ~PIXEL_WIDTH) - cropped_pixel_bytes_per_row;
    int cropped_image_size = cropped_width * cropped_height * PIXEL_WIDTH + cropped_height * padding_size; // why padding_size not cropped_padding_size?????

    unsigned char* cropped_pixel_data = (unsigned char*) malloc(cropped_image_size);
    if (cropped_pixel_data == NULL) {
        printf("Error: Failed to allocate memory for cropped pixel data\n");
        return 1;
    }

    unsigned char* pixel_data = image->pixel_data;

    for(int row = 1; row < height - 1; row++) {
        for(int col = 1; col < width - 1; col++) {
            int offset = row  * (width * PIXEL_WIDTH + padding_size) + col * PIXEL_WIDTH;
            int new_offset = (row - 1) * (cropped_width * PIXEL_WIDTH + cropped_padding_size) + (col - 1) * PIXEL_WIDTH;

            cropped_pixel_data[new_offset] = pixel_data[offset];
            cropped_pixel_data[new_offset + 1] = pixel_data[offset + 1];
            cropped_pixel_data[new_offset + 2] = pixel_data[offset + 2];
        }
    }

    // Update the bitmap header
    *(uint32_t*) &bmp_header[2] = cropped_image_size ;
    *(uint32_t*) &bmp_header[18] = cropped_width;
    *(uint32_t*) &bmp_header[22] = cropped_height;

    free(image->pixel_data);
    image->pixel_data = cropped_pixel_data;

    return 0;
}

int save_image(bmpImage* image, char* filename) {
    if (image == NULL) {
        return 1;
    }

    int fd = open(filename, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        printf("Error: Failed to create output file\n");
        return 1;
    }

    unsigned char* bmp_header = image->header;
    unsigned char* pixel_data = image->pixel_data;
    uint32_t image_size = image->image_size;

    write(fd, bmp_header, BMP_HEADER_SIZE);

    long bytes_read = write(fd, pixel_data, image_size - BMP_HEADER_SIZE);
    if (bytes_read != image_size - BMP_HEADER_SIZE) {
        printf("Error: Failed to write pixel data\n");
        return 1;
    }

    close(fd);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <filename> <filter>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];
    char *filter = argv[2];

    bmpImage *bmp = read_bmp_image(filename);
    if (bmp == NULL) {
        return 1;
    }

    int status = run_filter(bmp, filter);
    if (status != 0) {
        free_bmp_file(bmp);
        return status;
    }

    crop_image(bmp);

    status = save_image(bmp, "output.bmp");
    if (status != 0) {
        free_bmp_file(bmp);
        return status;
    }

    free_bmp_file(bmp);
    return 0;
}