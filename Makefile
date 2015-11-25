BIN			= haloes

# environment setup
CCC 		= gcc
CFLAG		= -Wall -std=c++11 -g -Wextra -DDEBUG -arch x86_64
INCLUDE     = -I /mnt/local/include/
LIB        	= -L /mnt/local/lib/
LFLAG		= -framework OpenGL -framework GLUT -lglew -lAntTweakBar -lstdc++ 
#-lglut

SOURCE		= $(wildcard *.cpp)
OBJECT		= $(SOURCE:.cpp=.o)


all: $(SOURCE) $(BIN)

$(BIN): $(OBJECT)
	$(CCC) $(LFLAG) $(OBJECT) $(LIB) -o $@

%.o : %.cpp
	$(CCC) -c $(INCLUDE) $(CFLAG) $*.cpp

clean:
	rm -f *.o $(BIN)

run: all
	./$(BIN)
