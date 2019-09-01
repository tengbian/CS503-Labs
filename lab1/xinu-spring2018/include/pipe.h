/* pipe.h: contains all the macros and definition for the pipe type in XINU */

#define MAXPIPES 10
#define PIPE_SIZE 1024

enum pipe_state_t {
	PIPE_FREE,
	PIPE_USED,
	PIPE_CONNECTED,
	PIPE_OTHER
};

struct pipe_t {
	pipid32 pipid;			    // Pipe ID
	enum pipe_state_t state;	// Pipe state defined by the enum
    // LAB2: TODO: Fill the structure with the fields as required.
};

extern struct pipe_t pipe_tables[MAXPIPES];	// Table for all pipes
