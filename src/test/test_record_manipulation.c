#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/relation.h"

int main() {
  record_page_entry_t* test_rpage_entry = calloc(1, sizeof(record_page_entry_t));
  record_page_t* test_rpage = calloc(1, sizeof(record_page_t));

  uint16_t test_record_size = 50; // test_record_size is key-size + remained-record-size
  char* rem_rec = calloc(20, sizeof(char));
  index_t test_key;

  test_rpage_entry->rpage = test_rpage;
  test_rpage->dirct_end_ptr = RECORD_PAGE_BUFFER_SIZE-1;

  //----------------------------------Insert record----------------------------------
  test_key.i = 10;
  rem_rec = "Hello, world";
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);

  test_key.i = 8;
  rem_rec = "I love JavaScript!";
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);

  test_key.i = 5;
  rem_rec = "Oh no!";
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);

  test_key.i = 3000;
  rem_rec = "QQQQQ";
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);
  rpage__insert_record(test_rpage_entry, test_record_size, test_key, rem_rec);

  //----------------------------------Show the page----------------------------------
  rpage__show_page(test_rpage_entry, test_record_size, 0);

}
