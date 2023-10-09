src += main.c
src += broadcast.c
src += pack_unpack.c
src += create_socket.c
src += filelist.c
src += key_scan_process.c
src += tcp_send_file.c
src += udp_process.c
src += userlist.c

obj = $(src:%.c=%.o)

CC = gcc

OPTION = -lpthread -g -Wall
EXEC_NAME = ipmsg

ipmsg:$(obj)
	@echo make ...
	$(CC) $^ -o $(EXEC_NAME) $(OPTION)
	@echo make over
	@echo =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

%.o:%.c
	$(CC) -c -o $@ $< $(OPTION)

.PHONY:clean
clean:
	@echo clean ...
	rm *.o $(EXEC_NAME) -rf
	@echo -----clean over------
