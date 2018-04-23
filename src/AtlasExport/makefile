LFLAGS+=-lpng
LFLAGS+=-lz

VPATH= ./:../

CC= g++
PNG_INC= /d/msys64/mingw64/include
PNG_LIB= /d/msys64/mingw64/lib
ZLIB_PATH = 

CORE_O= MImage.o ImageListParse.o
EXECUTE= ImageListParse

$(EXECUTE): MImage.o ImageListParse.o
	g++ $(CORE_O) -o $(EXECUTE) -L$(PNG_LIB) $(LFLAGS)

clean:
	rm -f $(CORE_O) $(EXECUTE)

.cpp.o:
	$(CC) -I$(PNG_INC) -c -o $@ $<

MImage.o: MImage.cpp MImage.h	
ImageListParse.o: ImageListParse.cpp MImage.h

