CXX = c++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wno-missing-field-initializers
CXXPPFLAGS = -Iinclude
LDFLAGS = -L. 

LDLIBS_WIN = -lopengl32 -lgdi32 -lwinmm
LDLIBS = -lraylib -lm -lz $(LDLIBS_WIN)

SRCS = source/main.cc \
       source/beatmap_parser.cc
OUT = rhythm

all:
	$(CXX) $(CXXFLAGS) $(CXXPPFLAGS) $(SRCS) $(LDFLAGS) $(LDLIBS) -o $(OUT)

clean:
	rm -f $(OUT)

