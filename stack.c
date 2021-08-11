#define STACK_MAX_SIZE 16

void push(unsigned short *stack, unsigned short sp, unsigned short element){
    if(sp >= STACK_MAX_SIZE){
        return;
    }
    *(stack + sp) = element;
    sp++;
}

void pop(unsigned short *stack, unsigned short sp){
    if(sp <= 0){
        return;
    }
    sp--;
}