#ifndef __CLASS_H
#define __CLASS_H

#ifdef __cplusplus
#error "C++ not supported."
#endif

#define __class_constructorReturn void

#define Class(className, constructorArgs, ...)              \
    __class_constructorReturn ConstRef(className)           \
        constructorArgs;                                    \
                                                            \
    struct className##_class {                              \
        __VA_ARGS__;                                        \
        __class_constructorReturn(*className)               \
            constructorArgs                                 \
            = &className##_construct;                       \
    };                                                      \
                                                            \
    typedef className;                                      \
                                                            \
    __class_constructorReturn ConstRef(className)           \
        constructorArgs                                     \

    
#define ConstRef(className) \
    className##_construct
#define Constructor(className) \
    ConstRef(className)

#endif
