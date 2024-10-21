APPS = ./apps
BIN = ./bin
INCLUDE = ./include
OBJ = ./obj
SRC = ./src

OBJS = $(OBJ)/funcoesFornecidas.o $(OBJ)/sql.o $(OBJ)/archive.o $(OBJ)/treeFile.o

CFLAGS = -g -I $(INCLUDE) 

all: $(BIN)/programaTrab

$(BIN)/programaTrab: $(OBJS) $(APPS)/main.c
	gcc $(CFLAGS) $(OBJS) $(APPS)/main.c -o $(BIN)/programaTrab

$(OBJ)/funcoesFornecidas.o: $(SRC)/funcoesFornecidas.c $(INCLUDE)/funcoesFornecidas.h
	gcc $(CFLAGS) -c $(SRC)/funcoesFornecidas.c -o $(OBJ)/funcoesFornecidas.o

$(OBJ)/archive.o: $(SRC)/archive.c $(INCLUDE)/archive.h
	gcc $(CFLAGS) -c $(SRC)/archive.c -o $(OBJ)/archive.o

$(OBJ)/sql.o: $(SRC)/sql.c $(INCLUDE)/sql.h
	gcc $(CFLAGS) -c $(SRC)/sql.c -o $(OBJ)/sql.o

$(OBJ)/treeFile.o: $(SRC)/treeFile.c $(INCLUDE)/treeFile.h
	gcc $(CFLAGS) -c $(SRC)/treeFile.c -o $(OBJ)/treeFile.o

run:
	$(BIN)/programaTrab

debug: $(BIN)/programaTrab
	gdb $(BIN)/programaTrab

clean:
	rm -f $(OBJ)/*.o
	rm -f $(BIN)/programaTrab


