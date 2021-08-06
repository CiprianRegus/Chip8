#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "stack.h"
#include "font.h"

#define START_ADDRESS 0x200
#define MEMORY_CAPACITY ((1<<12) - 0x200)
#define FONT_START_ADDRESS 0x50

struct chip8{
    unsigned char registers[16];
    unsigned char memory[4096];
    unsigned short index_register;
    unsigned short pc;
    unsigned short stack[16];
    unsigned char sp;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char keys[16];
    unsigned char display_memory[2048];
};

struct chip8 *new_chip8(){
    struct chip8 *ret = (struct chip8*)malloc(sizeof(struct chip8));
    ret->pc = 0x200;

    return ret;
}

void load_rom(struct chip8 *chip, const char *path){
    FILE *fd = fopen(path, "r");
    if(fd == NULL){
        perror("Error opening the file");
        errno = EINVAL;
        return;
    }
    fseek(fd, 0, SEEK_END);

    size_t file_size = ftell(fd);
    if(file_size > MEMORY_CAPACITY){
        perror("File to large to fit into memory");
        errno = ENOMEM;
        return;
    }

    void *buffer = malloc(file_size);
    /* Reset the cursor to the beginning of the file */
    fseek(fd, 0, SEEK_SET);
    fread(buffer, file_size, 1, fd);

    /* Transfer the ROM data into the chip's memory */
    for(size_t i = 0; i < file_size; i++){
        /* There is reserved memory space from 0x0 to 0x1ff */
        chip->memory[START_ADDRESS + i] = *((unsigned char *)buffer + i);
    }

    free(buffer);
}


int main(){
    struct chip8 *chip = new_chip8();
    load_rom(chip, "chip8-test-rom/test_opcode.ch8");
    printf("%hhu", fontset[2]);

    free(chip);
}