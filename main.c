#include <stdio.h>
#include <stdlib.h>

#include "include/relation.h"

int main(){

  relation_page_t* test_relation_page = malloc(sizeof(relation_page_t));
  printf("Size of relation_page: %d\n", sizeof(*test_relation_page));

  leaf_page_t* test_leaf = malloc(sizeof(leaf_page_t));
  printf("Size of leaf: %d\n", sizeof(*test_leaf));

  branch_page_t* test_branch = malloc(sizeof(branch_page_t));
  printf("Size of branch: %d\n", sizeof(*test_branch));

  directory_page_t* test_directory = malloc(sizeof(directory_page_t));
  printf("Size of test_directory: %d\n", sizeof(*test_directory));

  record_page_t* test_record = malloc(sizeof(record_page_t));
  printf("Size of test_record: %d\n", sizeof(*test_record));

}
