#ifndef HELPER
#define HELPER

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>

#define MAX_INPUT_BUFFER 1000

/*Struct type for the superblock*/
typedef struct __attribute__((__packed__)) s_block {
    uint8_t fs_id[8]; 
    uint16_t block_size;
    uint32_t block_count;
    uint32_t FAT_starts;
    uint32_t FAT_blocks;
    uint32_t r_dir_start;
    uint32_t r_dir_blocks;
} s_block;

/*Stuct type for the directory entry time*/
typedef struct __attribute__((__packed__)) dir_entry_timedate {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

} dir_entry_timedate;

/*Struct type for the directy entry*/
typedef struct __attribute__((__packed__)) dir_entry {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    dir_entry_timedate create_time;
    dir_entry_timedate modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
} dir_entry;

/**
 * @brief Reads the FAT table contents of a FAT filesystem
 * @param fd the filenumber of the file to read
 * @param s a pointer to a super block struct of the file
 * @param free_blocks a pointer to an int to store the results of the number of free blocks of read_FAT_information
 * @param reserved_blocks a pointer to an int to store the results of the number of reserved blocks of read_FAT_information
 * @param allocated_blocks a pointer to an int to store the results of the number of allocated blocks read_FAT_information
 */
void read_FAT_information(int fd, s_block* s, int* free_blocks, int* reserved_blocks, int* allocated_blocks) {
    struct stat f_buffer;
    int status = fstat(fd, &f_buffer);
    if (fd == -1 || status == -1) {
        printf("ERROR - %s\n", (fd == -1)? "Fileno failed" : "Fstat failed");
        return;
    }

    char* address = mmap(NULL, f_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int end = s->block_size+((s->FAT_blocks)*s->block_size);

    int free_blocks_l = 0;
    int reserved_blocks_l = 0;
    int allocated_blocks_l = 0;
    for (int i = s->block_size; i < end; i+=4) {
        int cur_block = 0;
        memcpy(&cur_block, address+i, 4);
        cur_block = ntohl(cur_block);

        if (cur_block == 0x00000000) (free_blocks_l)++;
        else if (cur_block == 0x00000001) (reserved_blocks_l)++;
        else (allocated_blocks_l)++;
    }
    *free_blocks = free_blocks_l;
    *reserved_blocks = reserved_blocks_l;
    *allocated_blocks = allocated_blocks_l;
    if (!*free_blocks && !*reserved_blocks && !*allocated_blocks) printf("Image full\n");
    munmap(address, f_buffer.st_size);
}

/**
 * @brief returns the superblock information of a file
 * @param fd the filenumber of the file
 * @return a pointer to a super block struct that contains the contents of the files superblock
 * @pre assuming the file represets a FAT system
 */
s_block* initialize_super_block(int fd) {
    struct stat f_buffer;
    int status = fstat(fd, &f_buffer);

    if (fd == -1) {
        printf("ERROR - Fileno() failed\n");
        return NULL;
    }

    if (status == -1) {
        printf("ERROR - fstat failed\n");
        return NULL;
    }

    void* address = mmap(NULL, f_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (address == MAP_FAILED) {
        printf("ERROR - Failed to map file in memory\n");
        return NULL;
    }
    
    s_block* super_block = (s_block*)malloc(sizeof(s_block));
    memcpy(super_block, address, 30);
    super_block->block_size = ntohs(super_block->block_size);
    super_block->block_count = ntohl(super_block->block_count);
    super_block->FAT_starts = ntohl(super_block->FAT_starts);
    super_block->FAT_blocks = ntohl(super_block->FAT_blocks);
    super_block->r_dir_start = ntohl(super_block->r_dir_start);
    super_block->r_dir_blocks = ntohl(super_block->r_dir_blocks);
    munmap(address, f_buffer.st_size);
    return super_block;
}

/**
 * @brief counts the amount of directories a filepath would need to traverse
 * @param input a string that is a filepath
 * @return an int for the number of subdirectories
 */
int count_subdirectories(char* input) {
    int count = 0;
    char copy_input[MAX_INPUT_BUFFER];
    strcpy(copy_input, input);
    if (copy_input == NULL || copy_input[0] != '/') return -1;
    char* buffer = strtok(copy_input, "/");
    for(;buffer != NULL; buffer=strtok(NULL, "/")) count++;
    return count;
}

/**
 * @brief generates the result of the fat table at a specific block number
 * @param count_num the block number to be looked up
 * @param address the location of the file mapped in memoru
 * @param s a struct of a superblock of a file
 * @return 
 */
int generate_fat_table(int count_num, void* address, s_block* s) {
    count_num *= 4; 
    int fat_table_entry;
    memcpy(&fat_table_entry, address+s->block_size+count_num, 4);

    if (fat_table_entry == 0xFFFFFFFF) return -1;

    return ntohl(fat_table_entry);
}


/**
 * @brief
 * @param
 * @param
 * @param
 * @param
 * 
 */
int find_subdirectory(char* target_file, int input_block_num, void* address, s_block* s, dir_entry* entry, char type) {
    int cur_block = (input_block_num)*(s->block_size);
    int cur_block_num = cur_block/(s->block_size);
    int block_to_find;
    while (1) {
        for (int i = (cur_block_num*s->block_size); i < (cur_block_num*s->block_size)+s->block_size; i+=64) {
            dir_entry* dir_e = (dir_entry*)malloc(sizeof(dir_entry));
            dir_entry* empty = (dir_entry*)malloc(sizeof(dir_entry));


            memcpy(dir_e, address+i, 64);

            //May change to dir_e->status != 0
            if (!memcmp(dir_e, empty, 64)) {
                free(empty);
                free(dir_e);
                continue;
            }
            free(empty);
            if (memcmp((char *)dir_e->filename, target_file, (strlen((char*)(dir_e->filename)) > strlen(target_file))? strlen((char*)(dir_e->filename)) : strlen(target_file)) == 0 && dir_e->status & ((type == 'D')? 0b0000100 : 0b00000010)) {
                block_to_find = ntohl(dir_e->starting_block);
                if (entry != NULL) memcpy(entry, dir_e, 64);
                free(dir_e);
                return block_to_find;

            }
            if (entry != NULL) memcpy(entry, dir_e, 64); 
            free(dir_e);
        }
        int next_block = generate_fat_table(cur_block_num, address, s);
        if (next_block == -1) break;
        cur_block_num = next_block;
        cur_block = cur_block*s->block_size;
    }
    return -1;
}

void modify_subdirectory_time(char* target_file, int start_block_num, void* address, s_block* s) {
    int cur_block_num = start_block_num;
    while (1) {
        for (int i = (cur_block_num*s->block_size); i < (cur_block_num*s->block_size)+s->block_size; i+=64) {
            dir_entry* dir_e = (dir_entry*)malloc(sizeof(dir_entry));
            memcpy(dir_e, address+i, 64);

            if (memcmp((char *)dir_e->filename, target_file, strlen((char*)(dir_e->filename))) == 0 && dir_e->status & 0b00000100) {
                time_t now;
                struct tm *now_tm;
                now = time(NULL);
                now_tm = localtime(&now);
                int create_year = htons(now_tm->tm_year + 1900);
                int create_month = now_tm->tm_mon + 1;
                dir_entry_timedate input_time = {create_year, create_month, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec};
                memcpy(address+i+20, &input_time, 7);
                return;
            }
            free(dir_e);
        }
        int next_block = generate_fat_table(cur_block_num, address, s);
        if (next_block == -1) break;
        cur_block_num = next_block;
    }
}

int find_next_available_block(void* address, s_block* s) {
    int cur_block_num = -1;
    for (int i = s->block_size; i < s->block_size + (s->FAT_blocks * s->block_size); i += 4) {
        int cur_block = 0;
        memcpy(&cur_block, address+i, 4);
        //printf("i = %d\n", i);
        if (cur_block == 0x00000000) {
            cur_block_num = (i - s->block_size)/4;
            break;
        }
    }

    return cur_block_num;
}

void write_directory_entry(void* address, int dir_entry_placement, char type, int sb, int bc, int si, int y, int m, int d, int h, int min, int sec, char* name, s_block* s) {
    dir_entry* new_dir = (dir_entry*)malloc(sizeof(dir_entry));
    
    new_dir->status = (type == 'F')? 3 : 5;
    new_dir->starting_block = htonl(sb);
    new_dir->block_count = htonl(bc);
    new_dir->size = htonl(si);
    dir_entry_timedate time = {htons(y), m, d, h, min, sec};
    new_dir->create_time = time;
    new_dir->modify_time = time;
    
    strncpy((char*)new_dir->filename, name, 31);

    int cur_block_num = dir_entry_placement;
    int addr_dir = 0;
    while (1) {
        for (addr_dir = (cur_block_num*s->block_size); addr_dir < (cur_block_num*s->block_size)+s->block_size; addr_dir+=64) {
            dir_entry* dir_e = (dir_entry*)malloc(sizeof(dir_entry));
            memcpy(dir_e, address+addr_dir, 64);
            if (dir_e->status == 0) break;
            free(dir_e);
        }
        int new_block_num = generate_fat_table(cur_block_num, address, s);
        cur_block_num = new_block_num;
        if (cur_block_num == -1) break;
    }

    if (cur_block_num == 1) {
        printf("DIRECTORY FULL");
    }
    memcpy(address+addr_dir, new_dir, 64);


}

void write_to_FAT(void* address, s_block* s, int block_num, int change_to) {
    int to_copy = htonl(change_to);
    memcpy(address+s->block_size+(block_num*4), &to_copy, 4);
}

int traverse_to_holding_directory(int total_paths, void* address, char* in, char* out, s_block* s, dir_entry* entry, int flags) {
    int cur_block_num = s->r_dir_start;
    
    char* prev = out;
    out = strtok(in, "/");
    dir_entry* holding_dir_entry = (dir_entry*)malloc(sizeof(dir_entry));
    for (; total_paths > 0; total_paths--, out = strtok(NULL, "/")) {
        prev = out;
        if (out == NULL) {
            printf("ERROR - Could not parse string\n");
            out = prev;
            return -1;
        }
        int prev_block_num = cur_block_num;
        cur_block_num = find_subdirectory(out, prev_block_num, address, s, holding_dir_entry, 'D');
        if (cur_block_num != -1 && flags & 0b00000010) modify_subdirectory_time(out, prev_block_num, address, s);
        /*If the directory doesn't exist and this flags is set to 1*/
        /*Add a new directory entry*/
        if (flags & 0b00000001 && cur_block_num == -1) {
            /*See if there is an available dir_entry_spot*/
            while (1) {
                int no_new_block = 0;
                for (int i = prev_block_num*s->block_size; i < prev_block_num*s->block_size + s->block_size; i+=64) {                  
                    dir_entry* dir_e = (dir_entry*)malloc(sizeof(dir_entry));
                    memcpy(dir_e, address+i, 64);
                    if (dir_e->status == 0) {
                        int new_dir_start = find_next_available_block(address, s);
                        write_to_FAT(address, s, new_dir_start, 0xFFFFFFFF);
                        time_t now;
                        struct tm *now_tm;
                        now = time(NULL);
                        now_tm = localtime(&now);
                        write_directory_entry(address, prev_block_num,'D', new_dir_start, 1, 0, now_tm->tm_year+1900, now_tm->tm_mon+1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec, out, s);
                        free(dir_e);
                        cur_block_num = new_dir_start;
                        no_new_block = 1;
                        break;
                    }
                    free(dir_e);
                }
                if (no_new_block) {
                    break;
                }
            }
        }
    }
    return cur_block_num;
}

void write_to_disk(char* filename, void* address_disk, void* address_file, struct stat file_info, int dir_entry_block_num, s_block* s) {
    int total_blocks = (int)ceil((double)file_info.st_size / s->block_size);
    int start_block = -1;
    int cur_bytes_copied = 0;
    for (int i = 0; i < total_blocks; i++) {
        int open_block = find_next_available_block(address_disk, s);
        if (start_block == -1) start_block = open_block;
        int num_bytes_to_copy = (i + 1 == total_blocks)? file_info.st_size%s->block_size : s->block_size;
        memcpy(address_disk+open_block*s->block_size, address_file+cur_bytes_copied, num_bytes_to_copy);
        cur_bytes_copied += num_bytes_to_copy;
        write_to_FAT(address_disk, s, open_block, 0xFFFFFFFF);
        if (i != total_blocks - 1) {
            int next_block = find_next_available_block(address_disk, s);
            write_to_FAT(address_disk, s, open_block, next_block);
        }
    }

    struct tm *time = localtime(&file_info.st_mtime);
    char newfilename[MAX_INPUT_BUFFER];
    strncpy(newfilename, filename, strlen(filename));
    
    write_directory_entry(
        address_disk, 
        dir_entry_block_num, 
        'F', 
        start_block, 
        total_blocks, 
        file_info.st_size, 
        time->tm_year+1900,
        time->tm_mon+1,
        time->tm_mday,
        time->tm_hour,
        time->tm_min,
        time->tm_sec,
        filename,
        s);
        
}

char* get_filename(char* input) {
    char* buf;
    char* token = strtok(input, "/");
    while (token != NULL) {
        buf = token;  
        token = strtok(NULL, "/");
    }
    return buf;
}
#endif