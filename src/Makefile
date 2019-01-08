
PROGRAM	=	monitor	

CC= arm-linux-g++
#CC=	g++ 


CFLAG	=	-Wall -Os -DARM_LINUX -D_REENTRANT -DDLIBLOG -mstructure-size-boundary=8

INCLUDE	=	-I../include -I../include-inter 



OBJSMPRO	= main.o param.o process.o config_file.o system.o cpu.o \
              clockrecord.o badblock.o semastat.o msgqueue.o shrm.o \
			  netstat.o fs.o bp.o netevent.o loginevent.o usb.o dangercmd.o \
			  sshloginerr.o

all: $(PROGRAM)
	@arm-linux-strip --strip-all $(PROGRAM)


$(PROGRAM):mpro

mpro:$(OBJSMPRO)
	$(CC) -o $(PROGRAM)  $(OBJSMPRO) -lpthread -L../libs -Tlink.lds 

.cpp.o:
	$(CC) $(CFLAG) $(INCLUDE) -c $<

clean:
	rm -f *.o 
