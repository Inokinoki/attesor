/*
 * Simple x86_64 Test Program - Pure Assembly
 * No external dependencies - just basic arithmetic
 */

int main()
{
    volatile int x = 5;
    volatile int y = 3;
    volatile int result;

    /* This will compile to simple x86_64 instructions */
    result = x + y;    /* ADD instruction */

    /* Prevent optimization */
    if (result > 0)
    {
        x = result;
    }

    return result;
}
