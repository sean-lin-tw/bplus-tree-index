#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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
            { return strncmp(((data_entry_t*)a)->key.str, ((data_entry_t*)b)->key.str, 10); }));
        }
    } else {
        if(key_type == TYPE_INT) {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return ( ((tree_entry_t*)a)->key.i - ((tree_entry_t*)b)->key.i ); }));
        } else {
            qsort(base, nitems, size,
                  lambda (int, (const void *a, const void *b),
            { return strncmp(((tree_entry_t*)a)->key.str, ((tree_entry_t*)b)->key.str, 10); }));
        }
    }
}


int key__cmp (index_t key1, index_t key2, bp_key_t type)
{
    if (type == TYPE_INT)
        return key1.i - key2.i;
    else
        return strncmp(key1.str, key2.str, 10);
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
    // Find the leaf page and the slot for insert (top-down)
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
        /* We've splited the lower page */
        else {
            /* set uplevel */
            if (level > 1)
                (*new_child)->page_ptr.branch->uplevel = node.branch;
            else
                (*new_child)->page_ptr.leaf->uplevel = node.branch;

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

                // set uplevel
                node.leaf->uplevel = root->branch;
                new_leaf.leaf->uplevel = root->branch;

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

    data_entry_t fail_record;
    fail_record.pid = -1;
    fail_record.slot_num = -1;
    return fail_record;
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
        for(int i=0; i<node.branch->occupy; i++) {
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


int bp__scan_leaf(tree_page_ptr_t node, int level)
{
    // Do a DFS to find all the leaf pages
    int count = 0;

    // If the node is "branch"
    if(level > 0) {
        count += bp__scan_leaf(node.branch->first_ptr, level-1);
        for(int i=0; i<node.branch->occupy; i++) {
            if(node.branch->tentry[i].page_ptr.branch!=NULL)
                count += bp__scan_leaf(node.branch->tentry[i].page_ptr, level-1);
        }
        return count;
        // If the node is "leaf"
    } else {
        return 1;
    }
}

void print_entries(tree_page_ptr_t t_ptr, tree_page_t etype, bp_key_t ktype)
{

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
    printf("\n------|");
    for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
        printf("----|");
    }

    printf("\nKey   |");
    if(etype == TYPE_LEAF) {
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            if(ktype == TYPE_INT)
                printf("%4d|", t_ptr.leaf->dentry[j].key.i);
            else
                printf("%4s|", t_ptr.leaf->dentry[j].key.str);
        }

        printf("\n------|");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("----|");
        }
        printf("\nPid   |");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("%4d|", t_ptr.leaf->dentry[j].pid);
        }

        printf("\n------|");
        for(int j=0; j<PAGE_ENTRY_SIZE; j++) {
            printf("----|");
        }
        printf("\nSlot# |");
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

// TODO:
// root deletion
//
// refactor

void bp__delete(tree_page_ptr_t* root,
                tree_page_ptr_t node,
                index_t key,
                tree_entry_t** old_child,
                int call_entry_idx,
                int level,
                int* total_level,
                bp_key_t type)

{
    // Find the leaf page and the slot(s) for delete
    /* non-leaf */
    if (level > 0) {

        /* Find the child */
        if (key__cmp(node.branch->tentry[0].key, key, type) > 0) {
            bp__delete(root, node.branch->first_ptr, key, old_child, -1, level-1, total_level, type);
        } else if (node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr.leaf!=NULL &&
                   key__cmp(node.branch->tentry[PAGE_ENTRY_SIZE-1].key, key, type) <= 0) {
            bp__delete(root, node.branch->tentry[PAGE_ENTRY_SIZE-1].page_ptr, key, old_child, PAGE_ENTRY_SIZE-1, level-1, total_level, type);
        } else {
            for (int i=0; i<PAGE_ENTRY_SIZE-1; i++) {
                if ((key__cmp(node.branch->tentry[i].key, key, type) <= 0 &&
                    (key__cmp(node.branch->tentry[i+1].key, key, type)) > 0) ||
                     node.branch->tentry[i+1].page_ptr.leaf==NULL) {
                    bp__delete(root, node.branch->tentry[i].page_ptr, key, old_child, i, level-1, total_level, type);
                    break;
                }
            }
        }
        /* No merge happened in leaf */
        if (*old_child == NULL)
            return;
        /* Merge happened. Delete child */
        else {
            // remove old_child from this page
            int offset = (*old_child - node.branch->tentry);
            tree_page_ptr_t *orphan = &((*old_child)->page_ptr);
            node.branch->tentry[offset] = node.branch->tentry[node.branch->occupy-1];
            node.branch->occupy--;

            /* this is root */
            if (root->branch == node.branch) {
                if (node.branch->occupy == 0) {// only entry left in root
                    
                    // reset root pointer
					branch_page_t *old_root = root->branch;
					if (level > 1)
                   		root->branch = (&(root->branch->first_ptr)) ?(root->branch->first_ptr.branch) :(root->branch->tentry[0].page_ptr.branch);
					else
                   		root->leaf = (&(root->branch->first_ptr)) ?(root->branch->first_ptr.leaf) :(root->branch->tentry[0].page_ptr.leaf);

                    // free old root
                    free(old_root);
					(*total_level)--;	
                } else
                    bp__sort(node.branch->tentry, (size_t)node.branch->occupy, sizeof(tree_entry_t), TYPE_BRANCH, type);
				*old_child = NULL;				
                return;
            }

            // enough entries left to spare
            if (node.branch->occupy > floor(PAGE_ENTRY_SIZE/2)) {
                *old_child = NULL;
                return;
            } else {
                // try to re-distribute from siblings
                branch_page_t *prev = (call_entry_idx < 0) ?NULL :(
                                          (call_entry_idx == 0) ?node.branch->uplevel->first_ptr.branch
                                          :node.branch->uplevel->tentry[call_entry_idx-1].page_ptr.branch);

                branch_page_t *next = (call_entry_idx < PAGE_ENTRY_SIZE-1 && call_entry_idx < node.branch->occupy) ?node.branch->uplevel->tentry[call_entry_idx+1].page_ptr.branch :NULL;

                /* left re-distribution*/
                if (call_entry_idx >= 0 && prev->occupy > floor(PAGE_ENTRY_SIZE/2)) {

                    // sort in tmp_tentry
                    uint8_t sum_entries = node.branch->occupy + prev->occupy;
                    tree_entry_t tmp_tentry[sum_entries];
                    memcpy(tmp_tentry, node.branch->tentry, sizeof(tree_entry_t)*node.branch->occupy);
                    memcpy(tmp_tentry + sizeof(tree_entry_t)*node.branch->occupy, prev->tentry, sizeof(tree_entry_t)*prev->occupy);
                    bp__sort(tmp_tentry, sum_entries, sizeof(tree_entry_t), TYPE_BRANCH, type);

                    // evenly spare the entries into the two pages
                    memset(prev->tentry, 0, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                    memcpy(prev->tentry, &tmp_tentry, sizeof(tree_entry_t)*floor(sum_entries/2));
                    prev->occupy = floor(sum_entries/2);
                    memset(node.branch->tentry, 0, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                    memcpy(node.branch->tentry, tmp_tentry+sizeof(tree_entry_t)*(size_t)floor(sum_entries/2), sizeof(tree_entry_t)*ceil(sum_entries/2));
                    node.branch->occupy = ceil(sum_entries/2);

                    // replace the key value of the parent level
                    node.branch->uplevel->tentry[call_entry_idx].key = node.branch->tentry[0].key;
                    *old_child = NULL;
                    return;
                }
                /* right re-distribution*/
                else if (call_entry_idx < PAGE_ENTRY_SIZE-1 &&
                         next != NULL &&
                         next->occupy > floor(PAGE_ENTRY_SIZE/2) ) {

                    // sort in tmp_tentry
                    uint8_t sum_entries = node.branch->occupy + next->occupy;
                    tree_entry_t tmp_tentry[sum_entries];
                    memcpy(tmp_tentry, node.branch->tentry, sizeof(tree_entry_t)*node.branch->occupy);
                    memcpy(tmp_tentry + sizeof(tree_entry_t)*node.branch->occupy, next->tentry, sizeof(tree_entry_t)*next->occupy);
                    bp__sort(tmp_tentry, sum_entries, sizeof(tree_entry_t), TYPE_BRANCH, type);

                    // evenly spare the entries into the two pages
                    memset(node.branch->tentry, 0, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                    memcpy(node.branch->tentry, &tmp_tentry, sizeof(tree_entry_t)*floor(sum_entries/2));
                    node.branch->occupy = floor(sum_entries/2);
                    memset(next->tentry, 0, sizeof(tree_entry_t)*PAGE_ENTRY_SIZE);
                    memcpy(next->tentry, tmp_tentry+sizeof(tree_entry_t)*(size_t)floor(sum_entries/2), sizeof(tree_entry_t)*ceil(sum_entries/2));
                    next->occupy = ceil(sum_entries/2);

                    // replace the key value of the parent level
                    node.branch->uplevel->tentry[call_entry_idx+1].key = next->tentry[0].key;
                    *old_child = NULL;
                    return;
                }
                // otherwise, merge with a neighbor
                else {
                    /* right-merge */
                    if (call_entry_idx < 0) {

                        // PULLDOWN, merge and sort
                        uint8_t sum_entries = node.branch->occupy + next->occupy + 1;
                        memcpy(node.branch->tentry + sizeof(tree_entry_t)*node.branch->occupy, next->tentry, sizeof(tree_entry_t)*next->occupy);
                        node.branch->tentry[sum_entries-1].key = node.branch->uplevel->tentry[call_entry_idx+1].key;
                        node.branch->tentry[sum_entries-1].page_ptr = *orphan;
                        bp__sort(node.branch->tentry, sum_entries, sizeof(tree_entry_t), TYPE_LEAF, type);
                        node.branch->occupy = sum_entries;

                        // The key that needs to be removed in the upper level
                        *old_child = &(node.branch->uplevel->tentry[call_entry_idx+1]);

                        // garbage collection
                        free(next);
                    }
                    /* left-merge */
                    else {
                        // merge and sort
                        uint8_t sum_entries = node.branch->occupy + prev->occupy;
                        memcpy(node.branch->tentry + sizeof(tree_entry_t)*node.branch->occupy, prev->tentry, sizeof(tree_entry_t)*prev->occupy);
                        node.branch->tentry[sum_entries-1].key = node.branch->uplevel->tentry[call_entry_idx-1].key;
                        node.branch->tentry[sum_entries-1].page_ptr = *orphan;
                        bp__sort(node.branch->tentry, sum_entries, sizeof(tree_entry_t), TYPE_LEAF, type);
                        node.branch->occupy = sum_entries;

                        // The key that needs to be removed in the upper level
                        *old_child = &(node.branch->uplevel->tentry[call_entry_idx-1]);

                        // garbage collection
                        free(prev);
                    }
                    return;
                }
            }
        }
    }
    /* leaf */
    else {
        // enough entries left to spare
        if (node.leaf->occupy > floor(PAGE_ENTRY_SIZE/2)) {
            for (int i=0; i<node.leaf->occupy; i++) {
                if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                    node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                    node.leaf->occupy--;
                    break;
                }
            }
            bp__sort(node.leaf->dentry, (size_t)node.leaf->occupy, sizeof(data_entry_t), TYPE_LEAF, type);
            *old_child = NULL;
            return;
        } else {
            // try to re-distribute from siblings
            // TODO: consider boundary cases(e.g. NULL pages)
            /* this is root */
            if (root->leaf == node.leaf) {
                if (node.leaf->occupy == 1) { // only entry left in root
                    node.leaf->occupy--;
                    return;
                } else {
                    // remove the entry
                    for (int i=0; i<node.leaf->occupy; i++) {
                        if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                            node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                            node.leaf->occupy--;
                            break;
                        }
                    }
                    bp__sort(node.leaf->dentry, (size_t)node.leaf->occupy, sizeof(data_entry_t), TYPE_LEAF, type);
                }
                return;
            }

            /* left re-distribution */
            if (call_entry_idx >= 0 && node.leaf->prev->occupy > floor(PAGE_ENTRY_SIZE/2)) {
                // remove the entry
                for (int i=0; i<node.leaf->occupy; i++) {
                    if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                        node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                        node.leaf->occupy--;
                        break;
                    }
                }
                // sort in tmp_dentry
                uint8_t sum_entries = node.leaf->occupy + node.leaf->prev->occupy;
                data_entry_t tmp_dentry[sum_entries];
                memcpy(tmp_dentry, node.leaf->dentry, sizeof(data_entry_t)*node.leaf->occupy);
                memcpy(&tmp_dentry[node.leaf->occupy], node.leaf->prev->dentry, sizeof(data_entry_t)*node.leaf->prev->occupy);
                bp__sort(tmp_dentry, sum_entries, sizeof(data_entry_t), TYPE_LEAF, type);

                // evenly spare the entries into the two pages
                memset(node.leaf->prev->dentry, 0, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
                memcpy(node.leaf->prev->dentry, &tmp_dentry, sizeof(data_entry_t)*floor(sum_entries/2));
                node.leaf->prev->occupy = floor(sum_entries/2);
                memset(node.leaf->dentry, 0, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
                memcpy(node.leaf->dentry, &tmp_dentry[(int)floor(sum_entries/2)], sizeof(data_entry_t)*ceil(sum_entries/2));
                node.leaf->occupy = ceil(sum_entries/2);

                // replace the key value of the parent level
                node.leaf->uplevel->tentry[call_entry_idx].key = node.leaf->dentry[0].key;
                *old_child = NULL;
                return;
            }
            /* right re-distribution */
            else if (call_entry_idx < PAGE_ENTRY_SIZE-1 &&
                     node.leaf->uplevel->tentry[call_entry_idx+1].page_ptr.leaf != NULL &&
                     node.leaf->next->occupy > floor(PAGE_ENTRY_SIZE/2) ) {
                // remove the entry
                for (int i=0; i<node.leaf->occupy; i++) {
                    if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                        node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                        node.leaf->occupy--;
                        break;
                    }
                }
                // sort in tmp_dentry
                uint8_t sum_entries = node.leaf->occupy + node.leaf->next->occupy;
                data_entry_t tmp_dentry[sum_entries];
                memcpy(tmp_dentry, node.leaf->dentry, sizeof(data_entry_t)*node.leaf->occupy);
                memcpy(&tmp_dentry[node.leaf->occupy], node.leaf->next->dentry, sizeof(data_entry_t)*node.leaf->next->occupy);
                bp__sort(tmp_dentry, sum_entries, sizeof(data_entry_t), TYPE_LEAF, type);

                // evenly spare the entries into the two pages
                memset(node.leaf->dentry, 0, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
                memcpy(node.leaf->dentry, &tmp_dentry, sizeof(data_entry_t)*floor(sum_entries/2));
                node.leaf->occupy = floor(sum_entries/2);
                memset(node.leaf->next->dentry, 0, sizeof(data_entry_t)*PAGE_ENTRY_SIZE);
                memcpy(node.leaf->next->dentry, &tmp_dentry[(int)floor(sum_entries/2)], sizeof(data_entry_t)*ceil(sum_entries/2));
                node.leaf->next->occupy = ceil(sum_entries/2);

                // replace the key value of the parent level
                node.leaf->uplevel->tentry[call_entry_idx+1].key = node.leaf->next->dentry[0].key;
                *old_child = NULL;
                return;
            }
            // otherwise, merge with a neighbor
            else {
                /* right-merge */
                if (call_entry_idx < 0) {
                    // remove the entry
                    for (int i=0; i<node.leaf->occupy; i++) {
                        if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                            node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                            node.leaf->occupy--;
                            break;
                        }
                    }
                    // merge and sort
                    uint8_t sum_entries = node.leaf->occupy + node.leaf->next->occupy;
                    memcpy(node.leaf->dentry + node.leaf->occupy, node.leaf->next->dentry, sizeof(data_entry_t)*node.leaf->next->occupy);
                    bp__sort(node.leaf->dentry, sum_entries, sizeof(data_entry_t), TYPE_LEAF, type);
                    node.leaf->occupy = sum_entries;

                    // The key that needs to be removed in the upper level
                    *old_child = &(node.leaf->uplevel->tentry[call_entry_idx+1]);

                    // adjust sibling pointers & garbage collection
                    leaf_page_t *gc_page = node.leaf->next;
                    node.leaf->next = node.leaf->next->next;
					if (node.leaf->next)
                    	node.leaf->next->prev = node.leaf;
                    free(gc_page);
					node.leaf->uplevel->tentry[call_entry_idx+1].page_ptr.leaf= NULL;
                }
                /* left-merge */
                else {
                    // remove the entry
                    for (int i=0; i<node.leaf->occupy; i++) {
                        if (key__cmp(node.leaf->dentry[i].key, key, type) == 0) {
                            node.leaf->dentry[i] = node.leaf->dentry[node.leaf->occupy-1];
                            node.leaf->occupy--;
                            break;
                        }
                    }
                    // merge and sort
                    uint8_t sum_entries = node.leaf->occupy + node.leaf->prev->occupy;
                    memcpy(&(node.leaf->dentry[node.leaf->occupy]), node.leaf->prev->dentry, sizeof(data_entry_t)*node.leaf->prev->occupy);
                    bp__sort(node.leaf->dentry, sum_entries, sizeof(data_entry_t), TYPE_LEAF, type);
                    node.leaf->occupy = sum_entries;

                    // The key that needs to be removed in the upper level
                    *old_child = &(node.leaf->uplevel->tentry[call_entry_idx-1]);

                    // adjust sibling pointers & garbage collection
                    leaf_page_t *gc_page = node.leaf->prev;
                    node.leaf->prev = node.leaf->prev->prev;
					if (node.leaf->prev)
                   	 	node.leaf->prev->next = node.leaf;
                    free(gc_page);
					if (call_entry_idx >= 0)
						node.leaf->uplevel->tentry[call_entry_idx-1].page_ptr.leaf = NULL;
					else
						node.leaf->uplevel->first_ptr.leaf = NULL;
                }
                return;
            }
        }
    }
}
