#include <stdio.h>
#include <assert.h>
#include "../include/zob.h"

void testFunction() {
    int result = countTodos();
    assert(result == 0); 

    printf("testFunction passed.\n");
}

int main() {
    testFunction();

    printf("All tests passed.\n");
    return 0;
}
