#include <stdio.h>
#include <stdlib.h>

#include "../include/relation.h"

int main()
{

    printf("-----------------------------Size of pages-----------------------------\n");

    relation_page_t* test_relation_page = (relation_page_t*) malloc(sizeof(relation_page_t));
    printf("Size of relation_page: %d\n", sizeof(*test_relation_page));

    leaf_page_t* test_leaf = (leaf_page_t*) malloc(sizeof(leaf_page_t));
    printf("Size of leaf_page: %d\n", sizeof(*test_leaf));

    branch_page_t* test_branch = (branch_page_t*) malloc(sizeof(branch_page_t));
    printf("Size of branch_page: %d\n", sizeof(*test_branch));

    directory_page_t* test_directory = (directory_page_t*) malloc(sizeof(directory_page_t));
    printf("Size of directory_page: %d\n", sizeof(*test_directory));

    record_page_t* test_record = (record_page_t*) malloc(sizeof(record_page_t));
    printf("Size of record_page: %d\n", sizeof(*test_record));

    printf("\n-----------------------------Size of entries-----------------------------\n");

    slot_entry_t* test_slot_entry = (slot_entry_t*) malloc(sizeof(slot_entry_t));
    printf("Size of slot_entry: %d\n", sizeof(*test_slot_entry));

    data_entry_t* test_data_entry = (data_entry_t*) malloc(sizeof(data_entry_t));
    printf("Size of data_entry: %d\n", sizeof(*test_data_entry));

    tree_entry_t* test_tree_entry = (tree_entry_t*) malloc(sizeof(tree_entry_t));
    printf("Size of tree_entry: %d\n", sizeof(*test_tree_entry));

    record_page_entry_t* test_record_page_entry = (record_page_entry_t*) malloc(sizeof(record_page_entry_t));
    printf("Size of record_page_entry: %d\n", sizeof(*test_record_page_entry));

    printf("\n-----------------------------Size of others-----------------------------\n");

    index_t* test_index = (index_t*) malloc(sizeof(index_t));
    printf("Size of index: %d\n", sizeof(*test_index));

    tree_page_ptr_t* test_tree_page_ptr_t = (tree_page_ptr_t*) malloc(sizeof(tree_page_ptr_t));
    printf("Size of tree_page_ptr: %d\n", sizeof(*test_tree_page_ptr_t));

    relation_t* test_relation = (relation_t*) malloc(sizeof(relation_t));
    printf("Size of relation: %d\n", sizeof(*test_relation));

}
