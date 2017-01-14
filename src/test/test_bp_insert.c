#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/bptree.h"


int main(void)
{
    tree_page_ptr_t root;
    root.leaf = (leaf_page_t*) calloc(1, sizeof(leaf_page_t));
    int level = 0;

    data_entry_t inserted_entry;
    tree_entry_t* tmp_new_child = NULL;

    // Insert a lot of data-entry
    for(int j=0; j<800; j++) {
        inserted_entry.key.i = j;
        inserted_entry.pid = j;
        inserted_entry.slot_num = j;
        bp__insert(&root, root, &inserted_entry, &tmp_new_child, level, &level, TYPE_INT);
    }

    printf("\n\nTotal index pages: %d\n", bp__scan(root, level, TYPE_INT, 1));

    printf("\n\n\n");


    inserted_entry.key.i = 439;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 258;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 694;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 9527;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);


    return 0;
}
