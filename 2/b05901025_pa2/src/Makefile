EXE=gr

CC=g++
CPPFLAGS = -Wall -O2 $(OPTFLAGS) -std=c++11  


ROOT=$(PWD)
LIB_PATH=$(ROOT)/lib

OBJ_FILES= main.o parser.o routingdb.o tree.o router.o \
		  $(LIB_PATH)/mst.o


$(EXE): $(OBJ_FILES)
	$(CC) $(CPPFLAGS) -o $(EXE) $(OBJ_FILES)

#dependency generated by (g++ -MM *.cpp lib/*.cpp)
main.o: main.cpp parser.h routingdb.h tree.h
parser.o: parser.cpp parser.h routingdb.h
routingdb.o: routingdb.cpp routingdb.h
tree.o: tree.cpp lib/mst.h routingdb.h tree.h
router.o: routingdb.h router.h router.cpp
mst.o: lib/mst.cpp lib/mst.h

					     
clean:
	rm -f *.o lib/*.o $(EXE)
