#ifndef DISKGET
#define DISKGET

#include "helperfunctions.h"

/**
 * @brief write a file in filesystem to a file outputed to the local directory
 * @param input path of the file we wish to print
 * @param filename string of the output filename
 * @param start_block_num the start of the block number we wish to start printing
 * @param address the address to the beggining of a file mapped in memory
 * @param s a pointer to a super block struct
 */
void write_to_file(char* input, char* filename, int start_block_num, void* address, s_block* s) {
    dir_entry* e = (dir_entry*)malloc(sizeof(dir_entry));
    int cur_block_num = find_subdirectory(input, start_block_num, address, s, e, 'F');
    if (cur_block_num == -1) {
        printf("File not found.\n");
        free(e);
        return;
    }
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1 || ftruncate(fd, ntohl(e->size)) == -1) {
        printf((fd == -1)? "ERROR - Could not open file\n" : "ERROR - Could not allocated enough memory to new file\n");
        free(e);
        close(fd);
        return;
    }

    char* file_addr = mmap(NULL, ntohl(e->size), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int copied_values = 0;
    int cur_block_copied_values = 0;
    while (cur_block_num != -1 && copied_values+cur_block_copied_values < ntohl(e->size)) {
        strncpy(file_addr+copied_values+cur_block_copied_values, (char*)address+(s->block_size * cur_block_num)+cur_block_copied_values, 1);
        cur_block_copied_values++;

        if (cur_block_copied_values == s->block_size) {
            copied_values+=cur_block_copied_values;
            cur_block_copied_values = 0;
            cur_block_num = generate_fat_table(cur_block_num, address, s);
        }

    }
    munmap(file_addr, ntohl(e->size));
    free(e);
    close(fd);
    return;

}

int diskget(int argc, char* argv[]) {
    if (argc != 4) {
        printf("ERROR - Expecting 4 Arguements - ./diskget <filesystem> <path> <output>\n");
        return 1;
    }

    FILE* fptr = fopen(argv[1], "rb+");
    if (fptr == NULL) {
        printf("ERROR - Could not open input file\n");
        return 1;
    }

    int fd = fileno(fptr);
    s_block* s = initialize_super_block(fd);
    struct stat f_buffer;
    int status = fstat(fd, &f_buffer);

    if (fd == -1 || status == -1) {
        printf("ERROR - %s\n", (fd == -1)? "Fileno failed" : "Fstat failed");
        return 1;
    }

    void* address = mmap(NULL, f_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (address == MAP_FAILED) {
        printf("ERROR - Could not map file to memory");
        return 1;
    }

    int total_paths = count_subdirectories(argv[2])-1;
    if (total_paths < 0) {
        printf("ERROR - No File Listed\n");
        munmap(address, f_buffer.st_size);
        fclose(fptr);
        return 1;
    }

    int cur_block_num = s->r_dir_start;
    char* path_copy = strdup(argv[2]);
    char* filename = get_filename(path_copy);
    char* cur_dir = (total_paths == 0) ? strtok(argv[2], "/") : "";
    cur_block_num = traverse_to_holding_directory(total_paths, address, argv[2], cur_dir, s, NULL, 0);
    if (cur_block_num != -1) write_to_file(filename, argv[3], cur_block_num, address, s);
    else printf("File not found.\n");
    munmap(address, f_buffer.st_size);
    fclose(fptr);
    return 0;
}

#endif