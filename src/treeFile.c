#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "treeFile.h"
#include "archive.h"


struct b_tree_node *root; 

struct b_tree_node *create_node(int value, struct b_tree_node *son){
    struct b_tree_node *new_node;
    new_node = (struct b_tree_node *) malloc(sizeof(struct b_tree_node));

    new_node->key_vector[1] = value;
    new_node->count = 1;
    new_node->pointer[0] = root;
    new_node->pointer[1] = son;
    return new_node;
}

int define_root_node(FILE *file, int root_node){
    int n = ftell(file);                                      // save file location
    if(fseek(file,1,SEEK_SET) != 0) return (-1);              // goes to the 'root_node' location
    if(fwrite(&root_node,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

int define_next_node_RRN(FILE *file, int next_node_RRN){
    int n = ftell(file);                                      // save file location
    if(fseek(file,5,SEEK_SET) != 0) return (-1);              // goes to the location of 'next_node_RRN'
    if(fwrite(&next_node_RRN,sizeof(int),1,file) <= 0) return (-1);
    if(fseek(file,n,SEEK_SET) != 0) return (-1);              // returns to the initial file location
    return 0;
}

void insert_node(int value, int position, struct b_tree_node *node, struct b_tree_node *son){
    int i = node->count;

    while(i > position){
        node->key_vector[i + 1] = node->key_vector[i];
        node->pointer[i + 1] = node->pointer[i];
        i--;
    }

    node->key_vector[i + 1] = value;
    node->pointer[i + 1] = son;
    node->count++;
}

void split_node(int value, int *p_value, int position, struct b_tree_node *node, struct b_tree_node *son, struct b_tree_node **new_node){
    int i, median;

    if(position > MIN)
        median = MIN + 1;
    else
        median = MIN;

    *new_node = (struct b_tree_node *) malloc(sizeof(struct b_tree_node));
    i = median + 1;
    while(i <= MAX){
        (*new_node)->key_vector[i - median] = node->key_vector[i];
        (*new_node)->pointer[i - median] = node->pointer[i];
        i++;
    }

    node->count = median;
    (*new_node)->count = MAX - median;

    if(position <= MIN){
        insert_node(value, position, node, son);
    } else {
        insert_node(value, position - median, *new_node, son);
    }

    *p_value = node->key_vector[node->count];
    (*new_node)->pointer[0] = node->pointer[node->count];
    node->count--;
}

int set_value(int value, int *p_value, struct b_tree_node *node, struct b_tree_node **son){
    int position;

    if(!node){
        *p_value = value;
        *son = NULL;
        return 1;
    }

    if(value < node->key_vector[1]){
        position = 0;
    } else {
        for(position = node->count; (value < node->key_vector[position] && position > 1); position--);

        if(value == node->key_vector[position]){
            // printf("Duplicatas não são permitidas.\n");
            return 0;
        }
    }

    if(set_value(value, p_value, node->pointer[position], son)){

        if (node->count < MAX) {
            insert_node(*p_value, position, node, *son);
        } else {
            split_node(*p_value, p_value, position, node, *son, son);
            return 1;
        }
    }

    return 0;
}

void insert(int value){
    struct b_tree_node *son;
    int i, flag;    

    flag = set_value(value, &i, root, &son);

    if(flag){
        root = create_node(i, son);
    }   
}

void search(int value, int *position, struct b_tree_node *my_node){
    if(!my_node){
        return;
    }

    if(value < my_node->key_vector[1]){
        *position = 0;
    } else {
        for(*position = my_node->count; (value < my_node->key_vector[*position] && *position > 1); (*position)--);

        if(value == my_node->key_vector[*position]){
            printf("%d foi encontrado.\n", value);
            return;
        }
    }

    search(value, position, my_node->pointer[*position]);
}

void write_to_tree_file(FILE *file, b_tree_node_disk *node, int rrn){
    fseek(file, rrn * REGISTRY_OFFSET + DISK_SIZE, SEEK_SET);
    fwrite(node, REGISTRY_OFFSET, 1, file);
}

void update_tree_file(FILE *file, struct b_tree_node *my_node, int *next_node_RRN){
    if(!my_node){
        return;
    } 

    b_tree_node_disk node_disk;
    node_disk.leaf = my_node->leaf;
    node_disk.number_of_indexed_keys = my_node->count;
    node_disk.node_RRN = (*next_node_RRN)++;

    for(int i = 0; i < ORDER - 1; i++){
        if(i < my_node->count){
            node_disk.c[i] = my_node->key_vector[i + 1];
            node_disk.pr[i] = my_node->pr[i + 1];
        } else {
            node_disk.c[i] = -1;
            node_disk.pr[i] = -1;
        }
    }

    for(int i = 0; i < ORDER; i++){
        if(i <= my_node->count){
            node_disk.p[i] = my_node->pointer[i] ? my_node->pointer[i]->rrn : -1;
        } else {
            node_disk.p[i] = -1;
        }
    }

    write_to_tree_file(file, &node_disk, node_disk.node_RRN);

    for(int i = 0; i <= my_node->count; i++){
        update_tree_file(file, my_node->pointer[i], next_node_RRN);
    }
}

void traversal(struct b_tree_node *my_node){
    int i;

    if(my_node){
        for(i = 0; i < my_node->count; i++){
            traversal(my_node->pointer[i]);
            printf("%d ", my_node->key_vector[i + 1]);
        }
        traversal(my_node->pointer[i]);
    }
}

void write_tree_header(FILE *file, tree_header *hea){  // function to write the tree header record to binary file
    // setting trash string
    char trash[DISK_SIZE_2 - HEADER_OFFSET_2 + 1];
    memset(trash, '$', DISK_SIZE_2 - HEADER_OFFSET_2);
    trash[DISK_SIZE_2 - HEADER_OFFSET_2] = '\0';

    fseek(file, 0, SEEK_SET); // sets the 'file' pointer to position '0' from the beginning of the 'SEEK_SET' file.

    // writing header on new_file 
    fwrite(&hea->status, sizeof(char), 1, file);
    fwrite(&hea->root_node, sizeof(int), 1, file);
    fwrite(&hea->next_node_RRN, sizeof(int), 1, file);

    fwrite(&trash, sizeof(char), DISK_SIZE_2 - HEADER_OFFSET_2, file);
}

void create_index(char **data_filename, char **index_filename){
    FILE *data_file;
    if(*data_filename == NULL || !(data_file = fopen(*data_filename,"rb"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        free(*data_filename);
        *data_filename = NULL;
        return;
    }
    
    FILE *index_file;
    if(*index_filename == NULL || !(index_file = fopen(*index_filename,"wb"))){  // open the file in binary write mode
        printf("Falha no processamento do arquivo.\n");
        free(*index_filename);
        *index_filename = NULL;
        return;
    }

    int rrn = 0;

    header hea;
    tree_header hea2;
    initialize_header(&hea);
    char *trash = (char *) malloc(1579 * sizeof(char));
    read_header(data_file,&hea,trash);

    write_tree_header(index_file,&hea);
    
    data reg;
    long key = 0;

    while(!eof_verification(data_file)){
        if(verify_removed_register(data_file)){
            fseek(data_file,REGISTRY_OFFSET - 1,SEEK_CUR);
            rrn++;
            continue;
        } 

        fseek(data_file,sizeof(int),SEEK_CUR);
        get_file_data(data_file,&reg);

        key = converteNome(reg.name);
        if(key != -1){
            insert(key);  
        }
        
        free_data(&reg);   // deallocates memory for dynamically allocated strings
        rrn++;
        fseek(data_file, DISK_SIZE + rrn * REGISTRY_OFFSET, SEEK_SET);
    }

    update_tree_file(index_file, root, &hea2.next_node_RRN);

    define_status(index_file,'1');
    define_next_node_RRN(index_file,rrn);
 
    fclose(data_file);
    fclose(index_file);
    binarioNaTela(*index_filename);
    free(*index_filename);
    *index_filename = NULL; 
}

void search_records_2(char **data_filename, char **index_filename, char *field, char *value){
    FILE *data_file;
    if(*data_filename == NULL || !(data_file = fopen(*data_filename,"rb"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        free(*data_filename);
        *data_filename = NULL;
        return;
    }
    
    FILE *index_file;
    if(*index_filename == NULL || !(index_file = fopen(*index_filename,"rb"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        free(*index_filename);
        *index_filename = NULL;
        return;
    }
    
    header hea;
    initialize_header(&hea);
    char *trash = (char *) malloc(84 * sizeof(char));
    read_header(data_file,&hea,trash);

    if(hea.status != '1'){
        printf("Falha no processamento do arquivo.\n");
        fclose(data_file);
        fclose(index_file);
        free(*index_filename);
        *index_filename = NULL;
        return;
    }

    long key = converteNome(value);
    int position;

    search(key, &position, root);

    fclose(data_file);
    fclose(index_file);
    free(*index_filename);
    *index_filename = NULL;
}

void insert_record_2(char **data_filename, char **index_filename, int n){
    FILE *data_file;
    if(*data_filename == NULL || !(data_file = fopen(*data_filename,"rb+"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        free(*data_filename);
        *data_filename = NULL;
        return;
    }
    
    FILE *index_file;
    if(*index_filename == NULL || !(index_file = fopen(*index_filename,"rb+"))){  // open the file in binary read mode
        printf("Falha no processamento do arquivo.\n");
        free(*index_filename);
        *index_filename = NULL;
        return;
    }

    header hea;
    initialize_header(&hea);
    char *trash = (char *) malloc(84 * sizeof(char));
    read_header(index_file,&hea,trash);

    data new_reg;
    initialize_data(&new_reg); 

    if(hea.status != '1'){
        printf("Falha no processamento do arquivo.\n");
        fclose(data_file);
        fclose(index_file);
        free(*index_filename);
        *index_filename = NULL;
        return;
    }

    hea.status = '0';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&hea, sizeof(hea), 1, index_file);

    char population_buffer[30];
    char velocity_buffer[30];
    char speed_unit_buffer[30];
    char size_buffer[30];

    for(int i = 0; i < n; i++){
        scan_quote_string(new_reg.name);

        scan_quote_string(new_reg.diet);

        scan_quote_string(new_reg.habitat);

        scan_quote_string(population_buffer);
        if(strcmp(population_buffer, "") == 0){
            new_reg.population = -1;
        } else {
            new_reg.population = atoi(population_buffer);
        }

        scan_quote_string(new_reg.type);

        scan_quote_string(velocity_buffer);
        if(strcmp(velocity_buffer, "") == 0){
            new_reg.velocity = -1;
        } else {
            new_reg.velocity = atoi(velocity_buffer);
        }

        scan_quote_string(speed_unit_buffer);
        if(strcmp(speed_unit_buffer, "") == 0){
            new_reg.speed_unit = '$';
        } else {
            new_reg.speed_unit = speed_unit_buffer[0];
        }

        scan_quote_string(size_buffer);
        if(strcmp(size_buffer, "") == 0){
            new_reg.size = -1;
        } else {
            new_reg.size = atof(size_buffer);
        }

        scan_quote_string(new_reg.specie);

        scan_quote_string(new_reg.food);

        fseek(data_file, 0, SEEK_END);
        //new_reg.rrn = ftell(data_file) / sizeof(data);
        fwrite(&new_reg, sizeof(data), 1, data_file);

        long key = converteNome(new_reg.name);
        insert(key);
    }

    hea.status = '1';
    fseek(index_file, 0, SEEK_SET);
    fwrite(&hea, sizeof(hea), 1, index_file);


    free_data(&new_reg);   // deallocates memory for dynamically allocated strings
    fclose(data_file);
    fclose(index_file);
    binarioNaTela(*index_filename);
    free(*index_filename);
    *index_filename = NULL; 
}