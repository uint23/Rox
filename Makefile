CC = c++
CFLAGS = -std=c++20 -Wall -Wextra
CPPFLAGS = -Iinclude
LDFLAGS = -L. 

LDLIBS_WIN = -lopengl32 -lgdi32 -lwinmm
LDLIBS = -lraylib -lm $(LDLIBS_WIN)

SRCS = source/main.cc \
       source/beatmap_parser.cc
OUT = rhythm

all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRCS) $(LDFLAGS) $(LDLIBS) -o $(OUT)

clean:
	rm -f $(OUT)

