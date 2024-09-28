#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (!(argc == 5)) {
        printf("Usage: %s [num1] [num2] [num3] [num4]\n", argv[0]);
        return -1;
    }

    
    int num1, num2, num3, num4;
    num1 = atoi(argv[1]);
    num2 = atoi(argv[2]);
    num3 = atoi(argv[3]);
    num4 = atoi(argv[4]);

    // fibonacci 계산 호출 
    int fib=0;
    if (num1 >= 0) {
        fib = fibonacci(num1);
    } else {
        fib = 0; // 음수인 경우
    }

    int max=0;
    max = max_of_four_int(num1, num2, num3, num4);

    printf("%d %d\n", fib, max);


    return 0;
}
