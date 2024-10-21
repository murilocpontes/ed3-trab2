#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DISK_SIZE 1600
#define REGISTRY_OFFSET 160
#define HEADER_OFFSET 21 //5*int + 1

typedef struct{
    char removed;
    int linkingRRN;
    int population;
    float size;
    char speed_unit;
    int velocity;
    char* name;
    char* specie;
    char* habitat;
    char* type;
    char* diet;
    char* food;
 }data;

typedef struct{
    char status;
    int top;
    int nextRRN;
    int remRegNum;
    int diskPageNum;
    int compQtt;
    char* lixo;
}header;

/////////////// REGISTRY-RELATED FUNCTIONS ////////////////////////

// Functions to get file register info
void get_file_data(FILE* file, data* file_data);
void get_file_string(FILE* file, char** dest);

// Function to print a register data
void print_file_data(data* file_data);

// Function to write a register in a file
void write_registry(FILE* file, data* registry);

/////////////// HEADER-RELATED FUNCTIONS ////////////////////////

// Function to write the file header
void write_header(FILE *file, header *hea);

// Function to read header fields
void read_header(FILE *file, header *hea, char *trash);

// Functions to overwrite header field info
int define_status(FILE *file, char character);
int define_top(FILE *file, int top);
int define_nextRRN(FILE *file, int next);
int define_remRegNum(FILE *file, int removed);
int define_diskPageNum(FILE *file, int pages);
int define_compQtt(FILE *file, int quantity);

// Function to write compressed file header
void write_compressed_file_header(FILE* old_file, FILE* new_file);

/////////////// VERIFY FUNCTIONS ////////////////////////

// Funcion to verify EOF
int eof_verification(FILE* file);

// Function to verify if a register is logically removed
int verify_removed_register(FILE* file);

/////////////// INIT & FREE FUNCTIONS ////////////////////////

// Initialize struct fields (used when not reading from file)
void initialize_header(header *hea);
void initialize_data(data *reg);

// Function to free data pointers
void free_data(data* d);