#define DISK_SIZE 1600
#define REGISTRY_OFFSET 160
#define HEADER_OFFSET 21 //5*int + 1

#define ORDER 5  // ordem (m) da árvore B
#define DISK_SIZE_2 93
#define REGISTRY_OFFSET_2 93
#define HEADER_OFFSET_2 9 //2*int + 1

#define MAX 4 // Ordem - 1
#define MIN 2

typedef struct {
    char status;    
    int root_node;
    int next_node_RRN;
    char* trash;   // tem que ser 84 bytes para manter o tamanho fixo de 93 bytes
} tree_header;

typedef struct {
    char leaf; // folha
    int number_of_indexed_keys;
    int node_RRN;
    int p[ORDER];
    long c[ORDER - 1];
    int pr[ORDER - 1];
} b_tree_node_disk;

struct b_tree_node {
    int key_vector[MAX + 1], count;
    struct b_tree_node *pointer[MAX + 1];

    char leaf;
    int rrn;
    int pr[ORDER + 1];
};

//Passar as declarações das funções de árvore pra cá dps!!!!