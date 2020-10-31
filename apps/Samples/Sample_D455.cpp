#include <stdio.h>
#include <stdlib.h>

#include "D400s/d400s.h"

int main(void)
{
	printf("Sample D455\n");
	D400s d455;
	int ret = d455.init();
	if(ret == -1) return -1;

	return 0;
}
