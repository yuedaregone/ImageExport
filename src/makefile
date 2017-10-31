LFLAGS+=-lpng
LFLAGS+=-lz

PNG_INC= 
PNG_LIB= 
ZLIB_PATH = 

CORE_O= MImage.o export.o
EXECUTE= export

$(EXECUTE): export.o MImage.o
	g++ $(CORE_O) -o $(EXECUTE) -L$(PNG_LIB) $(LFLAGS)

clean:
	rm -f $(CORE_O) $(EXECUTE)

.cpp.o:
	$(CC) -I$(PNG_INC) -c -o $@ $<

MImage.o: MImage.cpp MImage.h	
export.o: export.cpp MImage.h

