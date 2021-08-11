#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "stack.h"
#include "cpu.h"

void decode(struct chip8 *chip){
    unsigned short instruction = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];
    unsigned char opcode = (instruction & 0xf000) >> 12;
    
    while(1){
        switch(opcode){
            case 0x0: {
                switch(instruction & 0x0fff){
                    case 0x00e0: {
                        
                    }
                }
            }
            default: {
                perror("Invalid instruction\n"); 
                return;
            }
        }
    }
}

int main(){
    struct chip8 *chip = new_chip8();
    load_rom(chip, "chip8-test-rom/test_opcode.ch8");
    if(errno != EINVAL && errno != ENOMEM){
        printf("Successfully loaded ROM in memory\n");
    }
    decode(chip);
    free(chip);
}