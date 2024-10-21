#include "funcoesFornecidas.h"
#include "sql.h"

void read_csv_field(FILE *csv_file, int number, data *reg){  // function that reads 10 fields from the .csv, that is, one line

    int aux1 = -1;
    char aux2, campo[160];
    int len;

    for(int n = 0; n != 9; n++){
        aux1 = fscanf(csv_file,"%[^,]",campo);        // reads a field until it finds a comma
        if(aux1 <= 0)                              // if the read field is empty, since fscanf returns the number of items read and assigned successfully
            strcpy(campo,"");
        fscanf(csv_file,"%c", &aux2);               // ignore the separating comma

        len = strlen(campo);   // checks if the last character is '\r' and replaces it with '\0'
        if (len > 0 && campo[len - 1] == '\r') {
            campo[len - 1] = '\0';
        }

        if(!number){                              // if you are reading the first line of the csv, it does not save
            if(n == 0){                         // name - string - cannot assume null values
                strcpy(reg->name, campo);
            } else if(n == 1){                  // diet - string - cannot assume null values
                strcpy(reg->diet, campo);
            } else if(n == 2){                  // habitat - string - can take null values
                strcpy(reg->habitat, campo);
            } else if(n == 3){                  // population - int - can take null values
                if(!strcmp(campo,""))
                    reg->population = -1;     // if it is NULL
                else
                    reg->population = atoi(campo);  // atoi() converts a sequence of characters into an integer value

            } else if(n == 4){                  // type - string - can take null values
                strcpy(reg->type, campo);
            } else if(n == 5){                  // velocity - int - can take null values
                if(!strcmp(campo,""))
                    reg->velocity = -1;    // if it is NULL
                else
                    reg->velocity = atoi(campo);  // atoi() converts a sequence of characters into an integer value

            } else if(n == 6){                  // unitMeasurement - char - can assume null values
                if(!strcmp(campo,""))             // if it is NULL
                    reg->speed_unit = '$';
                else
                    reg->speed_unit = campo[0];
            } else if(n == 7){                  // size - float - can take null values
                if(!strcmp(campo,""))
                    reg->size = -1.0;    // if it is NULL
                else
                    reg->size = atof(campo);     //  atof() converts a sequence of characters to float

            } else if(n == 8){                  // specie - string - cannot assume null values
                strcpy(reg->specie, campo);
            }
        }
    }
    aux1 = fscanf(csv_file,"%[^\n]",campo);           // read the last field of the line that ends in '\n'
    if(aux1 <= 0)                                  // if read field is empty
        strcpy(campo,"");

    len = strlen(campo);       // checks if the last character is '\r' and replaces it with '\0'
    if (len > 0 && campo[len - 1] == '\r') {
        campo[len - 1] = '\0';
    }

    fscanf(csv_file,"%c", &aux2);                   // ignore the '\n' separator

    if(!number){                                  // if you are reading the first line of the csv, it does not save
        strcpy(reg->food, campo);          // food - string - can take null values
        reg->linkingRRN = -1;              // there is not yet an 'RRN' of a logically removed next data record
        reg->removed = '0';                 // the data record is not removed
    }
}


