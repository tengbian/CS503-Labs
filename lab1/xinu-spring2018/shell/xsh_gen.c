#include <xinu.h>

#define NUM_WORDS  30

static unsigned char *words[NUM_WORDS] = {
  "save", "scan", "scanner", "screen", "screenshot",
  "script", "scroll", "security", "server", "shareware",
  "shell", "shift", "snapshot", "social", "software",
  "spam", "spammer", "spreadsheet", "status", "spyware",
  "supercomputer", "surf", "syntax", "backup", "bandwidth",
  "binary", "bit", "bitmap", "bite", "blog",
};

shellcmd xsh_gen(int nargs, char* argv[]) {
    for (int i=0; i<NUM_WORDS; i++) {
        printf("%s ", words[i]);

        if (i % 10 == 9)
            sleepms(2000);
    }
	return 0;
}
