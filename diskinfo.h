#ifndef DISKINFO
#define DISKINFO 

#include "helperfunctions.h"

/**
 * @brief prints the contents of a superblock
 * @param super_block a superblock pointer that contains information of the superblock
 */
void print_super_block(s_block* super_block) {
    printf("Super block information:\n");
    printf("Block size: %u\n", super_block->block_size);
    printf("Block count: %u\n", super_block->block_count);
    printf("FAT starts: %u\n", super_block->FAT_starts);
    printf("FAT blocks: %u\n", super_block->FAT_blocks);
    printf("Root directory start: %u\n", super_block->r_dir_start);
    printf("Root directory blocks: %u\n", super_block->r_dir_blocks);
}

/**
 * @brief prints the FAT information of a filesystem
 * @param fd is the filenumber of the file we would like to print information from, should support a FAT structure
 * @param s is a pointer to a superblock and files within the superblock
 */
void print_FAT_information(int fd, s_block* s) {
    int free_blocks, allocated_blocks, reserved_blocks = 0;
    read_FAT_information(fd, s, &free_blocks, &allocated_blocks, &reserved_blocks);
    printf("\n");
    printf("FAT information:\n");
    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", allocated_blocks);
    printf("Allocated Blocks: %d\n", reserved_blocks);

}  

/**
 * @brief prints the block information of a given input file that supports a FAT filesystem
 */
int diskinfo(int argc, char* argv[]) {

    /*Checks if we have 2 arguements*/
    if (argc != 2) {
        printf("ERROR - Expecting two arguements - ./diskinfo <filesystem>\n");
        return 1;
    }

    /*Open the file in read byte (plus additional flags) and checks if the file was opened correctly*/
    FILE* fptr = fopen(argv[1], "rb+");
    if (fptr == NULL) {
        printf("ERROR - Could not read from file; File may not exist");
        return 1;
    }

    /*Create a superblock struct with the given information*/
    s_block* super_block = initialize_super_block(fileno(fptr));
    if (super_block == NULL) {
        fclose(fptr);
        return 1;
    }
    
    /*Print the information required, handle the exiting of a file*/
    print_super_block(super_block);
    print_FAT_information(fileno(fptr), super_block);

    free(super_block);
    fclose(fptr);
    return 0;
}

#endif