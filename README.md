innocent_heart
==============

project of innocent heart on web

This branch is original source of the "Innocent Heart".

## Installation for Mac

Please download library.

http://openports.se/graphics/glpng

```
$ sudo mkdir /usr/local/include/GL
$ sudo cp -p  ~/Downloads/glpng/include/GL/glpng.h /usr/local/include/GL/
$ cd ~/Downloads/glpng/src
$ mv Makefile.LINUX Makefile
$ vi glpng.c
28,29c28,32
< #include <GL/glpng.h>
< #include <GL/gl.h>
---
> #include <GLUT/glut.h>
> #include "GL/glpng.h"

$ make
$ cp -p ../lib/libglpng.a your/dir/innocent_heart/
$ sudo ln -s /System/Library/Frameworks/GLUT.framework/Headers/glut.h /usr/local/include/GL
```

## Compile

```
$ cd your/dir/innocent_heart/
$ gcc -framework GLUT -framework OpenGL -o game Innocent_Heart.c libglpng.a
$ ./game
```


