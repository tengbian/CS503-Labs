/* shell.c  -  shell */

#include <xinu.h>
#include <stdio.h>
#include "shprototypes.h"

/************************************************************************/
/* Table of Xinu shell commands and the function associated with each	*/
/************************************************************************/
const	struct	cmdent	cmdtab[] = {
	{"argecho",	TRUE,	xsh_argecho},
	{"arp",		FALSE,	xsh_arp},
	{"cat",		FALSE,	xsh_cat},
	{"clear",	TRUE,	xsh_clear},
	{"date",	FALSE,	xsh_date},
	{"devdump",	FALSE,	xsh_devdump},
	{"echo",	FALSE,	xsh_echo},
	{"exit",	TRUE,	xsh_exit},
	{"help",	FALSE,	xsh_help},
	{"kill",	TRUE,	xsh_kill},
	{"memdump",	FALSE,	xsh_memdump},
	{"memstat",	FALSE,	xsh_memstat},
	{"netinfo",	FALSE,	xsh_netinfo},
	{"ping",	FALSE,	xsh_ping},
	{"ps",		FALSE,	xsh_ps},
	{"sleep",	FALSE,	xsh_sleep},
	{"udp",		FALSE,	xsh_udpdump},
	{"udpecho",	FALSE,	xsh_udpecho},
	{"udpeserver",	FALSE,	xsh_udpeserver},
	{"uptime",	FALSE,	xsh_uptime},
	{"gen",	    FALSE,	xsh_gen},
	{"count",	FALSE,	xsh_count},
	{"debug",	FALSE,	xsh_debug},
	{"?",		FALSE,	xsh_help}
};

#define true 1
#define false 0

uint32	ncmd = sizeof(cmdtab) / sizeof(struct cmdent);

/************************************************************************/
/* shell  -  Provide an interactive user interface that executes	*/
/*	     commands.  Each command begins with a command name, has	*/
/*	     a set of optional arguments, has optional input or		*/
/*	     output redirection, and an optional specification for	*/
/*	     background execution (ampersand).  The syntax is:		*/
/*									*/
/*		   command_name [args*] [redirection] [&]		*/
/*									*/
/*	     Redirection is either or both of:				*/
/*									*/
/*				< input_file				*/
/*			or						*/
/*				> output_file				*/
/*									*/
/************************************************************************/


// Return the index of matching cmdtab
// Return -1 if not found.
static int32 find_cmdtab_index(char *s) {
    for (int i= 0; i<ncmd; i++) {
        if (strcmp(s, cmdtab[i].cname) == 0)
            return i;
    }
    return -1;
}

static void dump_tokens(int32 *tok, char *tokbuf, int32 ntok, char *s) {
  dprintf("---------------------------\n");
  for (int i=0; i<ntok; i++)
    dprintf("(%s) tok[%d]: [:%d][%s]\n", s, i, tok[i], &tokbuf[tok[i]]);
}


static bool8 handle_builtin(did32 dev,
                            bool8 backgnd,
                            int32 ntok, char *tokbuf, int32 *tok, int32 tlen,
                            char *outname, char *inname) {

	char *args[SHELL_MAXTOK];
    int32 cmdtab_index;

    cmdtab_index = find_cmdtab_index(tokbuf);

    if (cmdtab_index == -1) {
        return false;
    }

    if (!cmdtab[cmdtab_index].cbuiltin) {
      // This shall be handled by handle_non_builtin
      return false;
    }

    if (inname != NULL || outname != NULL || backgnd){
        fprintf(dev, SHELL_BGERRMSG);
    } else {
        /* Set up arg vector for call */
        for (int i=0; i<ntok; i++) {
            args[i] = &tokbuf[tok[i]];
        }

        /* Call builtin shell function */
        if ((*cmdtab[cmdtab_index].cfunc)(ntok, args) == SHELL_EXIT) {
            return true;
        }
    }
    return true;
}

