#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/relation.h"

int main(){

  relation_page_t db = {0};

  relation__create(&db, "Student", TYPE_INT, 50);
  relation_display_info(&db.relations[0]);

  relation__create(&db, "Professor", TYPE_STRING, 25);
  relation_display_info(&db.relations[1]);

  relation_display_info(&db.relations[2]);

}