void read_csv(char **csv_filename, char **binary_filename){ // feature 1: reading multiple csv records

    FILE *csv_file = fopen(*csv_filename,"rt");
    if(!csv_file){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *binary_file = fopen(*binary_filename,"wb");
    if(!binary_file){
        printf("Falha no processamento do arquivo.\n");
        fclose(csv_file);
        return;
    }

    long unsigned int counter = 0;
    int pages = 0, next_rrn = 0, aux;

    header hea;
    initialize_header(&hea);

    data reg;
    initialize_data(&reg);

    write_header(binary_file,&hea);

    read_csv_field(csv_file,1,&reg);  // ignore csv header

    while(!eof_verification(csv_file)){
        read_csv_field(csv_file,0,&reg);
        write_registry(binary_file,&reg);
        counter += REGISTRY_OFFSET;
    }

    next_rrn = (counter/REGISTRY_OFFSET);
    counter += DISK_SIZE;

    aux = counter % DISK_SIZE;
    pages = ((aux == 0) ? (counter / DISK_SIZE) : ((int)(counter / DISK_SIZE) + 1));

    define_status(binary_file,'1');
    define_nextRRN(binary_file,next_rrn);
    define_diskPageNum(binary_file,pages);

    fclose(csv_file);
    fclose(binary_file);

    binarioNaTela(*binary_filename);
    free(*binary_filename);
    *binary_filename = NULL;

    free_data(&reg);
}

void print_info(char *address){
    FILE *file = fopen(address, "rb+");
    data file_data;
    char tmp[2];
    int disk_page_num, is_deleted, status, flag = 0, i = 0;

    if (file == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    // Validating file condition and setting '0' while reading file info
    fgets(tmp, 2, file);
    status = atoi(tmp);
    if (!status){
        printf("Falha no processamento do arquivo.\n");
		return;
	}
	fseek(file, - 1, SEEK_CUR);
    fputc('0', file);
    // Reading file disk page number
    fseek(file, 3 * sizeof(int), SEEK_CUR);
    fread(&disk_page_num, sizeof(int), 1, file);
    // Seeking to first register, getting file data and printing
    fseek(file, DISK_SIZE, SEEK_SET);
    while (!eof_verification(file)){ 
        if (verify_removed_register(file)){ // skip if is a removed register
            i++;
            fseek(file, REGISTRY_OFFSET - 1, SEEK_CUR);
            continue;
        }
        fseek(file, sizeof(int), SEEK_CUR);
        get_file_data(file, &file_data);
        print_file_data(&file_data);
        free_data(&file_data);
        i++;
        fseek(file, DISK_SIZE + i*REGISTRY_OFFSET, SEEK_SET); // fseek to next register
        flag++;
    }
    // Restore file condition and closing file
    fseek(file, 0, SEEK_SET);
    fputc('1', file);
    fclose(file);
    // flag == 0 means all registers were removed
    if (flag == 0){
        printf("Registro inexistente.\n");
        return;
    }

    printf("Numero de paginas de disco: %d\n\n", disk_page_num);
}

void search_records(char **filename, char *field, char *value){ // feature 3: search
    FILE *file;
    int rrn = 0;
    if(*filename == NULL || !(file = fopen(*filename,"rb"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    header hea;
    initialize_header(&hea);
    char *trash = (char *) malloc(1579 * sizeof(char));
    read_header(file,&hea,trash);

    data reg;

    int k = 0;
    int pages_accessed = 1;    // header is on the first page
    int records_per_page = DISK_SIZE / REGISTRY_OFFSET;   // 1600 / 160 = 10   -->> number of data records per disk page
    int record_counter = 0;

    while (!eof_verification(file)) {
        if(verify_removed_register(file)){
            fseek(file,REGISTRY_OFFSET - 1,SEEK_CUR);
            rrn++;
            continue;
        } 
        fseek(file,sizeof(int),SEEK_CUR);
        //read_record(file,&reg);
        get_file_data(file,&reg);

        record_counter++;
        if(record_counter > records_per_page){
            pages_accessed++;
            record_counter = 1;
        }

       
            int match = 0;    // matches

            if(strcmp(field, "nome") == 0 && strcmp(reg.name, value) == 0){
                match = 1;
            } else if(strcmp(field, "especie") == 0 && strcmp(reg.specie, value) == 0){
                match = 1;
            } else if(strcmp(field, "habitat") == 0 && strcmp(reg.habitat, value) == 0){
                match = 1;
            } else if(strcmp(field, "tipo") == 0 && strcmp(reg.type, value) == 0){
                match = 1;
            } else if(strcmp(field, "dieta") == 0 && strcmp(reg.diet, value) == 0){
                match = 1;
            } else if(strcmp(field, "alimento") == 0 && strcmp(reg.food, value) == 0){
                match = 1;
            } else if(strcmp(field, "populacao") == 0 && reg.population == atoi(value)){  // atoi() converts a sequence of characters into an integer value
                match = 1;
            } else if(strcmp(field, "tamanho") == 0 && reg.size == atof(value)){  // atof() converts a sequence of characters to float
                match = 1;
            } else if(strcmp(field, "velocidade") == 0 && reg.velocity == atoi(value)){
                match = 1;
            } else if(strcmp(field, "unidadeMedida") == 0 && strcmp(&reg.speed_unit, value) == 0){
                match = 1;
            } else if(strcmp(field, "encadeamento") == 0 && reg.linkingRRN == atoi(value)){
                match = 1;
            }
            if (match) {
                print_file_data(&reg);
                k = 1;
            }
        free_data(&reg);
        rrn++;
        fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }

    if(!k){
        printf("Registro inexistente.\n\n");
    }

    pages_accessed++;

    printf("Numero de paginas de disco: %d\n\n",pages_accessed);

    fclose(file);
    //free_data(&reg);
}

void search_and_delete_record(char* address, int n){
	FILE* file = fopen(address, "rb+");
	int status;
    char tmp[2];
    char data_type[20];
	if (file == NULL){
        printf("Falha no processamento do arquivo\n");
        return;
    }
    // Validating file status and updating while is open
    fgets(tmp, 2, file);
    status = atoi(tmp);
    if (!status){
        printf("Falha no processamento do arquivo\n");
		return;
	}
    fseek(file, - 1, SEEK_CUR);
    fputc('0', file);
    // setting how to remove registers in file based on user entry n times
	for(int i =0; i < n; i++){
		fseek(file, 1, SEEK_SET);
        scanf(" %s", data_type);
        if(strcmp(data_type, "populacao") == 0)
            remove_by_number(file, 1, 4);
        
        else if(strcmp(data_type, "tamanho") == 0)
            remove_by_number(file, 0, 8);
        
        else if(strcmp(data_type, "velocidade") == 0)
            remove_by_number(file, 1, 13);
        
        else if(strcmp(data_type, "nome") == 0)
            remove_by_string(file, 0);

        else if(strcmp(data_type, "especie") == 0)
            remove_by_string(file, 1);

        else if(strcmp(data_type, "habitat") == 0)
            remove_by_string(file, 2);

        else if(strcmp(data_type, "tipo") == 0)
            remove_by_string(file, 3);

        else if(strcmp(data_type, "dieta") == 0)
            remove_by_string(file, 4);

        else if(strcmp(data_type, "alimento") == 0)
            remove_by_string(file, 5);
	}
    fseek(file, 0, SEEK_SET);
    fputc('1', file);
    fclose(file);
}

// is_int defines if the number to be removed is interger or float
// offset is the number of bytes to be skipped to reach the desired field
void remove_by_number(FILE* file, int is_int, int offset){
    void* read_data;
    void* file_data;
    int top, nroRegRem, rrn = 0;
    // setting data type based on is_int
    size_t data_size = is_int ? sizeof(int) : sizeof(float);
    // setting trash string to overwrite removed register data
    char trash[REGISTRY_OFFSET-4];
    memset(trash, '$', REGISTRY_OFFSET-5);
    trash[REGISTRY_OFFSET-5] = '\0';
    // collecting value to delete matching registers
    if(is_int) {
        read_data = malloc(sizeof(int));
        scanf("%d", (int*)read_data);
    } else {
        read_data = malloc(sizeof(float));
        scanf("%f", (float*)read_data);
    }
    // setting file_data size equal to read_data size
    file_data = malloc(data_size);
    // getting top and number of removed register data from file
    fread(&top, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_CUR);
    fread(&nroRegRem, sizeof(int), 1, file);
    // until end of file, verify if file_data is equal to read_data
    while(!eof_verification(file)){
        fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
        if (verify_removed_register(file)){
            rrn++;
            fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
            continue;
        }

        fseek(file, offset, SEEK_CUR);
        fread(file_data, data_size, 1, file);
        // if int -> compares data as int : compares data as float
        if (is_int ? (*(int*)file_data == *(int*)read_data) : (*(float*)file_data == *(float*)read_data)) {
            // Returning to the register beggining, setting as removed and adding to the linked stack list
            fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
            fputc('1', file);
            fwrite(&top, sizeof(int), 1, file);
            int ret = fwrite(&trash, sizeof(char), REGISTRY_OFFSET - 5, file);
            fseek(file, 1, SEEK_SET);
            fwrite(&rrn, sizeof(int), 1, file);
            top = rrn;
            nroRegRem++;
            rrn++;
            fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
            continue;
        }
        rrn++;
        fseek(file,DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }
    // After removing all registers that match the read_data, update the removed register number and list top
    fseek(file, 1, SEEK_SET);
    fwrite(&top, sizeof(int), 1, file);
    fseek(file, 1 + 2*sizeof(int), SEEK_SET);
    fwrite(&nroRegRem, sizeof(int), 1, file);

    free(read_data);
    free(file_data);
}

// position is the number of the string in the sequence (name, specie, ...)
void remove_by_string(FILE* file, int position){
    char* file_data;
    char read_data[30];
    int nroRegRem, top, rrn = 0;
    // setting trash string to overwrite registry data
    char trash[REGISTRY_OFFSET-4];
    memset(trash, '$', REGISTRY_OFFSET-5);
    trash[REGISTRY_OFFSET-5] = '\0';
    // getting top of linked list and the number of deleted registers
    fread(&top, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_CUR);
    fread(&nroRegRem, sizeof(int), 1, file);
    // reading string to search match registers
    scan_quote_string(read_data);
    // until end of file, search match registers and delete them
    while(!eof_verification(file)){
        fseek(file, DISK_SIZE + rrn*REGISTRY_OFFSET, SEEK_SET);
        if(verify_removed_register(file)){
            rrn++;
            fseek(file, REGISTRY_OFFSET - 1, SEEK_CUR);
            continue;
        }
        // skip fixed size fields
        fseek(file, 17, SEEK_CUR);
        // skip field strings written before desired field
        for(int i = 0; i < position; i++){
            char tmp = fgetc(file);
            while(tmp != '#' && !eof_verification(file))
                tmp = fgetc(file);
        }
        get_file_string(file, &file_data);
        // if equal, set as removed, overwrite data and add to the linked list
        if(strcmp(file_data, read_data) == 0){
            fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
            free(file_data);
            fputc('1', file);
            fwrite(&top, sizeof(int), 1, file);
            fwrite(&trash, sizeof(char), REGISTRY_OFFSET - 5, file);
            fseek(file, 1, SEEK_SET);
            fwrite(&rrn, sizeof(int), 1, file);
            top = rrn;
            nroRegRem++;
            rrn++;
            // skip to next register
            fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
            continue;
        }        
        // skip to next register
        free(file_data);
        rrn++;
        fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }
    // after reaching eof, update number of deleted registers and list top
    fseek(file, 1, SEEK_SET);
    fwrite(&top, sizeof(int), 1, file);
    fseek(file, 1 + 2*sizeof(int), SEEK_SET);
    fwrite(&nroRegRem, sizeof(int), 1, file);
}

void insert_record(char **binaryfilename, int n){     // feature 5: insert new data records into a file
    FILE *binary_file;
    if(*binaryfilename == NULL || !(binary_file = fopen(*binaryfilename,"rb+"))){  // opens the file in binary read and write mode
        printf("Falha no processamento do arquivo.\n");
        free(*binaryfilename);
        *binaryfilename = NULL;
        return;
    }

    int next_rrn, pages;

    header hea;
    initialize_header(&hea); // initialize the header
    char *trash = (char *) malloc(1579 * sizeof(char));
    read_header(binary_file,&hea,trash);   // reads the file header

    data new_reg;
    initialize_data(&new_reg);                                           // initialize the new register


    char population_buffer[30];
    char velocity_buffer[30];
    char speed_unit_buffer[30];
    char size_buffer[30];

    for(int i = 0; i < n; i++){
        scan_quote_string(new_reg.name);

        scan_quote_string(new_reg.diet);


        scan_quote_string(new_reg.habitat);


        //scanf(" %s",population_buffer);
        scan_quote_string(population_buffer);
        if(strcmp(population_buffer, "") == 0){
            new_reg.population = -1;
        } else {
            new_reg.population = atoi(population_buffer);   // atoi() converts a sequence of characters into an integer value
        }

        scan_quote_string(new_reg.type);

        //scanf(" %s",velocity_buffer);
        scan_quote_string(velocity_buffer);
        if(strcmp(velocity_buffer, "") == 0){
            new_reg.velocity = -1;
        } else {
            new_reg.velocity = atoi(velocity_buffer);       // atoi() converts a sequence of characters into an integer value
        }

        //scanf(" %s",speed_unit_buffer);
        scan_quote_string(speed_unit_buffer);
        if(strcmp(speed_unit_buffer, "") == 0){
            new_reg.speed_unit = '$';
        } else {
            new_reg.speed_unit = speed_unit_buffer[0];
        }

        //scanf(" %s",size_buffer);
        scan_quote_string(size_buffer);
        if(strcmp(size_buffer, "") == 0){
            new_reg.size = -1;
        } else {
            new_reg.size = atof(size_buffer);       // atof() converts a sequence of characters to float
        }

        scan_quote_string(new_reg.specie);

        scan_quote_string(new_reg.food);

         // checks if the record already exists
        if (record_exists(binary_file, &new_reg, &hea)) {
            continue;                               // jump to the next record (next iteration)
        }


        // sizeof(data)
        // sizeof(header)
        if(hea.top == -1){        // hea.top == -1 : there are no records logically removed      ---->       insert at the end of the file
            fseek(binary_file, 0, SEEK_END); // sets the 'bynary_file' pointer to position '0' from the end of the file 'SEEK_END'.
            hea.nextRRN++;          // increment the next 'RRN'
            define_nextRRN(binary_file, hea.nextRRN);  // updates the next 'RRN' in the header
        } else {      // whether there are logically removed records
            fseek(binary_file, DISK_SIZE + hea.top * REGISTRY_OFFSET + 1, SEEK_SET); // position at the top of the stack  ---> adjusts the 'bynary_file' pointer to the position 'DISK_SIZE + hea.top * REGISTRY_OFFSET + 1' from the beginning of the file 'SEEK_SET'.
            fread(&hea.top, sizeof(int), 1, binary_file);
            fseek(binary_file, -(1+sizeof(int)), SEEK_CUR);
            hea.remRegNum--;
        }

        write_registry(binary_file, &new_reg);  // write the new register

        if(hea.nextRRN % 10 == 0){
            hea.diskPageNum++;
        }
        define_top(binary_file, hea.top);
        define_status(binary_file,'1');   // sets the 'status' of the file to consistent
        define_diskPageNum(binary_file, hea.diskPageNum);   // updates the number of disk pages
        define_remRegNum(binary_file, hea.remRegNum);
    }

    free_data(&new_reg);   // deallocates memory for dynamically allocated strings
    fclose(binary_file);
    binarioNaTela(*binaryfilename);
    free(*binaryfilename);
    *binaryfilename = NULL;
}

int compare_records(data *reg1, data *reg2){    // compares all fields in the record to check if they are the same
    return strcmp(reg1->name, reg2->name) == 0 &&
           strcmp(reg1->specie, reg2->specie) == 0 &&
           strcmp(reg1->habitat, reg2->habitat) == 0 &&
           strcmp(reg1->type, reg2->type) == 0 &&
           strcmp(reg1->diet, reg2->diet) == 0 &&
           strcmp(reg1->food, reg2->food) == 0 &&
           reg1->population == reg2->population &&
           reg1->size == reg2->size &&
           reg1->velocity == reg2->velocity &&
           reg1->speed_unit == reg2->speed_unit;
}

int record_exists(FILE *file, data *new_reg, header *hea){    // check if the record you want to insert already exists
    data existing_reg;
    int rrn = 0;

    fseek(file, DISK_SIZE, SEEK_SET); // goes to the beginning of data records
    while(!eof_verification(file)){
        if(verify_removed_register(file)){
            rrn++;
            fseek(file, DISK_SIZE + rrn*REGISTRY_OFFSET, SEEK_SET);
            continue;
        }
        fseek(file, sizeof(int), SEEK_CUR);
        get_file_data(file, &existing_reg);
        if(compare_records(new_reg, &existing_reg)){
            free_data(&existing_reg);
            return 1;
        }
        rrn++;
        free_data(&existing_reg);
        fseek(file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }
    return 0; // No duplicate records found
}

void compress_file(char* address){
    FILE* new_file = fopen("tmp", "wb");
    FILE* old_file = fopen(address, "rb");
    char tmp[2];
    int status, is_deleted, rrn = 0, qttPagDisk = 0, remRegNum = 0;
    data registry;
    
    if(old_file == NULL){
        printf("Falha no processamento do arquivo\n");
        fclose(new_file);
        return;
    }
    // Verifies file status and set to 0 while open
    fgets(tmp, 2, old_file);
    status = atoi(tmp);

    if(!status){
        printf("Falha no processamento do arquivo\n");
        fclose(new_file);
        fclose(old_file);
        return;
    }

    fseek(old_file, - 1, SEEK_CUR);
    fputc('0', old_file);
    // writing compressed_file_header
    write_compressed_file_header(old_file, new_file);
    // skipping to first register
    fseek(old_file, DISK_SIZE, SEEK_SET);
    // until end of file, write register on new file if not removed
    while(!eof_verification(old_file)){
        if(verify_removed_register(old_file)){
            fseek(old_file, REGISTRY_OFFSET - 1, SEEK_CUR);  
            remRegNum++;
        }
        else{
            fseek(old_file, sizeof(int), SEEK_CUR);
            get_file_data(old_file, &registry);
            write_registry(new_file, &registry);
            free_data(&registry);
        }
        rrn++;
        fseek(old_file,DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }
    // calculating new quantity of disks occupied
    if((rrn - remRegNum) % 10 == 0)
        qttPagDisk = (rrn - remRegNum)/10;
    else
        qttPagDisk = ((rrn - remRegNum)/10) + 1;
    // calculating next_rrn
    rrn = rrn - remRegNum + 1;
    // setting new values on file header
    define_nextRRN(new_file, rrn);   
    define_diskPageNum(new_file, qttPagDisk);
    define_status(new_file, '1');

    fclose(old_file);
    fclose(new_file);
    // after closing, remove old file and rename new file
    remove(address);
    rename("tmp", address);
}
