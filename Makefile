EXENAME = mutex
OBJS = mp3.o message.o

CXX = clang++
CXXFLAGS = -std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD = clang++
LDFLAGS = -std=c++1y -stdlib=libc++ -lc++abi

all : $(EXENAME)

$(EXENAME) : $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(EXENAME)

mp3.o : mp3.cpp message.cpp
	$(CXX) $(CXXFLAGS) mp3.cpp
	
message.o : message.cpp
	$(CXX) $(CXXFLAGS) message.cpp

clean :
	-rm -f *.o $(EXENAME)
