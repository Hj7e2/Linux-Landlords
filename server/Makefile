MYNAME = makefile
CC = gcc

objects = server.o game.o

server: $(objects)
	cc -g -o server $(objects) -lpthread

game.o: game.c config.h
	cc -c game.c

server.o: server.c config.h
	cc -c server.c
#比较稳健的clean做法，表示clean是一个伪目标
.PHONY: clean

#前面-的意思是：也许某些文件出现问题，忽略，继续执行
clean:
	-rm server $(objects) 

