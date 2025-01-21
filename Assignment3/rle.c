#include "./include/rle.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct RLENode {
    uint64_t count;
    struct RLENode* next;
} RLENode;

struct RLE {
    RLENode* head;
    RLENode* tail;
    uint64_t size;
};

static void append_to_rle(RLE* rle, uint64_t count) {
    RLENode* node = malloc(sizeof(RLENode));
    node->count = count;
    node->next = NULL;

    if (rle->tail) {
        rle->tail->next = node;
    } else {
        rle->head = node;
    }

    rle->tail = node;
    rle->size += 1;
}

/**
 * Create a new RLE data structure. The RLE data structure is a linked
 * list of RLENodes. Each RLENode contains a count of the number of
 * consecutive bits that are the same. The RLE data structure is
 * initialized with an entry.
 * @return a pointer to the RLE data structure
 */
RLE* create_rle() {
    RLE* rle = malloc(sizeof(RLE));
    rle->head = NULL;
    rle->tail = NULL;
    rle->size = 0;

    append_to_rle(rle, 0); // Start with a count of 0 bits

    return rle;
}

/**
 * Delete the RLE data structure and all of its nodes. This function
 * should be called when the RLE data structure is no longer needed.
 * @param rle the RLE data structure to delete
 */
void delete_rle(RLE* rle) {
    RLENode* node = rle->head;
    while (node) {
        RLENode* next = node->next;
        free(node);
        node = next;
    }
    free(rle);
}

static bool pop_head_rle(RLE* rle, uint64_t* count) {
    if (!rle->head) {
        return false;
    }

    RLENode* node = rle->head;
    *count = node->count;

    rle->head = node->next;
    if (rle->tail == node) {
        rle->tail = NULL;
    }

    free(node);
    rle->size -= 1;

    return true;
}

static uint64_t get_rle_total_count(RLE* rle) {
    uint64_t total = 0;
    RLENode* node = rle->head;
    while (node) {
        total += node->count;
        node = node->next;
    }
    return total;
}

/**
 * Fill rle counts with the provided data. The data should be treated as
 * binary data, not as a string, so the data is not null-terminated.
 *
 * This function counts the number of consecutive bits that are the
 * same and appends that count to the rle. The first entry of the rle
 * should always be the number of consecutive 0s at the beginning of
 * the data.
 *
 * For example, if the start of data is "00001111", then the rle should contain
 * two entries 4 and 4.
 *
 * If the start of data is "11110000", then the rle should contain three entries,
 * 0, 4, and 4
 * @param rle Will be filled with counts
 * @param data Source data, treated as binary data
 * @param size Size of the source data
 */
void encode_rle(RLE* rle, const char* data, size_t size) {
    uint8_t counting_bit = (rle->size & 1) ^ 1;

    for (size_t i = 0; i < size; i++) {
        for (int8_t j = 7; j >= 0; j--) {
            uint8_t current_bit = (data[i] >> j) & 1;
            if (current_bit == counting_bit) {
                rle->tail->count++;;
            } else {
                append_to_rle(rle, 1);
                counting_bit ^= 1; // Switch between 0 and 1
            }
        }
    }
}

/**
 * Decodes the rle to the appropriate binary data. The returned data
 * should be treated as binary data, not as a string, so the data is
 * not null-terminated.
 * @param rle assumed to be filled with counts
 * @param size will be set by this function and is the size of the returned data
 * @return binary data
 */
char* decode_rle(RLE* rle, size_t* size) {
    uint64_t total_bits = get_rle_total_count(rle);
    *size = (total_bits + 7) >> 3; // Round up to the nearest byte

    char* output = calloc(*size, sizeof(char));
    if (!output) {
        return NULL;
    }

    uint64_t count;
    size_t byte_index = 0;

    uint8_t bit = 0;
    uint8_t bit_index = 7;

    while (byte_index < *size && pop_head_rle(rle, &count)) {
        while (count > 0) {
            output[byte_index] |= bit << bit_index;
            if (bit_index == 0) {
                byte_index++;
                bit_index = 7;
            } else {
                bit_index--;
            }
            count--;
        }
        bit ^= 1; // Switch between 0 and 1
    }

    return output;
}

