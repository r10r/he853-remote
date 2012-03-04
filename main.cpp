#include <stdlib.h>
#include <stdio.h>
#include "he853.h"

HE853Controller *remote = 0;

int main(int argc, char **argv)
{
	printf("HomeEasy Stresstest.\nI will flood GER protocol on port 2000 with on and off.\n");

	remote = new HE853Controller();

	for (;;)
	{
		printf("Cycle...");
		remote->sendEU((uint16_t)2000, (uint8_t)1);
		remote->sendEU((uint16_t)2000, (uint8_t)0);
		printf("done.\n");
	}
	return 0;
}
