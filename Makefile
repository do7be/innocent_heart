#
#   Makefile
#


#   最終目的のファイル
TARGET = j06441.exe

#   ソースファイル(*.c)の一覧
SRCS = Innocent_Heart.c

#   オブジェクトファイル(*.o)の一覧
OBJS = ${SRCS:.c=.o}

#   ヘッダファイルの一覧
HEADERS =

#   コンパイラ・リンカの指定
CC = gcc 
CCFLAGS = -Wall -I/usr/include/opengl 
LD = gcc 
LDFLAGS = #-mwindows -mno-cygwin#DOS窓から実行できるようにする(未完成)
LIBS = -lglpng -lglut32 -lglu32 -lopengl32 icon.o

#   OBJSからTARGETを作る方法
$(TARGET) : $(OBJS)
	 $(LD) $(OBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

#   *.c から*.oを作る方法
.c.o :
	 $(CC) $(CCFLAGS) -c $<

#   *.o は HERDERS と Makefile に依存(これらが書き換わったときにも*.oを再構築)
$(OBJS) : $(HEADERS) Makefile

#   make cleanとしたときに実行されるコマンド
clean :
	 rm -f $(TARGET) $(OBJS) core *~
