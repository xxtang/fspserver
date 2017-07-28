#include "tweak.h"
#include <stdio.h>
#include <stdlib.h>
#include "my-string.h"
#include <math.h>

static int bitcount[16];
static int rounds;
static int result;
#define MAX_WORST_ALLOWED 0.1f

/* FSP classic algo */
static unsigned short classic (void)
{
  unsigned long k;

  k = random();
  k = k ^ (k >> 8) ^ (k >> 16) ^ (k << 8);

  return k;
}

/* get low bits from random result */
static unsigned short simple (void)
{
  return random();
}

/* get high bits from random result - better */
static unsigned short simple2 (void)
{
  return (random() >> 15);
}

/* The following algorithm is recommended by Numerical Recipies: */
/* Best, but needs floating point division */
static unsigned short nr(void)
{
        unsigned short ulRandom = ((float)(0xffff)*rand()/(RAND_MAX+1.0f));
	return(ulRandom);
}

static void run_randomtest( unsigned short (*keygen)(void) )
{
    int i,j;
    unsigned short rnd;

    /* zero bitcount first */
    memset(bitcount,0,16*sizeof(int));

    for(i=0;i<rounds;i++)
    {
	rnd=keygen();
	
	for(j=0;j<16;j++)
	{
	    if(rnd & 1)
		bitcount[j]++;
	    rnd=rnd>>1;
	}
    }
}

static void print_bitcount(void)
{
    int i;
    float worst;
    float ratio;

    printf("Set ratio: ");
    worst=0;

    for(i=0;i<16;i++)
    {
	ratio=(float)bitcount[i]/rounds;
	if(fabs(ratio-0.5f)>worst)
	    worst=fabs(ratio-0.5f);
	printf("%.2f ",ratio);
    }
    printf("  Worst: %.3f\n",worst);
    if(worst>MAX_WORST_ALLOWED) result++;
}


int main(int argc,const char *argv[])
{
    rounds=2000;
    if(argc>1)
    {
	rounds=atoi(argv[1]);
    }

    printf("Running %d rounds.\n\n",rounds);
	
    result=0;

    printf("Generator: classic\n");
    run_randomtest(classic);
    print_bitcount();

    printf("Generator: simple\n");
    run_randomtest(simple);
    print_bitcount();

    printf("Generator: simple2\n");
    run_randomtest(simple2);
    print_bitcount();

    result=0;
    printf("Generator: Numerical Recipes\n");
    run_randomtest(nr);
    print_bitcount();
    return result;
}
