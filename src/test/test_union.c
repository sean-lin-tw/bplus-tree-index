#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/bptree.h"

tree_page_ptr_t bp_new_page(tree_page_t type)
{
    tree_page_ptr_t tp_ptr;
    if (type == TYPE_BRANCH)
        tp_ptr.branch = (branch_page_t *)calloc(1, sizeof(branch_page_t));
    else
        tp_ptr.leaf = (leaf_page_t *)calloc(1, sizeof(leaf_page_t));

    return tp_ptr;
}

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

    tree_page_ptr_t node1 = bp_new_page(TYPE_LEAF);
    node1.leaf->occupy = 5;
    printf("%d\n", node1.leaf->occupy);

    return 0;
}