void print_rle(RLE* rle, uint8_t counts_per_line) {
    RLENode* node = rle->head;
    printf("{\n");
    int counter = 0;
    while (node) {
        printf("  %lu", node->count);
        if (node->next) printf(", "); // print comma only if this isn't the last node
        if (counts_per_line > 0 && ++counter >= counts_per_line) {
            printf("\n");
            counter = 0;
        }

        node = node->next;
    }
    printf(" }");
    printf("\n");
}

void printChar(char c) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (c >> i) & 1);
    }
    printf("\n");
}
void printChar4(char c) {
    for (int i = 3; i >= 0; i--) {
        printf("%d", (c >> i) & 1);
    }
    printf("\n");
}

size_t insert_bit_count(char *data, char bit, uint64_t count, size_t i, bool complete) {
    char encoded = bit << 3; // Extend the bit to 4 bits

    if (count < 4) {
        encoded |= count;

        // Check whether to append the count to the previous byte or create a new byte
        if (complete) {
            data[i++] = encoded << 4;
        } else {
            data[i - 1] |= encoded;
        }
    } else {
        encoded |= 0b01000000; // Set the 2nd bit to 1 to indicate an extended count (6 bits)
        encoded |= count;

        // If necessary, split the count into two bytes
        if (complete) {
            data[i++] = encoded;
        } else {
            data[i - 1] |= (encoded >> 4) & 0xFF;
            data[i++] = encoded << 4;
        }
    }

    return i;
}

char* serialize_rle(RLE *rle, size_t* size) {
    RLENode *node = rle->head;
    *size = rle->size * sizeof(uint64_t);

    char *data = malloc(*size);
    if (!data) {
        return NULL;
    }

    size_t i = 0;
    char bitType = 0;
    bool complete = true;
    while (node) {
        uint64_t nodeCount = node->count;

        bitType = nodeCount >> 63;
        char bitCount = 0;

        for(int j = 0; j < 64; j++) {
            // Get the current bit
            char next = nodeCount >> 63;
            if (next == bitType) {
                // Count the number of consecutive bits
                bitCount++;
            } else {
                // Insert the coded bit count into the output data
                i = insert_bit_count(data, bitType, bitCount, i, complete);
                if (bitCount < 4) {
                    complete = !complete;
                }

                bitCount = 1;
                bitType = next;
            }
            nodeCount <<= 1;
        }

        i = insert_bit_count(data, bitType, bitCount, i, complete);
        if (bitCount < 4) {
            complete = !complete;
        }

        node = node->next;
    }
    printf("Bin Data\n");
    for (int j = 0; j < 10; j++) {
        printChar(data[j]);
    }

    return data;

}


void deserialize_rle(RLE *rle, const char *data, size_t size) {
    rle->size = size / sizeof(uint64_t);
    rle->head = NULL;
    rle->tail = NULL;

    size_t i = 0;
    printf("size: %lu\n", size);

    bool reading = false;
    bool isExtended = false;
    char targetBit = 0;
    uint8_t bitCount = 0;
    char remainingBits = 0;
    char totalBitsRead = 0;

    uint64_t count = 0;

    while (i < size) {
        for (int j = 7; j >= 0; j--) {
            char bit = (data[i] >> j) & 1;

            if (!reading) {
                // Get the next bit type and number of bits for the count
                targetBit = bit;
                j--;
                isExtended = (data[i] >> j) & 1;
                if (isExtended) {
                    remainingBits = 6;
                } else {
                    remainingBits = 2;
                }
                reading = true;
                continue;
            }

            // Append the bit to the bitCount
            bitCount <<= 1;
            bitCount |= bit;
            remainingBits--;

            if (remainingBits == 0) {
                totalBitsRead += bitCount;

                for (int k = 0; k < bitCount; k++) {
                    count <<= 1;
                    count |= targetBit;
                }

                if (totalBitsRead == 64) {
                    append_to_rle(rle, count);
                    count = 0;
                    totalBitsRead = 0;
                }

                bitCount = 0;
                reading = false;
            }
        }
        i++;
    }
}
