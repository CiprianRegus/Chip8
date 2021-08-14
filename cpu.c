#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "stack.h"
#include "font.h"

#define START_ADDRESS 0x200
#define MEMORY_CAPACITY ((1<<12) - 0x200)
#define FONT_START_ADDRESS 0x50

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGTH 32
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
    unsigned char display_memory[256];
};


void load_fonts(struct chip8 *);

struct chip8 *new_chip8(){
    struct chip8 *ret = (struct chip8*)malloc(sizeof(struct chip8));
    ret->pc = 0x200;
    load_fonts(ret);
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

void load_fonts(struct chip8 *chip){
    /* Fonts are loaded into memory at address 0x50 */
    for(size_t i = 0; i < FONTSET_SIZE; i++){
        chip->memory[FONT_START_ADDRESS + i] = fontset[i];
    }
}

/* Clears the display memory */
void cls(struct chip8 *chip){
    memset(chip->display_memory, 0, 2048);
    chip->pc += 2;
}

/* Return from a subroutine */
void ret(struct chip8 *chip){
    /* Sets the PC to the address at the top of the stack,
    then substract 1 from SP */
    chip->pc = chip->stack[chip->sp];
    chip->sp--;
}

/* Jump to location nnn */
void jmp(struct chip8 *chip, unsigned short nnn){
    chip->pc = nnn;
}

/* Call subroutine at nnn */
void call(struct chip8 *chip, unsigned short nnn){
    push(chip->stack, chip->sp, chip->pc);
    chip->pc = nnn;
}

/* If Vx = kk, skip the next instruction */
void iskip_on_equal(struct chip8 *chip, unsigned short x, unsigned char kk){
    if(chip->registers[x] == kk){
        chip->pc += 2;
    }
    chip->pc += 2;
}

/* If Vx != kk, skip the next instruction */
void iskip_on_not_equal(struct chip8 *chip, unsigned short x, unsigned char kk){
    if(chip->registers[x] != kk){
        chip->pc += 2;
    }
    chip->pc += 2;
}

/* If Vx = Vy, skip the next instruction */
void skip_on_equal(struct chip8 *chip, unsigned short x, unsigned short y){
    if(chip->registers[x] == chip->registers[y]){
        chip->pc += 2;
    }
    chip->pc += 2;
}

/* Loads value kk into register Vx */
void iload(struct chip8 *chip, unsigned short x, unsigned char kk){
    chip->registers[x] = kk;
    chip->pc += 2;
}

/* Vx = Vx + kk */
void iadd(struct chip8 *chip, unsigned short x, unsigned char kk){
    chip->registers[x] += kk;
    chip->pc += 2;
}

/* Vx = Vy */
void assign(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] = chip->registers[y];
    chip->pc += 2;
}

/* Vx = Vx OR Vy */
void _or(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] |= chip->registers[y];
    chip->pc += 2;
}

/* Vx = Vx AND Vy */
void _and(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] &= chip->registers[y];
    chip->pc += 2;
}

/* Vx = Vx XOR Vy */
void _xor(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] ^= chip->registers[y];
    chip->pc += 2;
}

/* Vx = Vx + Vy, if the result is > 255, VF = 1 */
void add(struct chip8 *chip, unsigned short x, unsigned short y){
    int result = chip->registers[x] + chip->registers[y];
    if(result > 255){
        chip->registers[0xf] = 1;
    }else{
        chip->registers[0xf] = 0;
    }
    chip->registers[x] = (unsigned char)(result % 256);
    chip->pc += 2;
}

/* Vx = Vx - Vy, VF = NOT borrow */
void sub(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] -= chip->registers[y];
    if(chip->registers[x] < chip->registers[y]){
        chip->registers[0xf] = 0;
    }else{
        chip->registers[0xf] = 1;
    }
    chip->pc += 2;
}

/* Vx = Vx >> 1 */
void shr(struct chip8 *chip, unsigned short x){
    chip->registers[0xf] = chip->registers[x] & 1;
    chip->registers[x] >>= 1;
    chip->pc += 2;
}

/* Vx = Vy - Vx, VF = NOT borrow */
void subn(struct chip8 *chip, unsigned short x, unsigned short y){
    chip->registers[x] = chip->registers[y] - chip->registers[x];
    if(chip->registers[y] < chip->registers[x]){
        chip->registers[0xf] = 0;
    }else{
        chip->registers[0xf] = 1;
    }
    chip->pc += 2;
}

/* Vx = Vx << 1 */
void shl(struct chip8 *chip, unsigned short x){
    chip->registers[0xf] = chip->registers[x] & 0x80;
    chip->registers[x] <<= 1;
    chip->pc += 2;
}

/* If Vx != Vy, skip the next instruction */
void skip_on_not_equal(struct chip8 *chip, unsigned short x, unsigned short y){
    if(chip->registers[x] != chip->registers[y]){
        chip->pc += 2;
    }
    chip->pc += 2;
}

