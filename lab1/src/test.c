#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; ++i) {
        if (i % 15 == 0) {
            printf("number = %d, FizzBuzz", i);
        } else if (i % 3 == 0) {
            printf("number = %d, Fizz", i);
        } else if (i % 5 == 0) {
            printf("number = %d, Buzz", i);
        }
    }
}
