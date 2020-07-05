#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "safer_rand.h"

// max uint16_t
#define MAX_RAND_NUM (65535)
// How much to buffer out of /dev/urandom
#define RAND_BUF_LEN (128)

// Random number buffers
static uint16_t rand_buf[RAND_BUF_LEN];
static int      urand_fd = 0;
static size_t   pos = 0;



void
init_rand(void)
{
    urand_fd = open("/dev/urandom", O_RDONLY);

    if (urand_fd <= 0) {
        fprintf(stderr, "error: failed to open %s\n", "/dev/urandom");
        exit(1);
    }

    ssize_t n = read(urand_fd, rand_buf, sizeof(rand_buf));

    if (n != sizeof(rand_buf)) {
        fprintf(stderr, "error: read returned %zu bytes, expected %zu\n",
                n, sizeof(rand_buf));
        exit(1);
    }

    pos = 0;

    return;
}



size_t
safer_rand(const size_t min,
           const size_t max)
{
    // A simple function to generate a random number between
    // min and max with a decent distribution.

    if (!urand_fd) {
        fprintf(stderr, "error: do not call safer_rand before init!\n");
        exit(1);
    }

    if (max <= min) {
        fprintf(stderr, "error: invalid arguments: min=%zu, max=%zu\n",
                min, max);
        exit(1);
    }

    size_t range = (max + 1) - min;
    size_t shift = MAX_RAND_NUM % range;

again:
    if (pos > RAND_BUF_LEN - 1) {
        ssize_t n = read(urand_fd, rand_buf, sizeof(rand_buf));

        if (n != sizeof(rand_buf)) {
            fprintf(stderr, "error: read returned %zu bytes, expected %zu\n",
                    n, sizeof(rand_buf));
            exit(1);
        }

        pos = 0;
    }

    uint16_t value = rand_buf[pos];

    ++pos;

    if (value < shift) {
        goto again;
    }

    size_t result = ((value - shift) % range) + min;

    return result;
}
