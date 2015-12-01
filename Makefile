
DEBUG=-g

ArachneTest: ArachneTest.cc
	g++ $(DEBUG) -std=c++11 -o ArachneTest ArachneTest.cc -I../Arachne -L../Arachne -lArachne -lpthread

clean:
	rm -f ArachneTest
