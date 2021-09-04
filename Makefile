# pigs

CC = g++
CFLAGS = -std=c++17 -O3 
LDFLAGS = -lpthread 

# image magick 
MAGICKLIB=`Magick++-config --ldflags --libs` 
MAGICKINC=`Magick++-config --cxxflags --cppflags`

mandel: mandel.cpp
	@$(CC) $(CFLAGS) $(MAGICKINC) mandel.cpp -o mandel $(LDFLAGS) $(MAGICKLIB)
	@strip mandel

mandel.o: mandel.cpp Thread.h Time.h
	@$(CC) $(CFLAGS) $(MAGICKINC) -c mandel.cpp

clean:
	@rm -f mandel *.o