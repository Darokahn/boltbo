#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#define LETTERCOUNT 5
#define STRINGSIZE 6

#define NONALPHA 0
#define TOOSHORT 1
#define WHITESPACE 2
#define GOOD 3

#define GRAY 0
#define YELLOW 1
#define GREEN 2

int validateWord(char* word) {
    int returnVal = GOOD;
    for (int i = 0; i < LETTERCOUNT; i++) {
        if (isalpha(word[i])) continue;
        if (isspace(word[i])) {
            returnVal = WHITESPACE;
            continue;
        }
        if (word[i] == 0) return TOOSHORT;
        return NONALPHA;
    }
    return returnVal;
}

int main() {
    char secretWord[STRINGSIZE];
start:
    printf("Input the secret word (truncated to %d letters): ", LETTERCOUNT);
    fgets(secretWord, STRINGSIZE, stdin);
    secretWord[LETTERCOUNT] = 0;

    int status = validateWord(secretWord);
    if (status != GOOD) {
        printf("The word must be 5 alphabet characters.\n");
        goto start;
    }
    if (status != TOOSHORT);
    char c;
    char guess[STRINGSIZE];
    uint8_t secretWordCounts[26] = {0};
    for (int i = 0; i < LETTERCOUNT; i++) secretWordCounts[secretWord[i] - 'a'] += 1;
    while (true) {
        fflush(stdin);
        printf("\n");
        printf("Input a guess (truncated to %d letters): ", LETTERCOUNT);
getGuess:
        fgets(guess, STRINGSIZE, stdin);
        guess[LETTERCOUNT] = 0;
        int valid = validateWord(guess);
        if (valid == WHITESPACE) goto getGuess;
        if (valid != GOOD) {
            printf("\nThe word must be 5 alphabet characters.\n");
            continue;
        }
        uint8_t guessCounts[26] = {0};
        int status[5];
        int correctCount = 0;
        for (int i = 0; i < LETTERCOUNT; i++) {
            if (secretWord[i] != guess[i]) continue;
            status[i] = GREEN;
            correctCount++;
            guessCounts[guess[i] - 'a'] += 1;
        }
        if (correctCount == LETTERCOUNT) break;
        for (int i = 0; i < LETTERCOUNT; i++) {
            uint8_t c = guess[i];
            if (c == secretWord[i]) continue;
            if (secretWordCounts[c - 'a'] > guessCounts[c - 'a']) {
                status[i] = YELLOW;
                guessCounts[c - 'a'] += 1;
                continue;
            }
            status[i] = GRAY;
            guessCounts[c - 'a'] += 1;
        }
        for (int i = 0; i < LETTERCOUNT; i++) {
            printf("%d", status[i]);
        }
        printf("\n");
    }
    printf("You win!\n");
}
