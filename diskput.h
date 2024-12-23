#ifndef DISKPUT
#define DISKPUT

#include "helperfunctions.h"

int diskput(int argc, char* argv[]) {
    if (argc != 4) {
        printf("ERROR - Expecting 4 Arguements - ./diskget <filesystem> <path> <output>\n");
        return 1;
    }

    FILE* fptrIn = fopen(argv[2], "rb+");
    FILE* fptrOut = fopen(argv[1], "rb+");
    if (fptrIn == NULL || fptrOut == NULL) {
        if (fptrOut == NULL) printf("ERROR - Could not open file\n");
        else printf("File not found.\n");
        return 1;
    }

    int fdIn = fileno(fptrIn);
    struct stat f_bufferIn;
    int statusIn = fstat(fdIn, &f_bufferIn);

    int fdOut = fileno(fptrOut);
    struct stat f_bufferOut;
    s_block* s = initialize_super_block(fdOut);
    int statusOut = fstat(fdOut, &f_bufferOut);

    if (fdIn == -1 || statusIn == -1) {
        printf("ERROR - Input %s\n", (fdIn == -1)? "Fileno failed" : "Fstat failed");
        return 1;
    }
    if (fdOut == -1 || statusOut == -1) {
        printf("ERROR - Output %s\n", (fdOut == -1)? "Fileno failed" : "Fstat failed");
        return 1;
    }

    void* address_disk = mmap(NULL, f_bufferOut.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdOut, 0);
    void* address_file = mmap(NULL, f_bufferOut.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdIn, 0);
    
    if (address_disk == MAP_FAILED || address_file == MAP_FAILED) {
        printf("ERROR - Could not map file to memory");
        return 1;
    }

    int free_blocks, allocated_blocks, reserved_blocks = 0;
    read_FAT_information(fdOut, s, &free_blocks, &reserved_blocks, &allocated_blocks);
    if (f_bufferIn.st_size > free_blocks * s->block_size) {
        printf("No room to store disk\n");
        return 1;
    }

    int total_paths = count_subdirectories(argv[3]) - 1;
    if (total_paths < 0) {
        printf("ERROR - No File Listed\n");
        munmap(address_disk, f_bufferOut.st_size);
        munmap(address_file, f_bufferIn.st_size);
        fclose(fptrIn);
        fclose(fptrOut);
        return 1;
    }

    int cur_block_num = s->r_dir_start;
    char* path_copy = strdup(argv[3]);
    char* filename = get_filename(path_copy);
    char* cur_dir = (total_paths == 0) ? strtok(argv[3], "/") : "";
    cur_block_num = traverse_to_holding_directory(total_paths, address_disk, argv[3], cur_dir, s, NULL, 3);
    if (cur_block_num != -1) write_to_disk(filename, address_disk, address_file, f_bufferIn, cur_block_num, s);
    else printf("File not found.\n");
    munmap(address_disk, f_bufferOut.st_size);
    munmap(address_file, f_bufferIn.st_size);
    fclose(fptrIn);
    fclose(fptrOut);
    return 0;

}

#endif