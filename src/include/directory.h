#ifndef DIRECTORY_PAGE
#define DIRECTORY_PAGE

#include "bptree.h"

#define BITMAP_SIZE 4
#define DIRECTORY_PAGE_NUM 63
#define RECORD_PAGE_BUFFER_SIZE 506

typedef struct directory_page_s directory_page_t;
typedef struct record_page_entry_s record_page_entry_t;
typedef struct record_page_s record_page_t;
typedef struct slot_entry_s slot_entry_t;


uint16_t rpage__insert_record(record_page_entry_t* page_entry,
                              uint16_t record_size,
                              index_t key,
                              const char* remained_record);

void rpage__show_page(record_page_entry_t* page_entry, uint16_t record_size, int key_type);

struct slot_entry_s {
  uint16_t offset;
  uint16_t reclen;
};


struct record_page_s {
  uint16_t slot_num;
  uint16_t free_ptr;
  uint16_t dirct_end_ptr;
  uint8_t buffer[RECORD_PAGE_BUFFER_SIZE];
};


struct record_page_entry_s {
  uint16_t is_full;
  uint16_t pid;

  record_page_t* rpage;
};


struct directory_page_s {
  uint8_t is_full;
  uint8_t fullpage_num;
  uint16_t pid_base;

  directory_page_t* next;
  record_page_entry_t entry[DIRECTORY_PAGE_NUM];
};

#endif
