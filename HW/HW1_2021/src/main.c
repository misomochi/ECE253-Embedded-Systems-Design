#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cache_model.h"

// gcc -DENABLE = 0/1 main.c cache_model.c
#ifndef PROBLEM
#define PROBLEM 'A'
#endif

#ifndef ENABLE
#define ENABLE 1
#endif

#ifndef TEST_TIMES
#define TEST_TIMES 10
#endif

#ifndef READ_TIMES
#define READ_TIMES 100000
#endif

int next_address(int);

int main(int argc, char const *argv[])
{
	time_t t;
	srand((unsigned) time(&t));

	cm_init();

	#if ENABLE == 1
		cm_enable_cache();
	#else
		cm_disable_cache();
	#endif

	for (int i = 0; i < TEST_TIMES; ++i)
	{
		unsigned hit = 0, miss = 0;
		int address = rand() % CM_ADDRESS_SPACE_SIZE;

		for (int j = 0; j < READ_TIMES; ++j)
		{
			cm_do_access(address); // access memory at the location "address"
			(cm_get_last_access_cycles() == 1) ? ++hit : ++miss; // cm_get_last_access_cycles() == 1 implies a hit. If not (11 or 17), then it's a miss.

			#if PROBLEM == 'A'
				address = rand() % CM_ADDRESS_SPACE_SIZE; // generate next random address
			#else
				address = next_address(address);
			#endif
		}

		printf("%d-th time\n", i + 1);
		printf("Hit: %u\n", hit);
		printf("Miss: %u\n", miss);
		printf("Expected access cycle: %f\n", (double)(hit + miss * ((ENABLE == 1) ? 17 : 11))/READ_TIMES); // weighted average for access cycle. Miss penalties for enabled and disabled cache are 17 and 11 cycles respectively. 
		printf("-----------\n");
	}

	return 0;
}

int next_address(int address) {
	int r = rand(), a;
	short add = r % 2;
	double p = (double)r / RAND_MAX;

	if (p < 0.6) {
		a = address + 1;
	}
	else if (p < 0.95) {
		a = (add) ? ((r % 39) + address + 2) : ((r % 40) + address - 40);
	}
	else {
		if (address <= 40)
			a = r % (CM_ADDRESS_SPACE_SIZE - address - 41) + address + 41;
		else if (address >= CM_ADDRESS_SPACE_SIZE - 41)
			a = r % 40;
		else
			a = (add) ? ((r % (CM_ADDRESS_SPACE_SIZE - address - 41)) + address + 41) : (r % (address - 40));
	}

	if (a < 0)
		return 0;
	else if (a >= CM_ADDRESS_SPACE_SIZE)
		return CM_ADDRESS_SPACE_SIZE - 1;
	else
		return a;
}