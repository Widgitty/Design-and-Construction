#ifndef __LCD_BUFFER_H__
#define __LCD_BUFFER_H__
#include <stdint.h>

typedef struct {
	uint32_t* buffer;
	int size;
	int in;
	int out;
} BUFFER;

/* Initialise a buffer structure by allocating the given number of bytes for its contents */
void buffer_init(BUFFER *buffer, int size);

/* Free the memory allocated by initBuffer once the buffer is no longer required */
void buffer_destroy(BUFFER *buffer);

/* Returns true (non-zero) if there is no more space in the buffer */
int buffer_full(BUFFER *buffer);

/* Returns true (non-zero) if there is no data to be read from the buffer */
int buffer_empty(BUFFER *buffer);

/* Puts the given data into the buffer, if space is available */
void buffer_put(BUFFER *buffer, uint32_t data);

/* Gets the next available item from the buffer */
uint32_t buffer_get(BUFFER *buffer);

#endif /*__LCD_BUFFER_H__*/
