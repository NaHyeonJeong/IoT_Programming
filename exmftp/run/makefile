PROGS 	= ftp_client1 ftp_server1
CC 	    = gcc
LDLIBS 	= practical.o
CFLAGS 	= -std=c99 -Wall -D_GNU_SOURCE -DLINUX 

# Common temp files to delete from each directory.
TEMPFILES = core core.* *.o temp.* *.out typescript*

all: ${LDLIBS} ${PROGS} 

clean:
	rm -f ${PROGS} ${TEMPFILES} file.hole
