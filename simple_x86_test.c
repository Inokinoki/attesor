/*
 * Simple x86_64 Test Program
 * This is designed to be cross-compiled for x86_64 and then translated to ARM64
 */

#include <stdio.h>

/* Simple function that tests basic operations */
int add_numbers(int a, int b)
{
    return a + b;
}

/* Function with a loop */
int factorial(int n)
{
    if (n <= 1)
        return 1;
    return n * factorial(n - 1);
}

/* Main test function */
int main()
{
    int x = 5;
    int y = 3;
    int result;

    /* Test addition */
    result = add_numbers(x, y);
    printf("Addition: %d + %d = %d\n", x, y, result);

    /* Test factorial */
    result = factorial(5);
    printf("Factorial(5) = %d\n", result);

    /* Test loop */
    int sum = 0;
    for (int i = 1; i <= 10; i++)
    {
        sum += i;
    }
    printf("Sum 1 to 10: %d\n", sum);

    /* Test conditional */
    if (sum > 50)
    {
        printf("Sum is greater than 50\n");
    }
    else
    {
        printf("Sum is 50 or less\n");
    }

    return 0;
}
