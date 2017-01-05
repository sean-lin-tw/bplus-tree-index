#include <stdint.h>

#define LEAF_PAGE_ENTRY_SIZE 35
#define BRANCH_PAGE_ENTRY_SIZE 31
#define STRING_KEY_SIZE 10

typedef struct leaf_page_s leaf_page_t;
typedef struct branch_page_s branch_page_t;
typedef struct tree_entry_s tree_entry_t;
typedef struct data_entry_s data_entry_t;
typedef union tree_page_ptr_u tree_page_ptr_t;
typedef union index_u index_t;


union index_u {
  int i;
  char str[STRING_KEY_SIZE];
} __attribute__((packed));;


union tree_page_ptr_u {
  leaf_page_t* leaf;
  branch_page_t* branch;
};


struct data_entry_s {
  uint16_t pid;
  uint16_t slot_num;

  index_t key;
} __attribute__((packed));;


struct tree_entry_s {
  index_t key;
  tree_page_ptr_t page_ptr;
} __attribute__((packed));;


struct branch_page_s {
  uint8_t occupy;

  branch_page_t* uplevel;
  tree_entry_t tentry[BRANCH_PAGE_ENTRY_SIZE];
  tree_page_ptr_t last_ptr;
};
// __attribute__((aligned(512)));;


struct leaf_page_s {
  uint8_t occupy;

  branch_page_t* uplevel;
  leaf_page_t* prev;
  leaf_page_t* next;

  data_entry_t dentry[LEAF_PAGE_ENTRY_SIZE];
};
// __attribute__((aligned(512)));;
