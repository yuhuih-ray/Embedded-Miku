# Makefile for building embedded application.
# by Brian Fraser

# Edit this file to compile extra C files into their own programs.
TARGET= wave_player

SOURCES= led.c


PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror


# Asound process:
# get alibsound2 lib on target:
# 	# apt-get install libasound2
# Copy target's /usr/lib/arm-linux-gnueabihf/libasound.so.2.0.0 
#      to host  ~/public/asound_lib_BBB/libasound.so
# Copy to just base library:

LFLAGS = -Llibs


# -pg for supporting gprof profiling.
#CFLAGS += -pg


test:
	gcc $(CFLAGS) $(SOURCES) -o test

all: wav node
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound

# Copy wave files to the shared folder
wav:
	mkdir -p $(PUBDIR)/beatbox-wav-files/
	cp beatbox-wav-files/* $(PUBDIR)/beatbox-wav-files/ 

node:
	mkdir -p $(PUBDIR)/beatbox-server-copy/
	cp -R server/* $(PUBDIR)/beatbox-server-copy/ 

clean:
	rm -f $(OUTDIR)/$(TARGET)
