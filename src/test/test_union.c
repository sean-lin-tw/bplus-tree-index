#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/bptree.h"

int main(void)
{
	tree_page_ptr_t node;
	leaf_page_t* leaf = (leaf_page_t *)malloc(sizeof(leaf_page_t));
	leaf->occupy = 10;
  branch_page_t* branch = (branch_page_t *)malloc(sizeof(branch_page_t));
	branch->occupy = 20;

	node.leaf = leaf;
	printf("%d, %d\n", node.leaf->occupy, node.branch->occupy);
	node.branch = branch;
	printf("%d\n", node.branch->occupy);
	
	return 0;
}
