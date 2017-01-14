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
    for(int j=0; j<1000; j++) {
        inserted_entry.key.i = j;
        inserted_entry.pid = j;
        inserted_entry.slot_num = j;
        // printf("Insert: %d\n", inserted_entry.key.i);
        bp__insert(&root, root, &inserted_entry, &tmp_new_child, level, &level, TYPE_INT);
    }

    /* Print out the tree with two level */
    printf("\nRoot level / Root page:\n");
    for(int j=0; j<35; j++) {
        printf("%3d ", root.branch->tentry[j].key.i);
    }

    printf("\n\nFirst level / branch page-0:\n");
    for(int j=0; j<35; j++) {
        printf("%3d ", root.branch->first_ptr.branch->tentry[j].key.i);
    }

    printf("\n\nFirst level / branch page-1:\n");
    for(int j=0; j<35; j++) {
        printf("%3d ", root.branch->tentry[0].page_ptr.branch->tentry[j].key.i);
    }

    printf("\n\nSecond level / leaf page-0:\n");
    for(int j=0; j<35; j++) {
        printf("%3d ", root.branch->first_ptr.branch->first_ptr.leaf->dentry[j].key.i);
    }

    printf("\n\nSecond level / leaf page-1:\n");
    for(int j=0; j<36; j++) {
        printf("%3d ", root.branch->first_ptr.branch->tentry[0].page_ptr.leaf->dentry[j].key.i);
    }

    printf("\n\n\n");

    inserted_entry.key.i = 439;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 258;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 694;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);

    // for(int j=0; j<35; j++) {
    //   if(root.branch->tentry[j].page_ptr.leaf!=NULL){
    //     printf("\n\nleaf page-%d:\n", j+1);
    //     for(int k=0; k<35; k++) {
    //       printf("%d ", root.branch->first_ptr.leaf->->dentry[k].key.i);
    //     }
    //   }
    // }


    return 0;
}
