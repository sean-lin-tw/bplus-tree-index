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



relation_t* get_relation(relation_page_t* header, const char* relation_name)
{
    if(header == NULL)
        return NULL;

    relation_t* target;

    while(header!=NULL) {
        for(int i=0; i<RELATION_PAGE_SIZE; i++) {
            if(strcmp(header->relations[i].name, relation_name) == 0)
                return &(header->relations[i]);
        }
        header = header->next;
    }

    return NULL;
}


void relation_display_info(relation_t* relation)
{

    if(relation->page_header == NULL) {
        fprintf(stderr, "This relation is empty!\n");
        return;
    }

    printf("New relation name: %s\n", relation->name);
    printf("----> Key type: %s\n", relation->ktype == TYPE_INT ? "Integer" : "String");
    printf("----> Record length: %d\n", relation->record_length);
    printf("----> Tree level: %d\n", relation->level+1);
    printf("----> Root Address: %p, size: %lu\n",
           relation->root.leaf, sizeof(*(relation->root.leaf)));
    printf("----> Directory Address: %p, size: %lu\n\n",
           relation->page_header, sizeof(*(relation->page_header)));
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

void relation__index_scan(relation_t* relation)
{
    printf("\nTotal leaf pages: %d\n", bp__scan_leaf(relation->root, relation->level));
    printf("Total index pages: %d\n", bp__scan(relation->root, relation->level, relation->ktype, 0));
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
    if(found_record.pid!=UINT16_MAX && found_record.slot_num!=UINT16_MAX) {
        dpage__find_record(relation->page_header,
                           relation->record_length,
                           relation->ktype,
                           found_record.pid,
                           found_record.slot_num,
                           ACTION_PRINT);
    }
}


void relation__find_range(relation_t* relation, index_t key1, index_t key2)
{
    // Use bp__get to get the location of the first page
    tree_page_ptr_t found_leaf = bp__get(relation->root, key1, relation->level, relation->ktype);
    data_entry_t found_record;
    int entry_index = -1;

    for(int i=0; i<PAGE_ENTRY_SIZE; i++) {
        if(key__cmp(found_leaf.leaf->dentry[i].key, key1, relation->ktype)==0) {
            entry_index = i;
            break;
        }
    }

    // Iterate with the doubly-linked list
    if(entry_index == -1) {
        if(relation->ktype == TYPE_INT)
            fprintf(stderr, "Warning: Cannot find the record with key range: [%d, %d]>\n", key1.i, key2.i);
        else
            fprintf(stderr, "Warning: Cannot find the record with key range: [%s, %s]>\n", key1.str, key2.str);
        return;
    } else {
        while(found_leaf.leaf != NULL) {
            for(entry_index; entry_index<found_leaf.leaf->occupy; entry_index++) {
                found_record = found_leaf.leaf->dentry[entry_index];
                if(key__cmp(found_record.key, key1, relation->ktype)>=0 &&
                        key__cmp(found_record.key, key2, relation->ktype)<=0) {
                    // Fetch the remained_record from data page
                    // _print_data_entry(found_record, relation->ktype);
                    dpage__find_record(relation->page_header,
                                       relation->record_length,
                                       relation->ktype,
                                       found_record.pid,
                                       found_record.slot_num,
                                       ACTION_PRINT);
                } else {
                    return;
                }
            }
            found_leaf.leaf = found_leaf.leaf->next;
            entry_index = 0;
        }
    }
}


void relation__page_display(relation_t* relation, uint16_t pid)
{
    directory_page_t* cur_dirct = relation->page_header;
    int dirct_id = pid / DIRECTORY_ENTRY_NUM;
    int dirct_entry_id = pid % DIRECTORY_ENTRY_NUM;

    while(cur_dirct!=NULL && cur_dirct->pid_base != dirct_id*DIRECTORY_ENTRY_NUM)
        cur_dirct = cur_dirct->next;

    rpage__show_page(&(cur_dirct->entry[dirct_entry_id]),
                     relation->record_length,
                     relation->ktype);
}


void relation__statistic(relation_t* relation)
{
    printf("\nTotal index pages: %d\n", bp__scan(relation->root, relation->level, relation->ktype, 0));
    printf("Total slotted data page: %d\n", dpage__statistics(relation->page_header));
}
