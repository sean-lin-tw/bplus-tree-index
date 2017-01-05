#include "directory.h"
#include "bptree.h"

#define RELATION_NAME_SIZE 107
#define RELATION_PAGE_SIZE 4

enum key_type {
  TYPE_INT,
  TYPE_STRING
};

typedef struct relation_s relation_t;
typedef struct relation_page_s relation_page_t;


struct relation_s {
  enum key_type ktype;
  int key_length;
  int record_length;

  tree_page_ptr_t root;
  directory_page_t* page_header;

  char name[RELATION_NAME_SIZE];

} __attribute__((packed));;


struct relation_page_s {
  relation_page_t* next;
  relation_t relations[RELATION_PAGE_SIZE];
};
// __attribute__((aligned(512)));;
