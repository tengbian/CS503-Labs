/* pipe.h: contains all the macros and definition for the pipe type in XINU */
#define MAXPIPES 10
#define PIPE_SIZE 10

enum pipe_state_t {
	PIPE_FREE,
	PIPE_USED,
	PIPE_CONNECTED,
	PIPE_RD_CONN,
	PIPE_WT_CONN,
	PIPE_DISCONN,
	PIPE_OTHER
};

struct pipe_t {
	pipid32 pipid;			    // Pipe ID
	enum pipe_state_t state;	// Pipe state defined by the enum
    // LAB2: TODO: Fill the structure with the fields as required.
	pid32 owner;
	pid32 writer;
	pid32 reader;

	char *piphead;
	char *piptail;
	char pipbuffer[PIPE_SIZE];
	sid32	pipwsem;// prod
	sid32	piprsem;// cons

};

struct pipe_t pipe_tables[MAXPIPES];	// Table for all pipes
extern struct pipe_t pipe_tables[MAXPIPES];	// Table for all pipes
extern did32 pipid32_to_did32(pipid32);
extern pipid32 did32_to_pipid32(did32);
