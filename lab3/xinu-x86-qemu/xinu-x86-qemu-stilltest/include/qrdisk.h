#ifndef _QRDISK_H
#define _QRDISK_H

#define QRDISK_MAX_BUFF 4096

struct qrds_buff {
  int32	status;
  pid32	pid;
  char	data[RD_BLKSIZ];
};

struct qrds_blk {
  int32	state;
  char	id[RD_IDLEN];
  int32	seq;
  struct qrds_buff buff[QRDISK_MAX_BUFF];
};

extern struct qrds_blk qrds[];

#endif
