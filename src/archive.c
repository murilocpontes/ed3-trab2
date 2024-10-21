#include "archive.h"

void get_file_data(FILE *file, data *file_data){
    // After skiping removed and linking variables, reads file info
    fread(&file_data->population, sizeof(int), 1, file);
    fread(&file_data->size, sizeof(float), 1, file);
    fread(&file_data->speed_unit, sizeof(char), 1, file);
    fread(&file_data->velocity, sizeof(int), 1, file);
    get_file_string(file, &file_data->name);
    get_file_string(file, &file_data->specie);
    get_file_string(file, &file_data->habitat);
    get_file_string(file, &file_data->type);
    get_file_string(file, &file_data->diet);
    get_file_string(file, &file_data->food);
}

void get_file_string(FILE *file, char** dest){
    int size = 1;
    char tmp;
    tmp = fgetc(file);
    // Reads until separator or file end to find end of string
    while (tmp != '#' && !eof_verification(file)){
        tmp = fgetc(file);
        size++;
    }
    // Back to string beginning and actually reading it, in allocated dest string, setting '\0' instead of ''#'
    fseek(file, -size, SEEK_CUR);
    *dest = (char*)malloc(size);
    fread(*dest, 1, size, file);
    (*dest)[size - 1] = '\0';
}

void print_file_data(data *file_data){
    // Printing all collected info (population if needed)
    printf("Nome: %s\n", file_data->name);
    printf("Especie: %s\n", file_data->specie);
    if(strcmp(file_data->type, "") != 0)
        printf("Tipo: %s\n", file_data->type);
    printf("Dieta: %s\n", file_data->diet);
    if(strcmp(file_data->habitat, "") != 0)
    printf("Lugar que habitava: %s\n", file_data->habitat);
    // if (file_data->population != -1)
    //     printf("População: %d\n", file_data->population);
    if (file_data->size != -1)
        printf("Tamanho: %.1f m\n", file_data->size);
    if (file_data->velocity != -1 && file_data->speed_unit != '$')
        printf("Velocidade: %d %cm/h\n", file_data->velocity,
               file_data->speed_unit);
    printf("\n");
}

void write_registry(FILE* file, data* registry){
    int total_size = ftell(file); 
    registry->removed = '0';
    registry->linkingRRN = -1;
    char delim = '#';
    char trash[REGISTRY_OFFSET];
    // writing registry data, and updating total_size with written strings size
    fwrite(&registry->removed, 1, 1, file);
    fwrite(&registry->linkingRRN, sizeof(int),1 , file);
    fwrite(&registry->population, sizeof(int), 1, file);
    fwrite(&registry->size, sizeof(float), 1, file);
    fwrite(&registry->speed_unit, sizeof(char), 1, file);
    fwrite(&registry->velocity, sizeof(int), 1, file);
    fwrite(registry->name, sizeof(char), strlen(registry->name), file);
    fwrite(&delim, sizeof(char), 1, file);
    fwrite(registry->specie, sizeof(char), strlen(registry->specie), file);
    fwrite(&delim, sizeof(char), 1, file);
    fwrite(registry->habitat, sizeof(char), strlen(registry->habitat), file);
    fwrite(&delim, sizeof(char), 1, file);
    fwrite(registry->type, sizeof(char), strlen(registry->type), file);
    fwrite(&delim, sizeof(char), 1, file);
    fwrite(registry->diet, sizeof(char), strlen(registry->diet), file);
    fwrite(&delim, sizeof(char), 1, file);
    fwrite(registry->food, sizeof(char), strlen(registry->food), file);
    fwrite(&delim, sizeof(char), 1, file);
    // setting trash string with remaining registry size and writing
    total_size = ftell(file) - total_size;
    memset(trash, '$', REGISTRY_OFFSET - total_size);
    trash[REGISTRY_OFFSET-total_size] = '\0';
    fwrite(trash, sizeof(char), REGISTRY_OFFSET - total_size, file);
}

void write_header(FILE *file, header *hea){  // function to write header record to binary file
    // setting trash string
    char trash[DISK_SIZE - HEADER_OFFSET + 1];
    memset(trash, '$', DISK_SIZE - HEADER_OFFSET);
    trash[DISK_SIZE - HEADER_OFFSET] = '\0';

    fseek(file, 0, SEEK_SET); // sets the 'file' pointer to position '0' from the beginning of the 'SEEK_SET' file.

    // writing header on new_file 
    fwrite(&hea->status, sizeof(char), 1, file);
    fwrite(&hea->top, sizeof(int), 1, file);
    fwrite(&hea->nextRRN, sizeof(int), 1, file);
    fwrite(&hea->remRegNum, sizeof(int), 1, file);
    fwrite(&hea->diskPageNum, sizeof(int), 1, file);
    fwrite(&hea->compQtt, sizeof(int), 1, file);
    fwrite(&trash, sizeof(char), DISK_SIZE - HEADER_OFFSET, file);
}

