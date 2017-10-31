/*
 * lwan - simple web server
 * Copyright (c) 2012 Leandro A. F. Pereira <leandro@hardinfo.org>
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

#pragma once

#include <stddef.h>
#if defined(__x86_64__)
#include <stdint.h>
typedef uintptr_t coro_context[10];
#elif defined(__i386__)
#include <stdint.h>
typedef uintptr_t coro_context[7];
#else
#include <ucontext.h>
typedef ucontext_t coro_context;
#endif

#define DEFAULT_BUFFER_SIZE 4096
#define ALWAYS_INLINE inline __attribute__((always_inline))

struct coro;
typedef struct coro coro_t;

typedef int    (*coro_function_t)	(struct coro *coro, void *data);

typedef struct coro_switcher {
    coro_context caller;
    coro_context callee;
}coro_switcher_t;

struct coro *coro_new(struct coro_switcher *switcher, coro_function_t function, void *data);
void	coro_free(struct coro *coro);

void    coro_reset(struct coro *coro, coro_function_t func, void *data);

int	coro_resume(struct coro *coro);
int	coro_resume_value(struct coro *coro, int value);
int	coro_yield(struct coro *coro, int value);

void    coro_defer(struct coro *coro, void (*func)(void *data), void *data);
void    coro_defer2(struct coro *coro, void (*func)(void *data1, void *data2),
            void *data1, void *data2);

void    coro_deferred_run(struct coro *coro, size_t generation);
size_t  coro_deferred_get_generation(const struct coro *coro);

void   *coro_malloc(struct coro *coro, size_t sz)
            __attribute__((malloc));
void   *coro_malloc_full(struct coro *coro, size_t size, void (*destroy_func)())
            __attribute__((malloc));
char   *coro_strdup(struct coro *coro, const char *str);
char   *coro_strndup(struct coro *coro, const char *str, size_t len);
char   *coro_printf(struct coro *coro, const char *fmt, ...);

#define CORO_DEFER(fn)		((void (*)(void *))(fn))
#define CORO_DEFER2(fn)		((void (*)(void *, void *))(fn))

struct lwan_array {
    void *base;
    size_t elements;
};

int lwan_array_init(struct lwan_array *a);
int lwan_array_reset(struct lwan_array *a);
void *lwan_array_append(struct lwan_array *a, size_t element_size);
void lwan_array_sort(struct lwan_array *a, size_t element_size, int (*cmp)(const void *a, const void *b));
struct lwan_array *coro_lwan_array_new(struct coro *coro);

#define LIKELY_IS(x,y)        __builtin_expect((x), (y))
#define LIKELY(x)       LIKELY_IS(!!(x), 1)
#define UNLIKELY(x)     LIKELY_IS((x), 0)

#define DEFINE_ARRAY_TYPE(array_type_, element_type_) \
    struct array_type_ { \
        struct lwan_array base; \
    }; \
    __attribute__((unused)) \
    static inline int array_type_ ## _init(struct array_type_ *array) \
    { \
        return lwan_array_init((struct lwan_array *)array); \
    } \
    __attribute__((unused)) \
    static inline int array_type_ ## _reset(struct array_type_ *array) \
    { \
        return lwan_array_reset((struct lwan_array *)array); \
    } \
    __attribute__((unused)) \
    static inline element_type_ * array_type_ ## _append(struct array_type_ *array) \
    { \
        return (element_type_ *)lwan_array_append((struct lwan_array *)array, sizeof(element_type_)); \
    } \
    __attribute__((unused)) \
    static inline void array_type_ ## _sort(struct array_type_ *array, int (*cmp)(const void *a, const void *b)) \
    { \
        lwan_array_sort((struct lwan_array *)array, sizeof(element_type_), cmp); \
    } \
    __attribute__((unused)) \
    static inline struct array_type_ *coro_ ## array_type_ ## _new(struct coro *coro) \
    { \
        return (struct array_type_ *)coro_lwan_array_new(coro); \
    }
