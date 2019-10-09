CXXC = g++
CXXFLAGS = -Wall -O3
DEL = rm -f
OBJ = LifeAPI.o UnitTest.o

build: UnitTest
debug: UnitTestDebug
ansi: UnitTest11 UnitTest03

UnitTest: $(OBJ)
	$(CXXC) $(CXXFLAGS) $^ -o $@

UnitTestDebug: $(OBJ)
	$(CXXC) $(CXXFLAGS) -g3 $^ -o $@

UnitTest11: $(OBJ)
	$(CXXC) $(CXXFLAGS) -std=c++11 $^ -o $@

UnitTest03: $(OBJ)
	$(CXXC) $(CXXFLAGS) -std=c++03 $^ -o $@

*.o: *.cpp
	$(CXXC) $(CXXFLAGS) -c $?

clean:
	$(DEL) *.o *.exe UnitTest UnitTest03 UnitTest11 UnitTestDebug