# makefile for opengl (glut) applications
# author: milix (milix_gr@hotmail.com)

# add here the name of the project 
PROJ = lesson11
PROJ2 = lesson11-2
ENDIAN = endian_test
BMPRW = bmprw_test
FOLD_UP = fold_up
LERP = lerp
LERP2 = lerp2
LERP3 = lerp3

# aliases
CC = g++
RC = windres
RM = -rm -f
CP = -copy

# include and library paths
INCPATH = -I/app/include 
LIBPATH = -L/app/lib

# compiler and linker switches
CFLAGS = -g -fomit-frame-pointer -Wall
#LDFLAGS = -lGLU -lGL -lXmu -lXext -lX11 -lm -lglut -L/usr/lib64 -lstdc++
LDFLAGS = -lstdc++

# add here additional object files
OBJS = $(PROJ).o  
OBJS2 = $(PROJ2).o  
ENDIAN_OBJ = $(ENDIAN).o  
BMPRW_OBJ = $(BMPRW).o  
SHARED = bmprw.o
FOLD_UP_OBJ = $(FOLD_UP).o
LERP_OBJ = $(LERP).o
LERP_OBJ2 = $(LERP2).o
LERP_OBJ3 = $(LERP3).o

all: $(BMPRW).exe $(PROJ).exe $(PROJ2).exe $(ENDIAN).exe $(FOLD_UP).exe \
	$(LERP).exe $(LERP2).exe $(LERP3).exe

$(LERP3).exe: $(LERP_OBJ3) $(SHARED) M3D_math3d.o M3D_plane.o $(LERP3).h
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

$(LERP2).exe: $(LERP_OBJ2) $(SHARED) M3D_math3d.o M3D_plane.o
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

$(LERP).exe: $(LERP_OBJ) $(SHARED) M3D_math3d.o M3D_plane.o
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

$(FOLD_UP).exe: $(FOLD_UP_OBJ) $(SHARED) M3D_math3d.o M3D_plane.o
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

$(PROJ).exe: $(OBJS) $(SHARED)
	$(CC) $(LIBPATH) -o $@ $(SHARED) $(OBJS) $(LDFLAGS)

$(PROJ2).exe: $(OBJS2) $(SHARED)
	$(CC) $(LIBPATH) -o $@ $(SHARED) $(OBJS2) $(LDFLAGS)

$(ENDIAN).exe: $(ENDIAN_OBJ)
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

$(BMPRW).exe: $(BMPRW_OBJ) $(SHARED)
	$(CC) $(LIBPATH) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(INCPATH) $(CFLAGS) -c $*.c

%.o: %.cpp
	$(CC) $(INCPATH) $(CFLAGS) -c $*.cpp

%.o: %.rc
	$(RC) -i $*.rc -o $*.o


bmprw.o: bmprw.cpp bmprw.h


clean:
	$(RM) *.o
	$(RM) *.exe
	$(RM) $(SHARED)
	$(RM) *_dst.bmp

