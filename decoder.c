#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "stack.h"
#include "cpu.h"

void decode(struct chip8 *chip){

    while(1){
        if(chip->pc >= 0x1000){
            perror("Invalid memory address\n");
            return;
        }
        unsigned short instruction = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];
        unsigned char opcode = (instruction & 0xf000) >> 12;
        
        switch(opcode){
            case 0x0: {
                switch(instruction & 0x0fff){
                    case 0x00e0: {
                        cls(chip);
                    }
                    case 0x00ee: {
                        ret(chip);
                    }
                }
            }
            case 0x1: {
                unsigned short jump_addr = instruction & 0x0fff;
                jmp(chip, jump_addr);
                break;
            }
            case 0x2: {
                unsigned short jump_addr = instruction & 0x0fff;
                call(chip, jump_addr);
                break;
            }
            case 0x3: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned char kk = instruction & 0x00ff;
                iskip_on_equal(chip, x, kk);
                break;
            }
            case 0x4: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned char kk = instruction & 0x00ff;
                iskip_on_not_equal(chip, x, kk);
                break;
            }
            case 0x5: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned short y = (instruction & 0x00f0) >> 4;
                skip_on_equal(chip, x, y);
                break;
            }
            case 0x6: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned char kk = instruction & 0x00ff;
                iload(chip, x, kk);
                break;
            }
            case 0x7: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned char kk = instruction & 0x00ff;
                iadd(chip, x, kk);
                break;
            }
            case 0x8: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned short y = (instruction & 0x00f0) >> 4;
                switch(instruction & 0xf){
                    case 0: {
                        assign(chip, x, y);
                        break;
                    }
                    case 1: {
                        _or(chip, x, y);
                        break;
                    }
                    case 2: {
                        _and(chip, x, y);
                        break;
                    }
                    case 3: {
                        _xor(chip, x, y);
                        break;
                    }
                    case 4: {
                        add(chip, x, y);
                        break;
                    }
                    case 5: {
                        sub(chip, x, y);
                        break;
                    }
                    case 6: {
                        shr(chip, x);
                        break;
                    }
                    case 7: { 
                        subn(chip, x, y);
                        break;
                    }
                    case 0xe: {
                        shl(chip, x);
                        break;
                    }
                    default: {
                        perror("Invalid instruction\n"); 
                        return;
                    }
                }
                
                break;
            }

            case 0x9: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned short y = (instruction & 0x00f0) >> 4;
                skip_on_not_equal(chip, x, y);
                break;
            }
            case 0xA: {
                unsigned short nnn = instruction & 0x0fff;;
                set_index(chip, nnn);
                break;
            }
            case 0xB: {
                unsigned short nnn = instruction & 0x0fff;;
                jmp_rel(chip, nnn);
                break;
            }
            case 0xC: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned char kk = instruction & 0x00ff;
                set_rand(chip, x, kk);
                break;
            }
            case 0xD: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                unsigned short y = (instruction & 0x00f0) >> 4;
                unsigned short n = instruction & 0x000f;
                display_sprite(chip, x, y, n);
                break;
            }
            case 0xE: {
                switch(instruction & 0xff){
                    case 0x9e: {
                        unsigned short x = (instruction & 0x0f00) >> 8;
                        skip_pressed(chip, x);
                        break;
                    }
                    case 0xa1: {
                        unsigned short x = (instruction & 0x0f00) >> 8;
                        skip_not_pressed(chip, x);
                        break;
                    }
                    default: {
                        perror("Invalid instruction\n"); 
                        return;
                    }
                }
                break;
            }
            case 0xF: {
                unsigned short x = (instruction & 0x0f00) >> 8;
                switch(instruction & 0xff){
                    case 0x07: {
                        load_delay(chip, x);
                        break;
                    }
                    case 0x0a: {
                        wait_for_key(chip, x);
                        break;
                    }
                    case 0x15: {
                        set_delay(chip, x);
                        break;
                    }
                    case 0x18: {
                        set_sound(chip, x);
                        break;
                    }
                    case 0x1e: {
                        add_to_index(chip, x);
                        break;
                    }
                    case 0x29: {
                        load_location(chip, x);
                        break;
                    }
                    case 0x33: {
                        store_bcd(chip, x);
                        break;
                    }
                    case 0x55: {
                        store_registers(chip, x);
                        break;
                    }
                    case 0x65: {
                        load_registers(chip, x);
                    }
                    default: {
                        perror("Invalid instruction\n"); 
                        return;
                    }
                }
                break;
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