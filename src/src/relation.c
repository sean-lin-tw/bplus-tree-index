#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/relation.h"

relation_t* relation__create(relation_page_t* header,
                             const char* relation_name,
                             bp_key_t key_type,
                             int rec_len)
{
    relation_t* new_relation = NULL;

    // Find an empty relation space
    for (int i=0; i<RELATION_PAGE_SIZE; i++) {
        if(header->relations[i].root.leaf == NULL) {
            new_relation = &(header->relations[i]);
            break;
        }
    }

    if(new_relation == NULL) {
        fprintf(stderr, "This page is full!");
        return NULL;
    }

    new_relation->ktype = key_type;
    new_relation->key_length = (key_type==TYPE_INT ? 4 : 10);
    new_relation->record_length = rec_len;

    new_relation->root.leaf = (leaf_page_t*) calloc(1, sizeof(leaf_page_t));
    new_relation->page_header = (directory_page_t*) calloc(1, sizeof(directory_page_t));

    strncpy(new_relation->name, relation_name, RELATION_NAME_SIZE);

    return new_relation;
}



void relation_display_info(relation_t* relation)
{

    if(relation->page_header == NULL) {
        fprintf(stderr, "This relation is empty!\n");
        return;
    }

    printf("New relation name: %s\n", relation->name);
    printf("----> Key type: %s\n",
           relation->ktype == TYPE_INT ? "Integer" : "String");
    printf("----> Record length: %d\n", relation->record_length);
    printf("----> Root Address: %p, size: %lu\n",
           relation->root.leaf,
           sizeof(*(relation->root.leaf)));
    printf("----> Directory Address: %p, size: %lu\n\n",
           relation->page_header,
           sizeof(*(relation->page_header)));
}



data_entry_t relation__insert(relation_t* relation,
                              index_t key,
                              const char* remained_record)
{
    tree_entry_t* tmp_new_child = NULL;
    // Insert the record to pages and get RID
    data_entry_t inserted_entry =  dpage__insert_record(relation->page_header,
                                   relation->record_length,
                                   relation->ktype,
                                   key,
                                   remained_record);
    // Then insert the RID and key to B+Tree
    bp__insert(&(relation->root), relation->root, &inserted_entry,
               &tmp_new_child, relation->level, &(relation->level), relation->ktype);
}


void relation__find(relation_t* relation, index_t key)
{
    if(relation->page_header == NULL) {
        fprintf(stderr, "This relation is empty!\n");
        return;
    }

    // Fetch the pid and slot# from B+tree
    data_entry_t found_record = bp__find_record(relation->root, key, relation->level, relation->ktype);

    // Fetch the remained_record from data page
    dpage__find_record(relation->page_header,
                       relation->key_length,
                       relation->ktype,
                       found_record.pid,
                       found_record.slot_num,
                       ACTION_PRINT);
}
