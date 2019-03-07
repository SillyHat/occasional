#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    char dump[512];
    srand((unsigned)time(NULL));
    int lim;
    do {
        printf("\033[1;34mHello~, please enter an upper limit.\033[0m\n");
        fgets(dump, sizeof dump, stdin);
        lim = atoi(dump);
    } while (lim < 2);
    int stuff = rand() % lim + 1;
    printf("\033[1;34mNow please guess a number between "
        "1 and %d.\033[0m\n", lim);
    int steps = 0;
    while (1 < 2) {
        ++steps;
        fgets(dump, sizeof dump, stdin);
        int buf = atoi(dump);
        if (buf < stuff)
            printf("\033[1;31mToo low!\033[0m\n");
        else if (buf > stuff)
            printf("\033[1;33mToo high!\033[0m\n");
        else
            break;
    }
    printf("\033[1;36mYou guessed right! It's "
        "\033[1;35m%d\033[1;36m.\033[0m\n", stuff);
    printf("\033[1;33mIt took you \033[1;36m%d\033[1;33m "
        "guesses.\033[0m\n",
        steps);
    if (steps <= ceil(log2(lim)))
        printf("\033[1;32mThat's nice!\033[0m\n");
    else
        printf("\033[1;31mNot nice!\033[0m\n");
    return 0;
}
