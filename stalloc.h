#include <stdint.h>

struct heap;
struct m_block;

typedef struct heap heap;
typedef struct m_block m_block;

void  init_heap(heap* h, uint32_t size);
void  create_block(heap* h, uint8_t chunk_size, uint8_t chunk_amt);
void* stalloc(heap* h, uint32_t size);
void  free(heap* h, void* addr);
void  print_blocks(heap* h);
