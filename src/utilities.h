#ifndef UTILITIES_H
#define UTILITIES_H


typedef struct {
    char * array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, char element);
void freeArray(Array *a);

uint32_t freeRam();
void Delay_ms(int delayms);
void Delay_us(int delayus);
void Delay(void);



#endif
