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
    printf("\n*********************\n");
    printf("| Create a relation |\n");
    printf("*********************\n");

    relation__create(&db, "Student", TYPE_INT, 100);
    relation_t* cur_relation = get_relation(&db, "Student");

    relation_display_info(cur_relation);

    //-------------------- Insert a lot of random records into this relation --------------------
    printf("\n***************************\n");
    printf("| Insert a lot of records |\n");
    printf("***************************\n");

    index_t test_key;
    char* rem_rec = (char*) calloc(20, sizeof(char));

    for(int i=0; i<503; i++) {
        test_key.i = i;
        rem_rec = randstring((rand()%30+10));
        relation__insert(cur_relation, test_key, rem_rec);
    }

    //-------------------- Delete two records from this relation --------------------
    printf("\n************************\n");
    printf("| Delete a few records |\n");
    printf("************************\n");

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


    // //-------------------- Display some pages of this relation --------------------
    printf("\n**********************\n");
    printf("| Display some pages |\n");
    printf("**********************\n");

    relation__page_display(cur_relation, 15);
    relation__page_display(cur_relation, 23);
    relation__page_display(cur_relation, 59);

    //-------------------- Display the index information of this relation --------------------
    printf("\n*********************************\n");
    printf("| Display the index information |\n");
    printf("*********************************\n");

    relation__index_scan(cur_relation);

    //-------------------- Display a single record of this relation with key --------------------
    printf("\n*******************************\n");
    printf("| Display some single records |\n");
    printf("*******************************\n");

    test_key.i = 319;
    relation__find(cur_relation, test_key);

    test_key.i = 258;
    relation__find(cur_relation, test_key);

    test_key.i = 900;
    relation__find(cur_relation, test_key);

    //-------------------- Display a lot of records of this relation with key --------------------
    printf("\n*******************\n");
    printf("| Do range search |\n");
    printf("*******************\n");

    index_t test_key2;
    test_key.i = 342;
    test_key2.i = 378;
    relation__find_range(cur_relation, test_key, test_key2);

    //-------------------- Display the file and index statistic of this relation --------------------
    printf("\n*************************\n");
    printf("| Display the statistic |\n");
    printf("*************************\n");

    relation__statistic(cur_relation);


    // //==================== Create another relation and display the information ====================
    // relation__create(&db, "Professor", TYPE_STRING, 25);
    // cur_relation = &db.relations[1];
    // relation_display_info(cur_relation);
    //
    // //-------------------- Display the information of a non-exist relation--------------------
    // relation_display_info(&db.relations[2]);
    // printf("\nTotal slotted data page: %d\n\n", dpage__statistics(cur_relation->page_header));

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
