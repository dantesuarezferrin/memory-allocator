#include "memalloc.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(myMalloc(0) == NULL);
    myFree(NULL);

    void *a = myMalloc(16);
    void *b = myMalloc(32);
    void *c = myMalloc(64);
    assert(a != NULL && b != NULL && c != NULL);
    assert(((uintptr_t)a % 16) == 0);
    assert(((uintptr_t)b % 16) == 0);
    assert(((uintptr_t)c % 16) == 0);

    myFree(a);
    myFree(b);
    myFree(c);

    void *big = myMalloc(160);
    assert(big == a);
    myFree(big);

    void *part1 = myMalloc(32);
    void *part2 = myMalloc(64);
    assert(part1 == big);
    assert(part2 == (void *)((char *)part1 + 32 + 32));
    myFree(part2);
    myFree(part1);

    assert(myCalloc(0, 16) == NULL);
    assert(myCalloc(16, 0) == NULL);
    assert(myCalloc(SIZE_MAX, 2) == NULL);

    char *dirty = (char *)myMalloc(64);
    memset(dirty, 'X', 64);
    myFree(dirty);

    int *arr = (int *)myCalloc(16, sizeof(int)); // 64 bytes -> recicla 'dirty'
    assert(arr != NULL && (void *)arr == (void *)dirty);
    for (int i = 0; i < 16; i++) {
        assert(arr[i] == 0);
    }
    myFree(arr);

    // 1. myRealloc(NULL, size) == myMalloc(size)
    int *r1 = (int *)myRealloc(NULL, 4 * sizeof(int)); // 16B
    assert(r1 != NULL);
    for (int i = 0; i < 4; i++) {
        r1[i] = (i + 1) * 10; // [10, 20, 30, 40]
    }

    void *neighbor = myMalloc(64);
    void *barrier = myMalloc(32); // Impide que neighbor sea el ultimo o se mueva
    assert(neighbor != NULL && barrier != NULL);
    myFree(neighbor);

    int *r1_expanded = (int *)myRealloc(r1, 16 * sizeof(int)); // Pide 64B
    assert(r1_expanded == r1); // Misma direccion, sin copiar!
    for (int i = 0; i < 4; i++) {
        assert(r1_expanded[i] == (i + 1) * 10);
    }

    int *r1_moved = (int *)myRealloc(r1_expanded, 64 * sizeof(int)); // Pide 256B
    assert(r1_moved != NULL && r1_moved != r1_expanded);
    assert(((uintptr_t)r1_moved % 16) == 0);
    for (int i = 0; i < 4; i++) {
        assert(r1_moved[i] == (i + 1) * 10); // Datos preservados intactos
    }
           (void *)r1_expanded, (void *)r1_moved);

    int *r1_shrunk = (int *)myRealloc(r1_moved, 4 * sizeof(int)); // Reduce de 256B a 16B
    assert(r1_shrunk == r1_moved);
    assert(r1_shrunk[0] == 10 && r1_shrunk[3] == 40);

    assert(myRealloc(r1_shrunk, 0) == NULL); // Equivale a myFree(r1_shrunk)
    myFree(barrier);

    return 0;
}
