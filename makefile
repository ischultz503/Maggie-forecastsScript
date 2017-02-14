CC = gcc
CFLAGS = 
IFLAGS  = -I/nfs/crypt/arena/users/model/src/database/emwx_lib/ \
		  -I/usr/lib/gcc/x86_64-redhat-linux/4.1.1/include/ \
          -I/usr/include/mysql
LFLAGS  = -L/usr/lib64/mysql \
          -L/users/model/src/database/emwx_lib/Linux \
          -L/usr/freeware/lib32
LIBS   =  -lemwx_lib -lmysqlclient 

practice.exe: practice.o makefile
	$(CC) $(CFLAGS) practice.o $(LFLAGS) $(LIBS) -o $@

practice.o: practice.c
	$(CC) $(CFLAGS) $(IFLAGS) -c practice.c
