#include <stdint.h>

#define DIRECTORY_PAGE_NUM 63
#define RECORD_PAGE_BUFFER_SIZE 497

typedef struct directory_page_s directory_page_t;
typedef struct record_entry_s record_entry_t;
typedef struct record_page_s record_page_t;


struct record_page_s {
  uint8_t total_slot;
  uint32_t bitmap;
  uint8_t buffer[RECORD_PAGE_BUFFER_SIZE];
};
// __attribute__((aligned(512)));;


struct record_entry_s {
  uint8_t is_full;
  uint8_t slot_limit;
  uint16_t pid;

  record_page_t* rpage;
};


struct directory_page_s {
  uint8_t is_full;
  uint8_t fullpage_num;
  uint16_t pid_base;

  directory_page_t* next;
  record_entry_t entry[DIRECTORY_PAGE_NUM];
};
// __attribute__((aligned(512)));;
