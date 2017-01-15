#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/relation.h"

char* randstring(size_t length);

int main()
{

    //-------------------- Initial a database --------------------
    relation_page_t db = {0};

    //-------------------- Create a relation and display the information --------------------
    relation__create(&db, "Student", TYPE_INT, 100);
    relation_display_info(&db.relations[0]);

    //-------------------- Insert a lot of random records into this relation --------------------
    index_t test_key;
    char* rem_rec = (char*) calloc(20, sizeof(char));
    relation_t* cur_relation = &db.relations[0];

    for(int i=0; i<503; i++) {
        test_key.i = i;
        rem_rec = randstring((rand()%30+10));
        relation__insert(cur_relation, test_key, rem_rec);
    }

    //-------------------- Delete two records from this relation --------------------
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 125, 0, ACTION_DELETE);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 125, 1, ACTION_DELETE);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 130, 0, ACTION_DELETE);
//-------------------- Insert back two records into this relation --------------------
    test_key.i = 503;
    rem_rec = randstring(30);
    relation__insert(cur_relation, test_key, rem_rec);
    test_key.i = 504;
    relation__insert(cur_relation, test_key, rem_rec);

    //-------------------- Display whole pages of this relation --------------------
    directory_page_t* cur_dirct = cur_relation->page_header;
    for(int i=0; i<500; i++) {
        if(cur_dirct!=NULL && cur_dirct->entry[i%DIRECTORY_ENTRY_NUM].rpage!=NULL) {
            rpage__show_page(&cur_dirct->entry[i%DIRECTORY_ENTRY_NUM],
                             cur_relation->record_length,
                             cur_relation->ktype);
            if((i%DIRECTORY_ENTRY_NUM == (DIRECTORY_ENTRY_NUM-1)))
                cur_dirct = cur_dirct->next;
        } else {
            break;
        }
    }

    //-------------------- Display a single record of this relation with PID --------------------

    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 36, 2, ACTION_PRINT);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 24, 3, ACTION_PRINT);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 68, 1, ACTION_PRINT);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 73, 23, ACTION_PRINT);
    dpage__find_record(cur_relation->page_header,
                       cur_relation->record_length,
                       cur_relation->ktype, 500, 3, ACTION_PRINT);

    printf("\n\n\nTotal slotted data page: %d\n\n", dpage__statistics(cur_relation->page_header));

    //-------------------- Create another relation and display the information --------------------
    relation__create(&db, "Professor", TYPE_STRING, 25);
    cur_relation = &db.relations[1];
    relation_display_info(cur_relation);

    //-------------------- Display the information of a non-exist relation--------------------
    relation_display_info(&db.relations[2]);
    printf("\n\n\nTotal slotted data page: %d\n\n", dpage__statistics(cur_relation->page_header));

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
