#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/bptree.h"

char* randstring(size_t length);

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
        bp__insert(&root, root, &inserted_entry, &tmp_new_child, level, &level, TYPE_INT);
    }

    // Test bp__scan
    printf("\n\nTotal index pages: %d\n", bp__scan(root, level, TYPE_INT, 1));
    printf("Total leaf pages: %d\n", bp__scan_leaf(root, level));

    // Test bp__range_search
    printf("\n\n\n");
    index_t key2;
    inserted_entry.key.i = 25;
    key2.i = 35;
    bp__range_search(root, inserted_entry.key, key2, level, TYPE_INT);

    // Test bp__find_record
    printf("\n\n\n");
    inserted_entry.key.i = 439;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 258;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 694;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);
    inserted_entry.key.i = 9527;
    bp__find_record(root, inserted_entry.key, level, TYPE_INT);


    // -------------------------------------------------------------------------------
    // ---------------------------------- Test String --------------------------------
    // -------------------------------------------------------------------------------

    tree_page_ptr_t str_root;
    str_root.leaf = (leaf_page_t*) calloc(1, sizeof(leaf_page_t));
    int str_level = 0;

    data_entry_t str_inserted_entry;
    tree_entry_t* str_tmp_new_child = NULL;
    memset(str_inserted_entry.key.str, '\0',10);

    // Insert a lot of data-entry
    for(int j=0; j<100; j++) {
        memset(str_inserted_entry.key.str, j+64, 10);

        str_inserted_entry.pid = j;
        str_inserted_entry.slot_num = j;
        bp__insert(&str_root, str_root, &str_inserted_entry,
                   &str_tmp_new_child, str_level, &str_level, TYPE_STRING);
    }

    // Test bp__scan
    printf("\n\nTotal index pages: %d\n", bp__scan(str_root, str_level, TYPE_STRING, 1));
    printf("Total leaf pages: %d\n", bp__scan_leaf(str_root, str_level));

    // Test bp__range_search
    printf("\n\n\n");
    index_t str_key2;
    memset(str_inserted_entry.key.str, 83, 10);
    memset(str_key2.str, 99, 4);
    bp__range_search(str_root, str_inserted_entry.key, str_key2, str_level, TYPE_STRING);

    printf("\n\n\n");

    // Test bp__find_record
    memset(str_inserted_entry.key.str, 85, 10);
    bp__find_record(str_root, str_inserted_entry.key, str_level, TYPE_STRING);
    memset(str_inserted_entry.key.str, 97, 10);
    bp__find_record(str_root, str_inserted_entry.key, str_level, TYPE_STRING);
    memset(str_inserted_entry.key.str, 150, 10);
    bp__find_record(str_root, str_inserted_entry.key, str_level, TYPE_STRING);
    memset(str_inserted_entry.key.str, 209, 10);
    bp__find_record(str_root, str_inserted_entry.key, str_level, TYPE_STRING);


    return 0;
}

char* randstring(size_t length)
{

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    char* randomString = NULL;

    if (length) {
        randomString = (char*) malloc(sizeof(char) * (length +1));

        if (randomString) {
            for (int n = 0; n < length; n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}
