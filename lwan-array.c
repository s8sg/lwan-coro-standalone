/*
 * lwan - simple web server
 * Copyright (c) 2017 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#include "lwan-array.h"

#define INCREMENT 16

int
lwan_array_init(struct lwan_array *a)
{
    if (UNLIKELY(!a))
        return -EINVAL;

    a->base = NULL;
    a->elements = 0;

    return 0;
}

int
lwan_array_reset(struct lwan_array *a)
{
    if (UNLIKELY(!a))
        return -EINVAL;

    free(a->base);
    a->base = NULL;
    a->elements = 0;

    return 0;
}


#if !defined (HAS_REALLOCARRAY)

#if !defined(HAVE_BUILTIN_MUL_OVERFLOW)
/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

static inline bool umull_overflow(size_t a, size_t b, size_t *out)
{
    if ((a >= MUL_NO_OVERFLOW || b >= MUL_NO_OVERFLOW) && a > 0 && SIZE_MAX / a < b)
        return true;
    *out = a * b;
    return false;
}
#else
#define umull_overflow __builtin_mul_overflow
#endif

void *reallocarray(void *optr, size_t nmemb, size_t size)
{
    size_t total_size;
    if (UNLIKELY(umull_overflow(nmemb, size, &total_size))) {
        errno = ENOMEM;
        return NULL;
    }
    return realloc(optr, total_size);
}

#endif /* HAS_REALLOCAARRAY */ 


#if !defined(HAVE_BUILTIN_ADD_OVERFLOW)
static inline bool add_overflow(size_t a, size_t b, size_t *out)
{
    if (UNLIKELY(a > 0 && b > SIZE_MAX - a))
        return true;

    *out = a + INCREMENT;
    return false;
}
#else
#define add_overflow __builtin_add_overflow
#endif


void *
lwan_array_append(struct lwan_array *a, size_t element_size)
{
    if (!(a->elements % INCREMENT)) {
        void *new_base;
        size_t new_cap;

        if (UNLIKELY(add_overflow(a->elements, INCREMENT, &new_cap))) {
            errno = EOVERFLOW;
            return NULL;
        }

        new_base = reallocarray(a->base, new_cap, element_size);
        if (UNLIKELY(!new_base))
            return NULL;

        a->base = new_base;
    }

    return ((unsigned char *)a->base) + a->elements++ * element_size;
}

void
lwan_array_sort(struct lwan_array *a, size_t element_size, int (*cmp)(const void *a, const void *b))
{
    if (LIKELY(a->elements))
        qsort(a->base, a->elements - 1, element_size, cmp);
}

static void
coro_lwan_array_free(void *data)
{
    struct lwan_array *array = data;

    lwan_array_reset(array);
    free(array);
}

struct lwan_array *
coro_lwan_array_new(struct coro *coro)
{
    struct lwan_array *array;

    array = coro_malloc_full(coro, sizeof(*array), coro_lwan_array_free);
    if (LIKELY(array))
        lwan_array_init(array);

    return array;
}
