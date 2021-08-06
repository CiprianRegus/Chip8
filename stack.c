#define STACK_MAX_SIZE 16

void push(unsigned short *stack, short sp, short element){
    if(sp >= STACK_MAX_SIZE){
        return;
    }
    *(stack + sp) = element;
    sp++;
}