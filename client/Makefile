MYNAME = makefile
CC = gcc

objects = client.o game.o interface.o

client: $(objects)
	cc -g -o client $(objects) -lpthread

game.o: game.c config.h
	cc -c game.c

server.o: client.c config.h
	cc -c client.c

interface.o: interface.c config.h
	cc -c interface.c
#比较稳健的clean做法，表示clean是一个伪目标
.PHONY: clean

#前面-的意思是：也许某些文件出现问题，忽略，继续执行
clean:
	-rm client $(objects) 

