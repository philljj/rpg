#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "safer_rand.h"

// How much to buffer out of /dev/urandom
#define RAND_BUF_LEN (128)

static uint16_t rpg_rand_get_uint16(void);

// Random number buffers
static uint16_t rand_buf[RAND_BUF_LEN];
static int      urand_fd = 0;
static size_t   pos = 0;

void
rpg_rand_init(void)
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
rpg_rand(const size_t min,
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

    if (max >= MAX_RAND_NUM) {
        fprintf(stderr, "error: max >= MAX_RAND_NUM: max=%zu, max allowed=%d\n",
                max, MAX_RAND_NUM - 1);
        exit(1);
    }

    size_t   range = (max + 1) - min;
    size_t   shift = MAX_RAND_NUM % range;
    uint16_t value;

again:
    value = rpg_rand_get_uint16();

    if (value < shift) {
        goto again;
    }

    size_t result = ((value - shift) % range) + min;

    return result;
}

static uint16_t
rpg_rand_get_uint16(void)
{
    ssize_t  n = 0;
    uint16_t value = 0;

    if (pos >= RAND_BUF_LEN) {
        pos = 0;
        n = read(urand_fd, rand_buf, sizeof(rand_buf));

        if (n != sizeof(rand_buf)) {
            fprintf(stderr, "error: read returned %zu bytes, expected %zu\n",
                    n, sizeof(rand_buf));
            exit(1);
        }
    }

    value = rand_buf[pos++];
    return value;
}