void read_header(FILE *file, header *hea, char *trash){  // function to read the header record of the binary file
    fseek(file,0,SEEK_SET);           //  sets the 'file' pointer to position '0' from the beginning of the 'SEEK_SET' file.

    fread(&hea->status, sizeof(char), 1, file);
    fread(&hea->top, sizeof(int), 1, file);
    fread(&hea->nextRRN, sizeof(int), 1, file);
    fread(&hea->remRegNum, sizeof(int), 1, file);
    fread(&hea->diskPageNum, sizeof(int), 1, file);
    fread(&hea->compQtt, sizeof(int), 1, file);

    for(int i = 0; i < 1579; i++){
         fread(&trash[i], sizeof(char), 1, file);   // trash reading
    }
}

int define_status(FILE *file, char character){
    int n = ftell(file);                                      // save file location
    if(fseek(file,0,SEEK_SET) != 0) return (-1);              // go to the 'status' location
    if(fwrite(&character,sizeof(char),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_top(FILE *file, int top){
    int n = ftell(file);                                      // save file location
    if(fseek(file,1,SEEK_SET) != 0) return (-1);              // goes to the 'top' location
    if(fwrite(&top,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_nextRRN(FILE *file, int next){
    int n = ftell(file);                                      // save file location
    if(fseek(file,5,SEEK_SET) != 0) return (-1);              // goes to the location of 'nextRRN'
    if(fwrite(&next,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_remRegNum(FILE *file, int removed){
    int n = ftell(file);                                      // save file location
    if(fseek(file,9,SEEK_SET) != 0) return (-1);              // goes to the location of 'remRegNum'
    if(fwrite(&removed,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_diskPageNum(FILE *file, int pages){
    int n = ftell(file);                                      // save file location
    if(fseek(file,13,SEEK_SET) != 0) return (-1);              // goes to the location of 'diskPageNum'
    if(fwrite(&pages,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_compQtt(FILE *file, int quantity){
    int n = ftell(file);                                      // save file location
    if(fseek(file,17,SEEK_SET) != 0) return (-1);              // goes to the location of 'compQtt'
    if(fwrite(&quantity,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

void write_compressed_file_header(FILE* old_file, FILE* new_file){
    int compQtt;
    header compressed_header;
    // getting compQtt from old_file
    fseek(old_file, 1 + 4*sizeof(int), SEEK_CUR);
    fread(&compQtt, sizeof(int), 1, old_file);
    // setting new header fields 
    initialize_header(&compressed_header);
    compressed_header.compQtt = compQtt +1;
    // writing header on new_file 
    write_header(new_file, &compressed_header);
}

int eof_verification(FILE *file){
    // Verifies if fread can read a byte
    int byte;
    if (fread(&byte, 1, 1, file) == 1){
        fseek(file, -1, SEEK_CUR);
        return 0;
    } 
    // Else, test if is end of file
    return feof(file)? 1 : 0;
}

int verify_removed_register(FILE* file){
    // Reads "removed" registry data and returns if it is removed or not
    int flag = 0;
    char tmp[2];
    fgets(tmp, 2, file);
    flag = atoi(tmp);
    if(flag == 1)
        return 1;
    return 0;
}


void initialize_header(header *hea){ // function to initialize the header record
    hea->status = '0';
    hea->top = -1;
    hea->nextRRN = 0;
    hea->remRegNum = 0;
    hea->diskPageNum = 0;
    hea->compQtt = 0;
}

void initialize_data(data *reg){   // function to initialize the data record
    reg->name = (char *)malloc(30 * sizeof(char));
    reg->specie = (char *)malloc(30 * sizeof(char));
    reg->habitat = (char *)malloc(30 * sizeof(char));
    reg->type = (char *)malloc(30 * sizeof(char));
    reg->diet = (char *)malloc(30 * sizeof(char));
    reg->food = (char *)malloc(30 * sizeof(char));
}

void free_data(data *d){
    // Free the dynamic field strings
    free(d->name);
    free(d->specie);
    free(d->habitat);
    free(d->type);
    free(d->diet);
    free(d->food);
}