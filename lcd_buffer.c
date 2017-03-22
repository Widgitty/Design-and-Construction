#include "lcd_buffer.h"
#include <stdlib.h>

#define __LCD_DRIVER_DONT_USE_MALLOC__

/* Hack to make driver work without malloc */
#ifdef __LCD_DRIVER_DONT_USE_MALLOC__
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 128
#endif /* BUFFER_SIZE */
uint32_t unmalloc_buffer[BUFFER_SIZE];
#endif /* __LCD_DRIVER_DONT_USE_MALLOC__ */

void buffer_init(BUFFER *buffer, int size) {


#ifdef __LCD_DRIVER_DONT_USE_MALLOC__
	/* Dirty hack if malloc is unavailable */
	buffer->buffer = unmalloc_buffer;
	buffer->size = BUFFER_SIZE;
	
#else
	/* Allocate memory for the buffer */
	buffer->buffer = malloc(size * sizeof(uint32_t));
	if (buffer->buffer == 0)
	{
		/* If the debugger gets here, malloc probably isn't 
		 * available. Please add
		 * #define __LCD_DRIVER_DONT_USE_MALLOC__
		 * to the top of this file (or define it in the 
		 * compiler settings) to make it work. 
		 */
		while(1);
	}
	/* Record the size of the buffer in the structure */
	buffer->size = size;
#endif
		
	/* Reset in and out pointers to zero */
	buffer->in = buffer->out = 0;
}

void buffer_destroy(BUFFER *buffer) {
	/* Release the memory allocated in initBuffer() */
	free(buffer->buffer);
}

int buffer_full(BUFFER *buffer) {
	/* Buffer is full when out == in+1 */
	return ((buffer->in + 1) % buffer->size) == buffer->out;
}

int buffer_empty(BUFFER *buffer) {
	/* Buffer is empty when in == out */
	return buffer->in == buffer->out;
}

void buffer_put(BUFFER *buffer, uint32_t data) {
	/* If the buffer is full, data loss will occur */
	if (buffer_full(buffer)) return;
	/* Put the data in the buffer */
	*(buffer->buffer + buffer->in) = data;
	/* Increment the in pointer, wrapping round if necessary */
	buffer->in = (buffer->in + 1) % buffer->size;
}

uint32_t buffer_get(BUFFER *buffer) {
	uint32_t *ptr;
	/* If the buffer's empty, there's nothing to return */
	if (buffer_empty(buffer)) return 0;
	/* Get a pointer to the item that's about to be returned */
	ptr = buffer->buffer + buffer->out;
	/* Increment the out pointer, wrapping round if necessary */
	buffer->out = (buffer->out + 1) % buffer->size;
	/* Return the data */
	return *ptr;
}
