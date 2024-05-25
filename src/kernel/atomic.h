#ifndef __ATOMIC_H
#define __ATOMIC_H

#include "kernel.h"

#define __atomic_lock u8

#define __atomic_getlock(identifier) \
    ___atomic_##identifier##_lock

#define __atomic_define_lock(identifier) \
    __atomic_lock __atomic_getlock(identifier) = 0

#define atomic(type, identifier) \
    type identifier;             \
    __atomic_define_lock

#define atomic_set(identifier) \
    if(__atomic_getlock(identifier) == 0)

#define atomic_lock(identifier) \
    __atomic_getlock(identifier) = 1

#define atomic_unlock(identifier) \
    __atomic_getlock(identifier) = 0

#endif

