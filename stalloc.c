#include "stalloc.h"

typedef uint8_t bitmap;

struct heap {
	uint64_t size;
	m_block* first_block;
} __attribute__((packed));

struct m_block {
	m_block* next;
	uint8_t chunk_size;
	uint8_t chunk_amt;
} __attribute__((packed));

static void     init_block(m_block* block, uint8_t chunk_size, uint8_t chunk_amt);
static m_block* calc_next_block_addr(m_block* prev); 
static void*    alloc_space_in_block(m_block* block, uint32_t size);
static uint8_t  get_free_chunks(m_block* block);
static uint8_t  get_next_free_chunk(m_block* block);
static void*    get_chunk_addr(m_block* block, uint8_t chunk);
static uint8_t  get_chunk_from_addr(m_block* block, void* addr);
static void     set_chunk_occupied(m_block* block, uint8_t chunk);
static bitmap*  get_bitmap(m_block* block);
static void*    get_mem_region(m_block* block);
static void     set_chunk_available(m_block* block, uint8_t chunk);
static m_block* first_fit(heap* h, uint32_t size);
static m_block* best_fit(heap* h, uint32_t size);
static m_block* worst_first(heap* h, uint32_t size);

static bitmap* get_bitmap(m_block* block)
{
	return (char*) ((char*) block) + sizeof(m_block);
}

void init_heap(heap* h, uint32_t size)
{
	for (uint32_t byte = 0; byte < size; byte++)
		((uint8_t*)h)[byte] = 0;


	// Create an initial "null" block
	m_block* first = (m_block*) ((char*) h + sizeof(heap));
	init_block(first, 0, 0);

	h->size = size;
	h->first_block = first;
}

void* stalloc(heap* h, uint32_t size)
{
	m_block* block = first_fit(h, size);
	return alloc_space_in_block(block, size);
}

static void* get_mem_region(m_block* block)
{
	return (void*) (((char*) get_bitmap(block)) + block->chunk_amt / 8);
}

static uint8_t get_chunk_from_addr(m_block* block, void* addr)
{
	uint64_t mem_reg = get_mem_region(block);
	uint64_t offset = addr - mem_reg;	
	return (offset / block->chunk_size);
}

static void set_chunk_occupied(m_block* block, uint8_t chunk)
{
	bitmap* table = get_bitmap(block);
	table[chunk / 8]  |= 1 << (chunk % 8);
}

static void set_chunk_available(m_block* block, uint8_t chunk)
{
	bitmap* map = get_bitmap(block);
	map[chunk / 8] &= ~(1 << chunk);	
}

static void free(heap* h, void* addr)
{
	m_block* block = h->first_block;
	while (block != NULL) {
		uint32_t size = block->chunk_amt * block->chunk_size;
		uint64_t mem_base = (uint64_t) get_mem_region(block);	
		uint64_t limit = mem_base + size;

		if ((uint64_t) addr >= mem_base && (uint64_t) addr <= limit) {
			set_chunk_available(block, get_chunk_from_addr(block, addr));
			return;
		}

		block = block->next;
	}
}

static void* alloc_space_in_block(m_block* block, uint32_t size)
{
	uint8_t free_chunk = get_next_free_chunk(block);		
	set_chunk_occupied(block, free_chunk);
	return get_chunk_addr(block, free_chunk);
}

static void* get_chunk_addr(m_block* block, uint8_t chunk)
{
	return (void*) (((char*) block) + sizeof(m_block) + block->chunk_amt / 8 + block->chunk_size * chunk);
}

static uint8_t get_free_chunks(m_block* block)
{
	uint8_t free = 0;
	char* bm = get_bitmap(block);

	for (uint8_t table = 0; table < block->chunk_amt / 8; table++)
		for (uint8_t bit = 0; bit < 8; bit++)
			if (((bm[table] >> bit) & 1) == 0)
				free++;

	return free;	
}

static uint8_t get_next_free_chunk(m_block* block)
{
	uint8_t chunk_number = 0;
	char* bm = get_bitmap(block);

	for (uint8_t table = 0; table < block->chunk_amt / 8; table++) 
		for (uint8_t bit = 0; bit < 8; bit++)
			if (((bm[table] >> bit) & 1) == 0)
				return chunk_number;
			else
				chunk_number++;
	return 255;
}

static m_block* first_fit(heap* h, uint32_t size)
{
	m_block* block = h->first_block;
	while (block != NULL) {
		if (block->chunk_size >= size
		    && get_free_chunks(block) > 0)
			return block;

		block = block->next;
	}
	return 0;
}

void create_block(heap* h, uint8_t chunk_size, uint8_t chunk_amt)
{
	// Find last block
	m_block* block = h->first_block;
	while (block->next != NULL)
		block = block->next;

	m_block* new_block = calc_next_block_addr(block);
	init_block(new_block, chunk_size, chunk_amt);
	block->next = new_block;
}

static m_block* calc_next_block_addr(m_block* prev)
{
	// prev addr + header + table + heap space = next block addr
	uint8_t c_size = prev->chunk_size;
	uint8_t c_amt  = prev->chunk_amt;
	return (m_block*) (((char*) prev) + sizeof(m_block) + (c_amt / 8) + (c_amt * c_size));
}

static void init_block(m_block* block, uint8_t chunk_size, uint8_t chunk_amt)
{
	block->chunk_size = chunk_size;
	block->chunk_amt = chunk_amt;
	block->next = NULL;
}
