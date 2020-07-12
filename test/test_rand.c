#include <stdio.h>
#include <stdlib.h>

#include "../src/safer_rand.h"

static void print_usage_and_die(void) __attribute__((__noreturn__)); 



int
main(int    argc,
     char * argv[])
{
    if (argc != 4) { print_usage_and_die(); }

    init_rand();

    size_t min = atoi(argv[1]);
    size_t max = atoi(argv[2]);
    size_t num_times = atoi(argv[3]);
    size_t result;

    for (size_t i = 0; i < num_times; ++i) {
        result = safer_rand(min, max);
        printf("%zu\n", result);
    }


    return EXIT_SUCCESS;
}



static void
print_usage_and_die(void)
{
   printf("usage:\n");
   printf("  test_rand <min> <max> <num times>\n");
   printf("\n");
   printf("args:\n");
   printf("  0   <= min       < UINT16_MAX \n");
   printf("  min <  max       < UINT16_MAX \n");
   printf("  1   <  num times < UINT16_MAX \n");
   printf("\n");
   printf("description:\n");
   printf("  Prints a random number between min and max (inclusive), \n");
   printf("  repeated num times. Random data is pulled from /dev/urandom\n");

    exit(EXIT_FAILURE);
}
