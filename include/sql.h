#include "archive.h"

/////////////// CREATE TABLE (1) ////////////////////////

void read_csv_field(FILE *csv_file, int number, data *reg);
void read_csv(char **csv_filename, char **binary_filename);

/////////////// SELECT FROM (2) ////////////////////////

// Function to print all registers on a file
void print_info(char* address);

/////////////// SELECT WHERE (3) ////////////////////////

void search_records(char **filename, char *field, char *value);

/////////////// LOGICALLY REMOVE (4) ////////////////////////

// Funcion to search and delete registers based on field input data
void search_and_delete_record(char* address, int n);

// Function to remove a register by a number input
void remove_by_number(FILE* file, int is_int, int offset);

// Function to remove a register by a string input
void remove_by_string(FILE* file, int position);

/////////////// INSERT INTO (5) ////////////////////////

void insert_record(char **binaryfilename, int n);
int compare_records(data *reg1, data *reg2);
int record_exists(FILE *file, data *new_reg, header *hea);

/////////////// COMPACT (6) ////////////////////////

// Function to compress a file
void compress_file(char* address);