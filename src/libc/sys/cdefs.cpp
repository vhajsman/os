#ifdef __cplusplus
    #include <sys/cdefs.h>
    #include <kernel/core/memory/memory.h>
    
    void* operator new(size_t size) {
        return malloc(size);  // Nahraďte vlastní alokační funkcí
    }

    void operator delete(void* ptr) {
        free(ptr);  // Nahraďte vlastní de-alokační funkcí
    }

#endif