static bool8 handle_non_builtin(did32 dev, bool8 backgnd,
                                int32 ntok, char *tokbuf, int32 *tok, int32 tlen, int32 *toktyp,
                                did32 stdinput, did32 stdoutput, char *outname, char *inname) {
    // LAB2: TODO: Modify this function to (1) create pipe, (2) connect pipe, and
    // (3) replace stdin/stdout

	int32 tmparg;
    int msg;
	pid32 childs[SHELL_MAXTOK];
    int32 cmdtab_index;

	did32 devpipe;

    for (int i=0; i<SHELL_MAXTOK; i++)
        childs[i] = -1;

    /* Open files and redirect I/O if specified */
    if (inname != NULL) {
        stdinput = open(NAMESPACE,inname,"ro");
        if (stdinput == SYSERR) {
            fprintf(dev, SHELL_INERRMSG, inname);
            return false;
        }
    }
    if (outname != NULL) {
        stdoutput = open(NAMESPACE,outname,"w");
        if (stdoutput == SYSERR) {
            fprintf(dev, SHELL_OUTERRMSG, outname);
            return false;
        } else {
            control(stdoutput, F_CTL_TRUNC, 0, 0);
        }
    }

    int cur = 0;
	int pprev_cur = 0;
	int prev_cur = 0;
    int next_cur = 0;

    dump_tokens(tok, tokbuf, ntok, "non-built-in");

    while (cur < ntok) {

        if (toktyp[cur] == SH_TOK_OTHER) {
            cmdtab_index = find_cmdtab_index(&tokbuf[tok[cur]]);
            if (cmdtab_index == -1) {
                fprintf(dev, "command %s not found\n", &tokbuf[tok[cur]]);
                return false;
            }

            ASSERT(toktyp[cur] == SH_TOK_OTHER);

            for (next_cur=cur+1; next_cur<ntok; next_cur++) {
                if (toktyp[next_cur] != SH_TOK_OTHER)
                    break;
            }

            ASSERT(toktyp[next_cur] != SH_TOK_OTHER || next_cur >= ntok);
			int num_args = next_cur-cur;
			ASSERT(num_args > 0);
			dprintf("(shell) num args: %d\n", num_args);

			/* Spawn child thread for non-built-in commands */
			childs[cur] = create(cmdtab[cmdtab_index].cfunc,
							   SHELL_CMDSTK, SHELL_CMDPRIO,
							   cmdtab[cmdtab_index].cname, 2, num_args, &tmparg);


			/* Set stdinput and stdoutput in child to redirect I/O */
			proctab[childs[cur]].prdesc[0] = stdinput;
			proctab[childs[cur]].prdesc[1] = stdoutput;

			/* If creation or argument copy fails, report error */
			if ((childs[cur] == SYSERR) ||
				(addargs(childs[cur], num_args, &tok[cur], tlen, &tokbuf[cur], &tmparg) == SYSERR) ) {
				fprintf(dev, SHELL_CREATMSG);
				return false;
			}

			// if the format is: other | other, then create pipe
			if(toktyp[prev_cur] == SH_TOK_STICK){
				devpipe = pipcreate();
				pipconnect(devpipe, childs[pprev_cur], childs[cur]);
				proctab[childs[pprev_cur]].prdesc[1] = devpipe;
				proctab[childs[cur]].prdesc[0] = devpipe;
			}

        } else {
			if(cur == 0 || toktyp[cur] != SH_TOK_STICK){
				fprintf(dev,"%s (parsing)\n", SHELL_SYNERRMSG);
				return false;
			}
			next_cur++;
        }
		pprev_cur = prev_cur;
		prev_cur = cur;
        cur = next_cur;
    }

	int procnum = 0;// record number of process
    msg = recvclr();
    for (int i=0; i<SHELL_MAXTOK; i++) {
        if (childs[i] == -1)
            continue;
        resume(childs[i]);
		procnum++;
   }

	// wait until all child process exit
	int procflag = 0;
	if(procnum > 0)
		procflag = TRUE;

	while (procflag){
		msg = receive();
		for (int i=0; i<SHELL_MAXTOK; i++){
			if (childs[i] == msg)
				childs[i] = -1;
		}
		procflag = FALSE;
		for ( int i=0; i<SHELL_MAXTOK; i++){
			if(childs[i] != -1)
				procflag = TRUE;
		}
	}

    return true;
}


