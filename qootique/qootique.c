#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void InitBoard(int16_t *board)
{
    printf("Making a game board...\n");
    *board = rand() % 25 + 25;
}

void PrintBoard(const int16_t board)
{
    if (!board) {
        return;
    }
    printf("\x1B[0m==============================\n");
    for (int32_t i = 1; i <= board; ++i) {
        printf("k ");
        if (i % 15 == 0 || i == board) {
            printf("\n");
        }
    }
    printf("\x1B[0m==============================\n\n");
    //printf("\nKeys left: %d\n", board);
}

int8_t MakeStrategy(const int16_t board)
{
    sleep(1);
    
    if (board <= 3) {
        return board;
    }
    
    int8_t strategy, mindset = rand() % 10;
    
    if (mindset == 0 || board % 4 == 0) {
        strategy = rand() % 3 + 1;
    } else {
        strategy = board % 4;
    }
    return strategy;
}

void StartGameSingle(int16_t *board)
{
    int8_t hand, turn = rand() % 2;
    char dump[16];
    
    do {
        if (turn) {
            hand = MakeStrategy(*board);
            printf("\x1B[36mPigeons take: %d\n", hand);
            *board -= hand;
            PrintBoard(*board);
            turn = !turn;
        } else {
            printf("\x1B[33mKitties take: ");
            fgets(dump, sizeof dump, stdin);
            hand = atoi(dump);
            
            if (hand >= 1 && hand <= 3 && *board - hand >= 0) {
                *board -= hand;
                PrintBoard(*board);
                turn = !turn;
            } else {
                printf("\x1B[0mCan't do that. Try again.\n");
            }
        }
    } while (*board);
    
    if (turn) {
        printf("\x1B[1;33mKitties win!\x1B[0m\n");
    } else {
        printf("\x1B[1;36mPigeons win!\x1B[0m\n");
    }
}

void StartGameMulti(int16_t *board)
{
    int8_t hand, turn = rand() % 2;
    char dump[16];
    
    do {
        if (turn) {
            printf("\x1B[36mPigeons take: ");
        } else {
            printf("\x1B[33mKitties take: ");
        }
        
        fgets(dump, sizeof dump, stdin);
        hand = atoi(dump);
        
        if (hand >= 1 && hand <= 3 && *board - hand >= 0) {
            *board -= hand;
            PrintBoard(*board);
            turn = !turn;
        } else {
            printf("\x1B[0mCan't do that. Try again.\n");
        }
    } while (*board);
    
    if (turn) {
        printf("\x1B[1;33mKitties win!\x1B[0m\n");
    } else {
        printf("\x1B[1;36mPigeons win!\x1B[0m\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("\x1B[31mSorry, can't do that.\x1B[0m\n");
        printf("Usage: ./Qootique <mode> (single or multiplayer)\n");
        return -1;
    }
    
    srand(time(NULL));
    
    int16_t keys;
    InitBoard(&keys);
    PrintBoard(keys);
    
    if (argv[1][0] == 'm') {
        printf("Two players mode.\n");
        StartGameMulti(&keys);
    } else {
        printf("Playing w/ AI.\n");
        StartGameSingle(&keys);
    }
    
    return 0;
}
