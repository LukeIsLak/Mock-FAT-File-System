#ifndef DISKLIST
#define DISKLIST

#include "helperfunctions.h"

/**
 * @brief prints a directory entry struct to print to the standard output
 * @param dir_e a pointer to the directy entry struct to be printed
 */
void print_dir_entry(dir_entry* dir_e) {
    char type = (dir_e->status & 0b0000100)? 'D' : ((dir_e->status & 0b0000010)? 'F' : 'C');

    printf("%c %10d %30s %d/%d/%d %d:%d:%d\n", 
        type, ntohl(dir_e->size),
        dir_e->filename,
        ntohs(dir_e->create_time.year),
        dir_e->create_time.month,
        dir_e->create_time.day,
        dir_e->create_time.hour,
        dir_e->create_time.minute,
        dir_e->create_time.second);
}

/**
 * @brief prints a directory entry struct to print to the standard output
 * @param input_block_num the number to the 
 * @param address the address to the beggining of a file mapped in memory
 * @param s a pointer to a super block struct
 */
void generate_subdirectory(int input_block_num, void* address, s_block* s) {
    int cur_block_num = input_block_num;
    while (1) {
        for (int i = (cur_block_num*s->block_size); i < (cur_block_num*s->block_size)+s->block_size; i+=64) {
            dir_entry* dir_e = (dir_entry*)malloc(sizeof(dir_entry));
            memcpy(dir_e, address+i, 64);
            if (dir_e->status != 0) print_dir_entry(dir_e);
            free(dir_e);
        }
        int new_block_num = generate_fat_table(cur_block_num, address, s);
        cur_block_num = new_block_num;
        if (cur_block_num == -1) break;
    }
}

int disklist(int argc, char* argv[]) {

    /*Checks if we have 3 arguements*/
    if (argc != 3) {
        printf("ERROR - Expecting 3 Arguements - ./disklist <filesystem> <path>\n");
        return 1;
    }

    /*Open the file in read byte (plus additional flags) and checks if the file was opened correctly*/
    FILE* fptr = fopen(argv[1], "rb+");
    if (fptr == NULL) {
        printf("ERROR - Could not open file");
        return 1;
    }

    /*Initialize information of the file and create a superblock*/
    int fd = fileno(fptr);
    s_block* s = initialize_super_block(fd);
    struct stat f_buffer;
    int status = fstat(fd, &f_buffer);

    /*Check if things initialize correctly*/
    if (fd == -1 || status == -1) {
        printf("ERROR - %s\n", (fd == -1)? "Fileno failed" : "Fstat failed");
        return 1;
    }

    void* address = mmap(NULL, f_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (address == MAP_FAILED) {
        printf("ERROR - Could not map file to memory");
        return 1;
    }

    /*Count the total paths of the */
    int total_paths = count_subdirectories(argv[2]);

    /*If the total number of paths 0; i.e. invalid path*/
    if (total_paths == -1) {
        printf("Directory not listed\n");
        munmap(address, f_buffer.st_size);
        fclose(fptr);
        return 1;
    }

    /*If the total number of paths is > 0, find the corresponding block number of the last directory in the path*/
    int cur_block_num = s->r_dir_start;
    char* cur_dir = strtok(argv[2], "/");
    for (; total_paths > 0; total_paths--, cur_dir = strtok(NULL, "/")) {
        if (cur_dir == NULL) {
            printf("ERROR - Parsing String\n");
            break;
        }
        cur_block_num = find_subdirectory(cur_dir, cur_block_num, address, s, NULL, 'D');
        if (cur_block_num == -1) break;
    }

    /*Print out the directory starting at the block number specified*/
    if (cur_block_num != -1) generate_subdirectory(cur_block_num, address, s);
    munmap(address, f_buffer.st_size);
    fclose(fptr);
    return 0;
}

#endif