#include "stalloc.h"

char space[4096];

int main(void)
{
	// Initialize the memory
	heap* h = (heap*) space;
	init_heap(h, 4096);

	// Create a block with 16 chunks, each being 8 bytes wide
	create_block(h, 8, 16);

	// Allocate memory
	int* number = (int*) stalloc(h, sizeof(int));
	*number = 42;

	printf("number = %p\n*number = %d\n", number, *number);

	free(h, number);

	return 0;
}
