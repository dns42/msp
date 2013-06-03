#ifndef MSP_DEFS_H
#define MSP_DEFS_H

#include <endian.h>

#define htoavr(_h) ({                   \
    typeof((typeof(_h))0) __s;          \
    switch (sizeof(_h)) {               \
    case 1:                             \
        __s = (_h);                     \
        break;                          \
    case 2:                             \
        __s = htole16(_h);              \
        break;                          \
    case 4:                             \
        __s = htole32(_h);              \
        break;                          \
    case 8:                             \
        __s = htole64(_h);              \
        break;                          \
    }                                   \
    __s;                                \
})

#define avrtoh(_s) ({                   \
    typeof((typeof(_s))0) __h;          \
    switch (sizeof(_s)) {               \
    case 1:                             \
        __h = (_s);                     \
        break;                          \
    case 2:                             \
        __h = le16toh(_s);              \
        break;                          \
    case 4:                             \
        __h = le32toh(_s);              \
        break;                          \
    case 8:                             \
        __h = le64toh(_s);              \
        break;                          \
    }                                   \
    __h;                                \
})

#define array_size(_a)                  \
    (sizeof(_a) / sizeof(_a[0]))

#define for_each_bit(_bit, _bits)                               \
    for (;                                                      \
         (_bit) = ffs(*(_bits)),                                \
             (_bit) = (_bit) ? (1<<((_bit)-1)) : 0;             \
         *(_bits) &= ~(_bit))

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
