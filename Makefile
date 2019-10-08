CXXC = g++
CXXFLAGS = -Wall -O3
DEL = del

build: UnitTest

test: UnitTest11 UnitTest03

UnitTest: LifeAPI.o UnitTest.o
	$(CXXC) $(CXXFLAGS) $? -o $@

UnitTestC++11: LifeAPI.o UnitTest.o
	$(CXXC) $(CXXFLAGS) -std=c++11 $? -o $@

UnitTestC++03: LifeAPI.o UnitTest.o
	$(CXXC) $(CXXFLAGS) -std=c++03 $? -o $@

LifeAPI.o: LifeAPI.cpp LifeAPI.h
	$(CXXC) $(CXXFLAGS) -c $<

UnitTest.o: UnitTest.cpp LifeAPI.h
	$(CXXC) $(CXXFLAGS) -c $<

clean:
	$(DEL) *.o *.exe