process	shell (
    did32	dev		/* ID of tty device from which	*/
	)				/*   to accept commands		*/
{
    char	buf[SHELL_BUFLEN];
	int32	len;
	char	tokbuf[SHELL_BUFLEN + SHELL_MAXTOK];
	int32	tlen;
	int32	tok[SHELL_MAXTOK];

	int32	toktyp[SHELL_MAXTOK];	/* Type of each token in tokbuf	*/
	int32	ntok;			/* Number of tokens on line	*/
	bool8	backgnd;		/* Run command in background?	*/
	char	*outname, *inname;	/* Pointers to strings for file	*/
	did32	stdinput, stdoutput;	/* Descriptors for redirected	*/

	/* Print shell banner and startup message */
	fprintf(dev, "\n\n%s%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
            SHELL_BAN0,SHELL_BAN1,SHELL_BAN2,SHELL_BAN3,SHELL_BAN4,
            SHELL_BAN5,SHELL_BAN6,SHELL_BAN7,SHELL_BAN8,SHELL_BAN9);

	fprintf(dev, "%s\n\n", SHELL_STRTMSG);

	/* Continually prompt the user, read input, and execute command	*/
	while (TRUE) {
        memset(tokbuf, 0, sizeof(tokbuf));
        memset(tok, 0, sizeof(tok));
        memset(buf, 0, sizeof(buf));

		/* Display prompt */
		fprintf(dev, SHELL_PROMPT);

		/* Read a command */
		len = read(dev, buf, sizeof(buf));

        // dprintf("buf: [%d][%s]\n", len, buf);

		/* Exit gracefully on end-of-file */

		if (len == EOF) {
			break;
		}

		/* If line contains only NEWLINE, go to next line */

		if (len <= 1) {
			continue;
		}

		buf[len] = SH_NEWLINE;	/* terminate line */

		/* Parse input line and divide into tokens */
		ntok = lexan(buf, len, tokbuf, &tlen, tok, toktyp);

		/* Handle parsing error */
		if (ntok == SYSERR) {
			fprintf(dev,"%s (parsing)\n", SHELL_SYNERRMSG);
			continue;
		}

		/* If line is empty, go to next input line */

		if (ntok == 0) {
			fprintf(dev, "\n");
			continue;
		}

        dump_tokens(tok, tokbuf, ntok, "after-parsing");

		/* If last token is '&', set background */
		if (toktyp[ntok-1] == SH_TOK_AMPER) {
			ntok-- ;
			tlen-= 2;
			backgnd = TRUE;
		} else {
			backgnd = FALSE;
		}

		/* Check for input/output redirection (default is none) */
		outname = inname = NULL;
		if ( (ntok >=3) && ( (toktyp[ntok-2] == SH_TOK_LESS)
                             ||(toktyp[ntok-2] == SH_TOK_GREATER))){
			if (toktyp[ntok-1] != SH_TOK_OTHER) {
				fprintf(dev,"%s (redir)\n", SHELL_SYNERRMSG);
				continue;
			}
			if (toktyp[ntok-2] == SH_TOK_LESS) {
				inname =  &tokbuf[tok[ntok-1]];
			} else {
				outname = &tokbuf[tok[ntok-1]];
			}
			ntok -= 2;
			tlen = tok[ntok];
		}

		if ( (ntok >=3) && ( (toktyp[ntok-2] == SH_TOK_LESS)
                             ||(toktyp[ntok-2] == SH_TOK_GREATER))){
			if (toktyp[ntok-1] != SH_TOK_OTHER) {
				fprintf(dev,"%s (redir)\n", SHELL_SYNERRMSG);
				continue;
			}
			if (toktyp[ntok-2] == SH_TOK_LESS) {
				if (inname != NULL) {
				    fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				    continue;
				}
				inname = &tokbuf[tok[ntok-1]];
			} else {
				if (outname != NULL) {
				    fprintf(dev,"%s\n", SHELL_SYNERRMSG);
				    continue;
				}
				outname = &tokbuf[tok[ntok-1]];
			}
			ntok -= 2;
			tlen = tok[ntok];
		}


		stdinput = stdoutput = dev;

        // Attempt to handle built-in case.
        // If not handled, try non-built-in case.
        if (!handle_builtin(dev, backgnd,
                            ntok, tokbuf, tok, tlen,
                            outname, inname)) {

            handle_non_builtin(dev, backgnd,
                               ntok, tokbuf, tok, tlen, toktyp,
                               stdinput, stdoutput, outname, inname);
        }
    }

    /* Terminate the shell process by returning from the top level */

    fprintf(dev,SHELL_EXITMSG);
    return OK;
}
