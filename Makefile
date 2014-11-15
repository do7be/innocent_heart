#
#   Makefile
#


#   �ŏI�ړI�̃t�@�C��
TARGET = j06441.exe

#   �\�[�X�t�@�C��(*.c)�̈ꗗ
SRCS = Innocent_Heart.c

#   �I�u�W�F�N�g�t�@�C��(*.o)�̈ꗗ
OBJS = ${SRCS:.c=.o}

#   �w�b�_�t�@�C���̈ꗗ
HEADERS =

#   �R���p�C���E�����J�̎w��
CC = gcc 
CCFLAGS = -Wall -I/usr/include/opengl 
LD = gcc 
LDFLAGS = #-mwindows -mno-cygwin#DOS��������s�ł���悤�ɂ���(������)
LIBS = -lglpng -lglut32 -lglu32 -lopengl32 icon.o

#   OBJS����TARGET�������@
$(TARGET) : $(OBJS)
	 $(LD) $(OBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

#   *.c ����*.o�������@
.c.o :
	 $(CC) $(CCFLAGS) -c $<

#   *.o �� HERDERS �� Makefile �Ɉˑ�(����炪������������Ƃ��ɂ�*.o���č\�z)
$(OBJS) : $(HEADERS) Makefile

#   make clean�Ƃ����Ƃ��Ɏ��s�����R�}���h
clean :
	 rm -f $(TARGET) $(OBJS) core *~
