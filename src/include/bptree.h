#ifndef BPTREE_PAGE
#define BPTREE_PAGE

#include <stdint.h>

#define PAGE_ENTRY_SIZE 35
#define STRING_KEY_SIZE 10

typedef struct leaf_page_s leaf_page_t;
typedef struct branch_page_s branch_page_t;
typedef struct tree_entry_s tree_entry_t;
typedef struct data_entry_s data_entry_t;
typedef union tree_page_ptr_u tree_page_ptr_t;
typedef union index_u index_t;


typedef enum key_type_e {
    TYPE_INT,
    TYPE_STRING
} bp_key_t;

typedef enum tree_page_type_e {
    TYPE_BRANCH,
    TYPE_LEAF
} tree_page_t;

union index_u {
    int i;
    char str[STRING_KEY_SIZE];
} __attribute__((packed));;


union tree_page_ptr_u {
    leaf_page_t* leaf;
    branch_page_t* branch;
};


int key__cmp (index_t key1, index_t key2, bp_key_t type);
tree_page_ptr_t bp__new_page(tree_page_t type);

void bp__sort(void* base, size_t nitems, size_t size, tree_page_t bpage_type, bp_key_t key_type);

void bp__insert(tree_page_ptr_t* root,
                tree_page_ptr_t node,
                data_entry_t* entry,
                tree_entry_t** new_child,
                int level,
                int* total_level,
                bp_key_t type);

tree_page_ptr_t bp__get(tree_page_ptr_t node, index_t key, int level, bp_key_t type);




struct data_entry_s {
    uint16_t pid;
    uint16_t slot_num;

    index_t key;
} __attribute__((packed));;


struct tree_entry_s {
    tree_page_ptr_t page_ptr;
    index_t key;
} __attribute__((packed));;


struct branch_page_s {
    uint8_t occupy;

    branch_page_t* uplevel;
    tree_entry_t tentry[PAGE_ENTRY_SIZE];
    tree_page_ptr_t first_ptr;
} __attribute__((aligned(512)));;


struct leaf_page_s {
    uint8_t occupy;

//  branch_page_t* uplevel;
    leaf_page_t* prev;
    leaf_page_t* next;

    data_entry_t dentry[PAGE_ENTRY_SIZE];
} __attribute__((aligned(512)));;

#endif
