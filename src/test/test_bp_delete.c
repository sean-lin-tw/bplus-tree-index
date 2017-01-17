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
    for(int j=0; j<200; j++) {
        inserted_entry.key.i = j;
        inserted_entry.pid = j;
        inserted_entry.slot_num = j;
        bp__insert(&root, root, &inserted_entry, &tmp_new_child, level, &level, TYPE_INT);
    }

    tree_entry_t* tmp_old_child = NULL;

		// Delete entries
    for(int j=0; j<199; j++) {
        index_t key;
				key.i = j;
        bp__delete(&root, root, key, &tmp_old_child, 0, level, &level, TYPE_INT);

		/* first-level*/
		int root_occupy = (level == 0)?root.leaf->occupy:root.branch->occupy;
		for (int k=0; k<root_occupy; k++) {
			if(level == 0)
				printf("%4d", root.leaf->dentry[k].key);
			else
				printf("%4d", root.branch->tentry[k].key);
		}
		printf("\n  Occupancy: %d\n", root.branch->occupy);
		puts("\n");

		if (level > 0) {
		for (int k=0; k<root.branch->first_ptr.leaf->occupy; k++) {
			printf("%4d", root.branch->first_ptr.leaf->dentry[k].key);
		}
		printf("\n  Occupancy: %d\n", root.branch->first_ptr.leaf->occupy);
		puts("\n");

		for (int k=0; k<root.branch->occupy; k++) {
			for(int l=0; l<root.branch->tentry[k].page_ptr.leaf->occupy; l++) {
				printf("%4d", root.branch->tentry[k].page_ptr.leaf->dentry[l].key);
			}
			printf("\n  Occupancy: %d\n", root.branch->tentry[k].page_ptr.leaf->occupy);
			puts("\n");
		}
		puts("\n");
		}
    }




}
