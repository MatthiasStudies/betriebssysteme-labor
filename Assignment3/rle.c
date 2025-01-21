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
    char o = bit << 3;
    if (count < 4) {
        printf("small: %d\n", count);
        o |= count;

        if (complete) {
            data[i++] = o << 4;
        } else {
            data[i - 1] |= o;
//            i++;
        }
//        printf("Existing: \n");
//        printChar(data[i]);
        printChar4(o);
    } else {
        printf("big: %d (%d)\n", count, complete);
        o |= 0b01000000;
        o |= count;

        if (complete) {
            data[i++] = o;
        } else {
//            printf("exisiting: ");
//            printChar(data[i - 1]);
            data[i - 1] |= (o >> 4) & 0xFF;
            data[i++] = o << 4;
        }

//                    data[i++] = o;
        printChar(o);
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
    char type = 0;
    bool complete = true;
    while (node) {
        uint64_t count = node->count;

        type = count >> 63;
        char c = 0;

        printf("\ncount: %lu\n", count);

        // 2 -> 0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0010

        for(int j = 0; j < 64; j++) {
            char next = count >> 63;
            if (next == type) {
                c++;
            } else {
                i = insert_bit_count(data, type, c, i, complete);
                if (c < 4) {
                    complete = !complete;
                }
//                char o = type << 3;
//                if (c < 4) {
//                    printf("small: %d\n", c);
//                    o |= c;
//
//                    if (complete) {
//                        data[i++] = o << 4;
//                        complete = false;
//                    } else {
//                        data[i] |= o;
//                        complete = true;
//                    }
//                    printChar4(o);
//                } else {
//                    printf("big: %d\n", c);
//                    o |= 0b01000000;
//                    o |= c;
//
//                    if (complete) {
//                        data[i++] = o;
//                    } else {
//                        data[i] |= o << 4;
//                        data[i + 1] = o >> 4;
//                    }
//
////                    data[i++] = o;
//                    printChar(o);
//                }
                c = 1;
                type = next;
            }
            count <<= 1;
        }

        if (c > 0) {
//            printf("insert: %d, %d, %zu, %d\n", type, c, i, complete);
            i = insert_bit_count(data, type, c, i, complete);
            if (c < 4) {
                complete = !complete;
            }
//            char o = type << 3;
//            if (c < 4) {
//                printf("small: %d\n", c);
//                o |= c;
//
//                if (complete) {
//                    data[i++] = o << 4;
//                    complete = false;
//                } else {
//                    data[i] |= o;
//                    complete = true;
//                }
//                printChar4(o);
//            } else {
//                printf("big: %d\n", c);
//                o |= 0b01000000;
//                o |= c;
//
//                if (complete) {
//                    data[i++] = o;
//                } else {
//                    data[i] |= o << 4;
//                    data[i + 1] = o >> 4;
//                }
//
////                    data[i++] = o;
//                printChar(o);
//            }
        }

//        if (i > 5) {
//            break;
//        }




//    break;




//        data[i++] = count & 0xFF;
//        data[i++] = (count >> 8) & 0xFF;
//        data[i++] = (count >> 16) & 0xFF;
//        data[i++] = (count >> 24) & 0xFF;
//        data[i++] = type;
//
//        printf("count: %zu %lu\n", i, count);
//
        node = node->next;
//
        type ^= 1;
    }
    printf("Bin Data\n");
    for (int j = 0; j < 10; j++) {
        printChar(data[j]);
    }

    return data;

}


void deserialize_rle(RLE *rle, const char *data, size_t size) {

//    char *data = malloc(sizeof(char) * 2);
//    data[0] = 0b01111110;
//    data[1] = 0b10010001;
//    size = 2;

    rle->size = size / sizeof(uint64_t);
    rle->head = NULL;
    rle->tail = NULL;

    size_t i = 0;
    printf("size: %lu\n", size);
    bool complete = true;

    size_t global = 0;

    bool reading = false;
    bool isExtended = false;
    char targetBit = 0;
    uint8_t buildCount = 0;
    char remainingBits = 0;
    char totalBitsRead = 0;

    uint64_t count = 0;

    while (i < size) {


        for (int j = 7; j >= 0; j--) {
            char bit = (data[i] >> j) & 1;

            printf("%d\n", bit);

            if (!reading) {
//                printf("Setting target bit: %d\n", bit);
                targetBit = bit;
                j--;
                isExtended = (data[i] >> j) & 1;
                printf("%d\n", isExtended);
//                printf("Is extended: %d\n", isExtended);

                if (isExtended) {
                    remainingBits = 6;
                } else {
                    remainingBits = 2;
                }

//                printf("target bit: %d %d\n", targetBit, isExtended);

                reading = true;
                continue;
            }



            buildCount <<= 1;
            buildCount |= bit;
            remainingBits--;

            if (remainingBits == 0) {
                totalBitsRead += buildCount;

                printf("Adding %lu * %d to count\n", buildCount, targetBit);

                for (int k = 0; k < buildCount; k++) {
                    count <<= 1;
                    count |= targetBit;
                }

                if (totalBitsRead == 64) {
                    printf("Constructed count: %lu\n---\n", count);
                    append_to_rle(rle, count);
                    count = 0;
                    totalBitsRead = 0;
                }

                buildCount = 0;
                reading = false;
            }
        }
        i++;


//        printf("count: %zu %lu\n", i, count);



//        append_to_rle(rle, count);
    }
}
