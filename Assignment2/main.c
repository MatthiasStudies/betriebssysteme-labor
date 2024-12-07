#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "filter.h"

#define BMP_HEADER_SIZE 54
#define PIXEL_WIDTH 3

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <filename> <filter>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];
    char *filter = argv[2];

    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("Error: Failed to open file\n");
        return 1;
    }

    unsigned char bmp_header[BMP_HEADER_SIZE];

    ssize_t bytes_read = read(fd, bmp_header, BMP_HEADER_SIZE);
    if (bytes_read != BMP_HEADER_SIZE) {
        printf("Error: Invalid bitmap header\n");
        return 1;
    }

    if (bmp_header[0] != 'B' || bmp_header[1] != 'M') {
        printf("Error: It's not a bitmap image\n");
        return 1;
    }

    uint32_t image_size = *(uint32_t *) &bmp_header[2];
    int32_t width = *(int32_t *) &bmp_header[18];
    int32_t height = *(int32_t *) &bmp_header[22];
    uint16_t bit_depth = *(uint16_t *) &bmp_header[28];

    if (bit_depth != 24) {
        printf("Error: Only 24-bit bitmaps are supported\n");
        return 1;
    }

    unsigned char *pixel_data = (unsigned char *) malloc(image_size - BMP_HEADER_SIZE);
    if (pixel_data == NULL) {
        printf("Error: Failed to allocate memory for pixel data\n");
        return 1;
    }

    bytes_read = read(fd, pixel_data, image_size - BMP_HEADER_SIZE);
    if (bytes_read != image_size - BMP_HEADER_SIZE) {
        printf("Error: Failed to read pixel data\n");
        return 1;
    }

    int pixel_bytes_per_row = width * PIXEL_WIDTH;
    int total_bytes_per_row = (pixel_bytes_per_row + PIXEL_WIDTH) & ~PIXEL_WIDTH;
    int padding_size = total_bytes_per_row - pixel_bytes_per_row;

    printf("Reading image with width %d, height %d and bit depth %d (padding: %d)\n", width, height, bit_depth,
           padding_size);
    if (strcmp(filter, "read") == 0){
        printf("File %s:\n", filename);
        printf("Width: %d\n", width);
        printf("Height: %d\n", height);
        printf("Bit depth: %d\n", bit_depth);
        printf("Padding size: %d\n", padding_size);
        printf("Image size: %d\n", image_size);
        return 0;
    }else if (strcmp(filter, "smooth") == 0) {
        printf("Applying smooth filter\n");
        apply_filter(pixel_data, f_smooth, width, height, padding_size);
    } else if (strcmp(filter, "sharp") == 0) {
        printf("Applying sharp filter\n");
        apply_filter(pixel_data, f_sharpen, width, height, padding_size);
    } else if (strcmp(filter, "edge") == 0) {
        printf("Applying edge filter\n");
        apply_filter(pixel_data, f_edge, width, height, padding_size);
    } else if (strcmp(filter, "emboss") == 0) {
        printf("Applying emboss filter\n");
        apply_filter(pixel_data, f_emboss, width, height, padding_size);
    } else {
        printf("Error: Unknown filter\n");
        return 1;
    }

    // Create a new file to write the modified image
    char *output_filename = "output.bmp";
    int output_fd = open(output_filename, O_CREAT | O_RDWR, 0666);
    if (output_fd < 0) {
        printf("Error: Failed to create output file\n");
        return 1;
    }


    int cropped_width = width - 2;
    int cropped_height = height - 2;
    int cropped_pixel_bytes_per_row = cropped_width * PIXEL_WIDTH;
    int cropped_padding_size = ((cropped_pixel_bytes_per_row + PIXEL_WIDTH) & ~PIXEL_WIDTH) - cropped_pixel_bytes_per_row;
    int cropped_image_size = cropped_width * cropped_height * PIXEL_WIDTH + cropped_height * padding_size; // why padding_size not cropped_padding_size?????

    unsigned char* cropped_pixel_data = (unsigned char*) malloc(cropped_image_size);

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

    write(output_fd, bmp_header, BMP_HEADER_SIZE);

    bytes_read = write(output_fd, cropped_pixel_data, cropped_image_size - BMP_HEADER_SIZE);
    if (bytes_read != cropped_image_size - BMP_HEADER_SIZE) {
        printf("Error: Failed to write pixel data\n");
        return 1;
    }

    close(fd);
    close(output_fd);

    // Free the allocated memory
    free(pixel_data);
    free(cropped_pixel_data);

    return 0;
}