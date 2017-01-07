#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/relation.h"

char* randstring(size_t length);

int main(){

  //-------------------- Initial a database --------------------
  relation_page_t db = {0};

  //-------------------- Create a relation and display the information --------------------
  relation__create(&db, "Student", TYPE_INT, 100);
  relation_display_info(&db.relations[0]);

  //-------------------- Insert a lot of random records into this relation --------------------
  index_t test_key;
  char* rem_rec = calloc(20, sizeof(char));
  relation_t* cur_relation = &db.relations[0];

  for(int i=0; i<503; i++) {
    test_key.i = i;
    rem_rec = randstring(25);
    relation__insert(cur_relation, test_key, rem_rec);
  }

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
  dpage__show_record(cur_relation->page_header,
                     cur_relation->record_length,
                     cur_relation->ktype, 36, 2);
  dpage__show_record(cur_relation->page_header,
                     cur_relation->record_length,
                     cur_relation->ktype, 24, 3);
  dpage__show_record(cur_relation->page_header,
                     cur_relation->record_length,
                     cur_relation->ktype, 68, 1);
  dpage__show_record(cur_relation->page_header,
                     cur_relation->record_length,
                     cur_relation->ktype, 68, 23);
  dpage__show_record(cur_relation->page_header,
                     cur_relation->record_length,
                     cur_relation->ktype, 500, 3);

  //-------------------- Create another relation and display the information --------------------
  relation__create(&db, "Professor", TYPE_STRING, 25);
  relation_display_info(&db.relations[1]);

  //-------------------- Display the information of a non-exist relation--------------------
  relation_display_info(&db.relations[2]);

}


char* randstring(size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    char* randomString = NULL;

    if (length) {
        randomString = malloc(sizeof(char) * (length +1));

        if (randomString) {
            for (int n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}
