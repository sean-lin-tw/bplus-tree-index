#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/bptree.h"

// Lambda Function !
#define lambda(l_ret_type, l_arguments, l_body)         \
  ({                                                    \
    l_ret_type l_anonymous_functions_name l_arguments   \
      l_body                                            \
    &l_anonymous_functions_name;                        \
  })


void bp__sort(void* base, size_t nitems, size_t size, tree_page_t bpage_type, bp_key_t key_type)
{
    if(bpage_type == TYPE_LEAF) {
        if(key_type == TYPE_INT) {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return ( ((data_entry_t*)a)->key.i - ((data_entry_t*)b)->key.i ); }));
        } else {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return strcmp(((data_entry_t*)a)->key.str, ((data_entry_t*)b)->key.str); }));
        }
    } else {
        if(key_type == TYPE_INT) {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return ( ((tree_entry_t*)a)->key.i - ((tree_entry_t*)b)->key.i ); }));
        } else {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return strcmp(((tree_entry_t*)a)->key.str, ((tree_entry_t*)b)->key.str); }));
        }
    }
}


int key__cmp (index_t key1, index_t key2, bp_key_t type)
{
    if (type == TYPE_INT)
        return key1.i - key2.i;
    else
        return strcmp(key1.str, key2.str);
}


tree_page_ptr_t bp__new_page(tree_page_t type)
{
    tree_page_ptr_t tp_ptr;
    if (type == TYPE_BRANCH)
        tp_ptr.branch = (branch_page_t *)calloc(1, sizeof(branch_page_t));
    else
        tp_ptr.leaf = (leaf_page_t *)calloc(1, sizeof(leaf_page_t));

    return tp_ptr;
}


