#include "directory.h"
#include "bptree.h"

#define RELATION_NAME_SIZE 107
#define RELATION_PAGE_SIZE 4

#define TYPE_INT 0
#define TYPE_STRING 1

typedef struct relation_s relation_t;
typedef struct relation_page_s relation_page_t;


struct relation_s {
  char name[RELATION_NAME_SIZE];
  uint8_t key_type; // key_type = TYPE_INT or TYPE_STRING
  int key_length;
  int record_length;

  tree_page_ptr_t root;
  directory_page_t* page_header;
} __attribute__((packed));;


struct relation_page_s {
  relation_page_t* next;
  relation_t relations[RELATION_PAGE_SIZE];
};
// __attribute__((aligned(512)));;
