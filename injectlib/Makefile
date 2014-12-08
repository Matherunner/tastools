CXX = g++
CXXFLAGS = -O3 -ffast-math -std=c++11 -m32 -march=native -mtune=native -Wall -Wextra -fPIC -flto
OBJS = injectmain.o symutils.o customhud.o movement.o strafemath.o
OUTPUT = tasinjectlib.so

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CXX) -shared -s $(CXXFLAGS) $(OBJS) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f *.o