void bp__insert(tree_page_ptr_t* root,
                tree_page_ptr_t node,
                data_entry_t* entry,
                tree_entry_t** new_child,
                int level,
                int* total_level,
                bp_key_t type)
{
    // Find the leaf page  and the slot for insert (top-down)
    // Then return if a page split happens at the leaf page (bottom-up)

    /* non-leaf */
    if (level > 0) {

        /* Find the child */
        if (key__cmp(node.branch->tentry[0].key, entry->key, type) > 0) {
            bp__insert(root, node.branch->first_ptr, entry, new_child, level-1, total_level, type);
        } else if (node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr.leaf!=NULL &&
                   key__cmp(node.branch->tentry[PAGE_ENTRY_SIZE-1].key, entry->key, type) <= 0) {
            bp__insert(root, node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr, entry, new_child, level-1, total_level, type);
        } else {
            for (int i=0; i<PAGE_ENTRY_SIZE-1; i++) {
                if (key__cmp(node.branch->tentry[i].key, entry->key, type) <= 0 &&
                        (key__cmp(node.branch->tentry[i+1].key, entry->key, type)) > 0 ||
                        node.branch->tentry[i+1].page_ptr.leaf==NULL) {
                    bp__insert(root, node.branch->tentry[i].page_ptr, entry, new_child, level-1, total_level, type);
                    break;
                }
            }
        }

        if (*new_child == NULL)
            return;
        /* We've splited the leaf page */
        else {
            /* we need to split this upper page as well  */
            if (node.branch->occupy == PAGE_ENTRY_SIZE) {
                tree_page_ptr_t new_branch = bp__new_page(TYPE_BRANCH);

                // copy to tmp_tentry and sort it
                tree_entry_t tmp_tentry[PAGE_ENTRY_SIZE+1];
                memcpy(tmp_tentry, node.branch->tentry, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                tmp_tentry[PAGE_ENTRY_SIZE] = **new_child;

                // sort entries
                bp__sort(tmp_tentry, PAGE_ENTRY_SIZE, sizeof(tree_entry_t), TYPE_BRANCH, type);

                // move entries
                memset(node.branch->tentry, 0, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                memcpy(node.branch->tentry, &tmp_tentry, sizeof(tree_entry_t)*(PAGE_ENTRY_SIZE+1)/2);
                node.branch->occupy = (PAGE_ENTRY_SIZE+1)/2;
                memcpy(new_branch.branch->tentry, &tmp_tentry[(PAGE_ENTRY_SIZE+1)/2 + 1], sizeof(tree_entry_t)*((PAGE_ENTRY_SIZE+1)/2 - 1));
                new_branch.branch->occupy = (PAGE_ENTRY_SIZE+1)/2 - 1;

                /* set the first-pointer of the new branch */
                (**new_child) = tmp_tentry[(PAGE_ENTRY_SIZE+1)/2]; /* smallest value on new_branch, points to new_branch */
                new_branch.branch->first_ptr = (*new_child)->page_ptr;
                (*new_child)->page_ptr = new_branch;

                /* we've just splitted the root */
                if ((*root).branch == node.branch) {
                    tree_page_ptr_t new_root = bp__new_page(TYPE_BRANCH);
                    new_root.branch->occupy = 1;
                    new_root.branch->first_ptr = node;
                    new_root.branch->tentry[0] = **new_child;
                    (*root) = new_root;
                    (*total_level)++;
                    free(*new_child);
                }

                return;

            } else {
                node.branch->tentry[node.branch->occupy++] = **new_child;
                // sort entries
                bp__sort(node.branch->tentry, node.branch->occupy, sizeof(tree_entry_t), TYPE_BRANCH, type);
                free(*new_child);
                *new_child = NULL;
                return;
            }
        }
    }
    /* leaf */
    else {
        // Split the leaf page only if it's full
        if (node.leaf->occupy == PAGE_ENTRY_SIZE) {

            tree_page_ptr_t new_leaf = bp__new_page(TYPE_LEAF);
            *new_child = (tree_entry_t *)calloc(1, sizeof(tree_entry_t));

            // copy to tmp_dentry and sort it
            data_entry_t tmp_dentry[PAGE_ENTRY_SIZE+1];
            memcpy(tmp_dentry, node.leaf->dentry, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
            tmp_dentry[PAGE_ENTRY_SIZE] = *entry;


            // sort entries
            bp__sort(tmp_dentry, PAGE_ENTRY_SIZE+1, sizeof(data_entry_t), TYPE_LEAF, type);

            // copy the half entries to new leaf
            memset(node.leaf->dentry, 0, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
            memcpy(node.leaf->dentry, &tmp_dentry, sizeof(data_entry_t)*(PAGE_ENTRY_SIZE+1)/2);
            node.leaf->occupy = (PAGE_ENTRY_SIZE+1)/2;
            memcpy(new_leaf.leaf->dentry, &tmp_dentry[(PAGE_ENTRY_SIZE+1)/2], sizeof(data_entry_t)*(PAGE_ENTRY_SIZE+1)/2);
            new_leaf.leaf->occupy = (PAGE_ENTRY_SIZE+1)/2;

            // set new_child
            (*new_child)->page_ptr = new_leaf;
            (*new_child)->key = new_leaf.leaf->dentry[0].key;

            // set sibling pointers
            new_leaf.leaf->next = node.leaf->next;
            new_leaf.leaf->prev = node.leaf;
            node.leaf->next = new_leaf.leaf;

            // for the first split, we need to create a new root
            if((*root).leaf == node.leaf) {
                tree_page_ptr_t new_root = bp__new_page(TYPE_BRANCH);
                new_root.branch->occupy = 1;
                new_root.branch->tentry[0] = **new_child;
                new_root.branch->first_ptr = node;
                (*root).branch = new_root.branch;
                (*total_level)++;
                free(*new_child);
            }

            return;
        } else {
            node.leaf->dentry[node.leaf->occupy++] = *entry;
            bp__sort(node.leaf->dentry, node.leaf->occupy, sizeof(data_entry_t), TYPE_LEAF, type);
            *new_child = NULL;
            return;
        }
    }
}


tree_page_ptr_t bp__get(tree_page_ptr_t node, index_t key, int level, bp_key_t type)
{
    // Search the tree top down to find a entry of a leaf page that links to a record
    if (level == 0)
        return node;
    else {
        // less than the first one
        /* Find the child */
        if (key__cmp(node.branch->tentry[0].key, key, type) > 0) {
            return bp__get(node.branch->first_ptr, key, level-1, type);
        } else if (node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr.leaf!=NULL &&
                   key__cmp(node.branch->tentry[PAGE_ENTRY_SIZE-1].key, key, type) <= 0) {
            return bp__get(node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr, key, level-1, type);
        } else {
            for (int i=0; i<PAGE_ENTRY_SIZE-1; i++) {
                if (key__cmp(node.branch->tentry[i].key, key, type) <= 0 &&
                        (key__cmp(node.branch->tentry[i+1].key, key, type)) > 0 ||
                        node.branch->tentry[i+1].page_ptr.leaf==NULL) {
                    return bp__get(node.branch->tentry[i].page_ptr, key, level-1, type);
                }
            }

            return node;
        }
    }
}


void _print_data_entry(data_entry_t dentry, bp_key_t type)
{
    if(type == TYPE_INT)
        printf("key: %d\n", dentry.key.i);
    else
        printf("key: %s\n", dentry.key.str);

    printf("------> pid: %d\n", dentry.pid);
    printf("----> slot#: %d\n", dentry.slot_num);
}


data_entry_t bp__find_record(tree_page_ptr_t root, index_t key, int level, bp_key_t type)
{

    tree_page_ptr_t found_leaf = bp__get(root, key, level, type);

    for(int i=0; i<PAGE_ENTRY_SIZE; i++) {
        if(key__cmp(found_leaf.leaf->dentry[i].key, key, type)==0) {
            _print_data_entry(found_leaf.leaf->dentry[i], type);

            return found_leaf.leaf->dentry[i];
        }
    }

    if(type == TYPE_INT)
        printf("Cannot find the record with key-%d\n", key.i);
    else
        printf("Cannot find the record with key-%s\n", key.str);

}


void bp__range_search(tree_page_ptr_t root, index_t key1, index_t key2, int level, bp_key_t type)
{
    // Use bp__get to get the location of the first page
    tree_page_ptr_t found_leaf = bp__get(root, key1, level, type);
    data_entry_t found_record;

    int entry_index = -1;
    for(int i=0; i<PAGE_ENTRY_SIZE; i++) {
        if(key__cmp(found_leaf.leaf->dentry[i].key, key1, type)==0) {
            entry_index = i;
            break;
        }
    }

    // Iterate with the doubly-linked list
    if(entry_index == -1)
        return;
    else {
        while(found_leaf.leaf != NULL) {
            for(entry_index; entry_index<found_leaf.leaf->occupy; entry_index++) {
                found_record = found_leaf.leaf->dentry[entry_index];
                if(key__cmp(found_record.key, key1, type)>=0 &&
                        key__cmp(found_record.key, key2, type)<=0) {
                    _print_data_entry(found_record, type);
                } else {
                    return;
                }
            }
            found_leaf.leaf = found_leaf.leaf->next;
            entry_index = 0;
        }
    }
}


int bp__scan(tree_page_ptr_t node, int level, bp_key_t ktype, int is_print)
{
    // Do a DFS to print all the records
    int count = 0;

    // If the node is "branch"
    if(level > 0) {
        if(is_print)
            print_entries(node, TYPE_BRANCH, ktype);

        count += bp__scan(node.branch->first_ptr, level-1, ktype, is_print);
        for(int i=0; i<PAGE_ENTRY_SIZE; i++) {
            if(node.branch->tentry[i].page_ptr.branch!=NULL)
                count += bp__scan(node.branch->tentry[i].page_ptr, level-1, ktype, is_print);
        }

        return count+1;

        // If the node is "leaf"
    } else {
        if(is_print)
            print_entries(node, TYPE_LEAF, ktype);
        return 1;
    }
}


void print_entries(tree_page_ptr_t t_ptr, tree_page_t etype, bp_key_t ktype) {

    if(t_ptr.branch == NULL)
        return;

    printf("\n=============");
    if(etype==TYPE_LEAF) {
        printf("\n* Leaf Page *");
    } else {
        printf("\n*Branch Page*");
    }
    printf("\n=============");

    printf("\nIndex |");
    for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
        printf("%4d|", j);
    }
    printf("\n");
    printf("------|");
    for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
        printf("----|");
    }
    printf("\n");
    printf("Key   |");

    if(etype == TYPE_LEAF) {
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            if(ktype == TYPE_INT)
                printf("%4d|", t_ptr.leaf->dentry[j].key.i);
            else
                printf("%4s|", t_ptr.leaf->dentry[j].key.str);
        }
        printf("\n");
        printf("------|");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("----|");
        }
        printf("\n");
        printf("Pid   |");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("%4d|", t_ptr.leaf->dentry[j].pid);
        }
        printf("\n");
        printf("------|");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("----|");
        }
        printf("\n");
        printf("Slot# |");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("%4d|", t_ptr.leaf->dentry[j].slot_num);
        }
    } else {
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            if(ktype == TYPE_INT)
                printf("%4d|", t_ptr.branch->tentry[j].key.i);
            else
                printf("%4s|", t_ptr.branch->tentry[j].key.str);
        }
    }

    printf("\n");
}


void bp__delete(tree_page_ptr_t node, index_t key)
{
    // Find the leaf page and the slot(s) for delete

    // Determine whether a re-distribution is needed

    // If a re-distribution is not possible, pull down and merge

    // Repeat the process recursively
}