/* I = nnn */
void set_index(struct chip8 *chip, unsigned short nnn){
    chip->index_register = nnn;
    chip->pc += 2;
}

/* Jump to location V0 + nnn */
void jmp_rel(struct chip8 *chip, unsigned short nnn){
    chip->pc = chip->registers[0] + nnn;
}

/* Vx = random byte & kk */
void set_rand(struct chip8 *chip, unsigned short x, unsigned char kk){
    srandom(time(NULL));
    unsigned char r = random() % 256;
    chip->registers[x] = r & kk;
    chip->pc += 2;
}

/* 
    Display sprite (stored in the memory starting at address index_register) 
    on the screen, starting at coordinates (Vx, Vy). The size of the sprite is n (<= 15).
*/
void display_sprite(struct chip8 *chip, unsigned short x, unsigned short y, unsigned short n){
    unsigned char x_pos = chip->registers[x];
    unsigned char y_pos = chip->registers[y];

    if(n > 15){
        fprintf(stderr, "Sprite size %hu is invalid. Must be <= 15\n", n);
        exit(1);
    }
    /* Display is 64 x 32 */
    if(x_pos % 8 != 0 || y_pos % 8 != 0){
        fprintf(stderr, "Screen coordinates must be byte aligned (%hhu, %hhu)\n", x_pos, y_pos);
        exit(1);
    }
    if(x_pos >= DISPLAY_WIDTH || y_pos >= DISPLAY_HEIGTH){
        fprintf(stderr, "Coordinates out of screen (%hhu, %hhu)\n", x_pos, y_pos);
        exit(1);
    }
    chip->registers[0xf] = 0;
    for(unsigned short i = 0; i < n; i++){
        int display_index = (y_pos + i * DISPLAY_WIDTH + x_pos) % 2048;
        unsigned char sprite_byte = chip->memory[chip->index_register + i];

        /* VF is set to 1 if any pixel was erased */ 
        if((sprite_byte ^ chip->display_memory[display_index]) == 0 && 
                chip->display_memory[display_index] == 1){
            chip->registers[0xf] = 1;
        }
        chip->display_memory[display_index] ^= sprite_byte; 
    }
    chip->pc += 2;
}

/* Skip the next instruction if a key is pressed */
void skip_pressed(struct chip8 *chip, unsigned short x){
    unsigned char key = chip->registers[x];
    if(chip->keys[key] == 1){
        chip->pc += 2;
    }
    chip->pc += 2;
}

/* Skip the next instruction if a key is NOT pressed */
void skip_not_pressed(struct chip8 *chip, unsigned short x){
    unsigned char key = chip->registers[x];
    if(chip->keys[key] == 0){
        chip->pc += 2;
    }
    chip->pc += 2;
}

void load_delay(struct chip8 *chip, unsigned short x){
    chip->registers[x] = chip->delay_timer;
}

/* Pause the execution until a key is pressed */
void wait_for_key(struct chip8 *chip, unsigned short x){
    unsigned char keys_copy[16];
    memcpy(keys_copy, chip->keys, 16);
    while(1){
        for(int i = 0; i < 16; i++){
            if(chip->keys[i] != keys_copy[i]){
                chip->registers[x] = i;
                return;
            }
        }
        usleep(10000);
    }
    chip->pc += 2;
}

/* Delay timer = Vx */
void set_delay(struct chip8 *chip, unsigned short x){
    chip->delay_timer = chip->registers[x];
    chip->pc += 2;
}

/* Sound timer = Vx */
void set_sound(struct chip8 *chip, unsigned short x){
    chip->sound_timer = chip->registers[x];
    chip->pc += 2;
}

/* I = I + Vx */
void add_to_index(struct chip8 *chip, unsigned short x){
    chip->index_register += chip->registers[x];
    chip->pc += 2;
}

/* I is set to the location of the sprite for digit Vx ?*/
void load_location(struct chip8 *chip, unsigned short x){
    chip->pc += 2;
}

/* Store BCD representation of the value in Vx into mem[I, I+1, I+2] */
void store_bcd(struct chip8 *chip, unsigned short x){
    unsigned char xreg_value = chip->registers[x];
    unsigned char hundreds = xreg_value / 100;
    unsigned char tens = (xreg_value - hundreds) / 10;
    unsigned char units = xreg_value % 10; 
    chip->memory[chip->index_register] = hundreds;
    chip->memory[chip->index_register + 1] = tens;
    chip->memory[chip->index_register + 2] = units;

    chip->pc += 2;
}

/* Store registers V0 to Vx into memory starting at location I */
void store_registers(struct chip8 *chip, unsigned short x){
    for(int i = 0; i < x; i++){
        chip->memory[chip->index_register + i] = chip->registers[i];
    }
    chip->pc += 2;
}

/* Load values from memory (starting at address I) into registers V0 to Vx */
void load_registers(struct chip8 *chip, unsigned short x){
    for(int i = 0; i < x; i++){
        chip->registers[i] = chip->memory[chip->index_register + i];
    }
    chip->pc += 2;
}

