#include <stdio.h>
#include <stdlib.h>
#include "cache_model.h"

#define CACHE_SIZE (CM_CACHE_LINES*CM_LINE_SIZE)

static int cache_tags[CM_CACHE_LINES];
static int cache_enabled;
static int dram_buffer_tag;
static void do_uncached_access(int address);
static void do_cached_access(int address);
static int access_cycles;

void cm_init()
{
	cm_reset_model();
}

void cm_reset_model()
{
	int i;
	for(i=0; i<CM_CACHE_LINES; i++)
		cache_tags[i] = -1;
	dram_buffer_tag = -1;
}


void cm_do_access(int address)
{
	if((address <0)|| (address >= CM_ADDRESS_SPACE_SIZE))
	{
		fprintf(stderr, "cm_do_access(%d): Address out of range!\n", address);
		exit(1);
	}
	if(cache_enabled)
		do_cached_access(address);
	else
		do_uncached_access(address);

}

void cm_enable_cache()
{
	cm_reset_model();
	cache_enabled = 1;
}

void cm_disable_cache()
{
	cm_reset_model();
	cache_enabled = 0;
}

static void do_cached_access(int address)
{
	int line = (address/CM_LINE_SIZE) % CM_CACHE_LINES;
	int tag = address / CM_LINE_SIZE;

	if(cache_tags[line] == tag)
	{	//Hit
		access_cycles = 1;
	}
	else
	{	//Miss
		access_cycles = CM_DRAM_LATENCY + CM_CACHE_UPDATE_CYCLES
			+ CM_DRAM_CYCLES_PER_WORD * CM_LINE_SIZE;
		cache_tags[line] = tag;
	}
}

static void do_uncached_access(int address)
{
	int tag = address/CM_DRAM_BUFFER_SIZE;
	if (tag == dram_buffer_tag)
	{	//Hit
		access_cycles =1;
	}
	else
	{
		access_cycles = CM_DRAM_LATENCY
			+ CM_DRAM_CYCLES_PER_WORD * CM_DRAM_BUFFER_SIZE;
		dram_buffer_tag = tag;
	}
}

int cm_get_last_access_cycles()
{
	return access_cycles;
}