#ifndef RELATION_PAGE
#define RELATION_PAGE

#include "directory.h"

#define RELATION_NAME_SIZE 103
#define RELATION_PAGE_SIZE 4

typedef struct relation_s relation_t;
typedef struct relation_page_s relation_page_t;

relation_t* relation__create(relation_page_t* relation_page,
                             const char* relation_name,
                             bp_key_t key_type,
                             int rec_len);

void relation_display_info(relation_t* relation);


data_entry_t relation__insert(relation_t* relation,
                              index_t key,
                              const char* remained_record);

void relation__find(relation_t* relation, index_t key);



struct relation_s {
    bp_key_t ktype;
    int key_length;
    int record_length;

    tree_page_ptr_t root;
    int level;
    directory_page_t* page_header;

    char name[RELATION_NAME_SIZE];

} __attribute__((packed));;


struct relation_page_s {
    relation_page_t* next;
    relation_t relations[RELATION_PAGE_SIZE];
};
// __attribute__((aligned(512)));;

#endif
