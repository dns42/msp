#ifndef CRT_DEFS_H
#define CRT_DEFS_H

#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#define containerof(_elem, _type, _memb) \
    ((_type *)((void*)(_elem) - offsetof(_type, _memb)))

#define likely(_cond) __builtin_expect(!(_cond), 0)
#define unlikely(_cond) __builtin_expect(!!(_cond), 0)

#ifndef __initcall
#define __initcall __attribute__((constructor))
#endif

#ifndef __exitcall
#define __exitcall __attribute__((destructor))
#endif

#ifndef __printf
#define __printf(_f, _a) __attribute__((format(printf, _f, _a)))
#endif

#define min(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define max(_a, _b) ((_a) > (_b) ? (_a) : (_b))

#define array_size(_a) (sizeof(_a) / sizeof(_a[0]))

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
