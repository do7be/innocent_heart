# syntax=docker/dockerfile:1

FROM gcc:9.2
# FROM debian:bullseye
# FROM thewtex/opengl:latest

WORKDIR /app

COPY Innocent_Heart.c ./
COPY icon.o ./
COPY Makefile.Docker ./Makefile
COPY image ./

RUN apt-get update
RUN apt-get install -y wget unzip make gcc
RUN apt-get install -y freeglut3 freeglut3-dev libglu1-mesa-dev mesa-common-dev libgl1-mesa-glx libc6-dev libx11-dev
# RUN export DISPLAY=host.docker.internal:0.0
# RUN export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/mea/libGL.so.1.2.0

RUN wget http://teacher.nagano-nct.ac.jp/ito/Springs_of_C/glpng.zip
RUN unzip glpng.zip
RUN cd src && \
  make -f Makefile.LINUX
RUN cp lib/libglpng.a /usr/lib/
RUN cp include/GL/glpng.h /usr/include/GL/

RUN make

CMD [ "./game" ]
