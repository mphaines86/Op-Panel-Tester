// LCD MODULES void cmd2LCD(char cmd)
#include <stdlib.h>
#include <stdint.h>
#include "utilities.h"

uint32_t freeRam ()
{
	extern int __heap_start, *__brkval;
	int v;
	return (uint32_t) &v - (__brkval == 0 ? (uint32_t) &__heap_start : (uint32_t) __brkval);
}

void Delay(void){
	float usdelay = 0.0;
	usdelay =(1000/ ( (1.0 / 28000000) * 1000000000 ) );

	while (usdelay != 0) {
		asm("NOP");
		usdelay--;
	}
}

void Delay_us(int delayus){
	while (delayus != 0){
		Delay();
		delayus--;
	}

}

void Delay_ms(int delayms){
	while (delayms != 0){
		Delay_us(1000);
		delayms--;
	}
}

void initArray(Array *a, size_t initialSize) {
	a->array = (char *) malloc(initialSize * sizeof(char*));
	a->used = 0;
	a->size = initialSize;
}

void insertArray(Array *a, char element) {
	// a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
	// Therefore a->used can go up to a->size
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (char *) realloc(a->array, a->size * sizeof(char));
	}
	a->array[a->used++] = element;
}

void freeArray(Array *a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}