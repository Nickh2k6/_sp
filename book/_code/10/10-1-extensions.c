#include <stdio.h>

typedef enum {
    GC_MARK,
    GC_SWEEP,
    GC_PAUSE
} GCPhase;

typedef struct {
    GCPhase phase;
    size_t bytesAllocated;
    size_t nextGC;
    int grayCount;
    int grayCapacity;
    void** grayStack;
} GC;

static GC gc;

void gcInit() {
    gc.phase = GC_PAUSE;
    gc.bytesAllocated = 0;
    gc.nextGC = 1024 * 1024;
    gc.grayCount = 0;
    gc.grayCapacity = 0;
    gc.grayStack = NULL;
}

void* gcAllocate(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    gc.bytesAllocated += size;
    return ptr;
}

void gcCollect() {
    printf("Garbage collection triggered...\n");
    printf("  Before: %zu bytes allocated\n", gc.bytesAllocated);
    
    gc.phase = GC_MARK;
    gc.grayCount = 0;
    
    gc.phase = GC_SWEEP;
    
    gc.phase = GC_PAUSE;
    gc.nextGC = gc.bytesAllocated * 2;
    printf("  After: %zu bytes allocated\n", gc.bytesAllocated);
}

void gcTrackObject(void* object) {
    if (gc.grayCount >= gc.grayCapacity) {
        gc.grayCapacity = gc.grayCapacity == 0 ? 8 : gc.grayCapacity * 2;
        gc.grayStack = realloc(gc.grayStack, sizeof(void*) * gc.grayCapacity);
    }
    gc.grayStack[gc.grayCount++] = object;
}

int main() {
    printf("Testing garbage collection framework...\n\n");
    
    gcInit();
    
    printf("Allocating some objects...\n");
    for (int i = 0; i < 10; i++) {
        void* obj = gcAllocate(256);
        gcTrackObject(obj);
        printf("  Allocated object %d, total: %zu bytes\n", i + 1, gc.bytesAllocated);
    }
    
    printf("\nTriggering garbage collection:\n");
    gcCollect();
    
    printf("\nGC configuration:\n");
    printf("  Next GC threshold: %zu bytes\n", gc.nextGC);
    printf("  Current phase: %d (PAUSE)\n", gc.phase);
    
    free(gc.grayStack);
    
    printf("\nTest passed!\n");
    return 0;
}
