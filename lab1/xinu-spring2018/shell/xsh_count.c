#include <xinu.h>
#include <stdio.h>

#define MAX_WORD_LEN 1024

shellcmd xsh_count(int argc, char *argv[]) {
    int word_count = 0;
    char word[MAX_WORD_LEN];

	while (TRUE) {
        uint32 retval = getc(stdin);

        if (retval == '\0')
            continue;

        if (retval == SYSERR)
            break;

        char ch = (char)retval;
        word[word_count] = ch;

        if (ch == ' ' || ch == '\n' || ch == '\r') {
            if (word_count > 0) {
                word[word_count] = '\0';
                printf("count: %d [%s]\n", word_count, word);
            }
            word_count = 0;
        } else {
            word_count++;
            if (word_count >= MAX_WORD_LEN) {
              printf("ERROR: Reached a maximum word length\n");
              return 1;
            }
        }
	}
	return 0;
}
