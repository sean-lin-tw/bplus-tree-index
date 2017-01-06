#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/relation.h"

// When the page is NOT full
uint16_t rpage__insert_record(record_page_entry_t* page_entry,
                              uint16_t record_size,
                              index_t key,
                              const char* remained_record)
{
  if(page_entry==NULL)
    return -1;

  if(page_entry->is_full == 1) {
    fprintf(stderr, "This page is full!\n");
    return -1;
  }

  uint16_t sid;
  record_page_t* page = page_entry->rpage;

  // Create a slot_entry
  slot_entry_t* slot_entry = malloc(sizeof(slot_entry_t));
  slot_entry->reclen = record_size;

  // ----------> If the page has free space
  if( (page->dirct_end_ptr - page->free_ptr) > (record_size + PAGE_ID_SIZE) ) {
    sid = page->slot_num;
    slot_entry->offset = page->free_ptr;

    // Insert the slot_entry to the page
    page->dirct_end_ptr -= 3;
    memcpy(&page->buffer[page->dirct_end_ptr], slot_entry, sizeof(slot_entry_t));
    page->dirct_end_ptr--;

    // Modify the number of total slot and free_ptr
    page->free_ptr += (record_size);
    page->slot_num++;

    // Check if the page is full
    if( (page->dirct_end_ptr - page->free_ptr) < (record_size + PAGE_ID_SIZE) )
      rpage__scan_full(page_entry, NULL);

  // ----------> If the page is not full but does not have enough free space
  } else {
    // Find the free slot
    sid = rpage__scan_full(page_entry, slot_entry);

    // Insert the slot_entry to the page
    memcpy(&page->buffer[(RECORD_PAGE_BUFFER_SIZE - 1) - (sid*PAGE_ID_SIZE + 3)],
           slot_entry,
           sizeof(slot_entry_t));
  }


  // Insert the record to the page
  uint16_t key_size = (key.i == 0 ? 10 : 4);

  memcpy(&page->buffer[slot_entry->offset], (const void *)&key, sizeof(uint8_t)*key_size);
  memcpy(&page->buffer[slot_entry->offset+key_size],
         remained_record,
         sizeof(uint8_t)*(record_size - key_size));

  free(slot_entry);
  return sid;
}



void rpage__show_page(record_page_entry_t* page_entry, uint16_t record_size, key_t key_type)
{
  if(page_entry==NULL)
    return;

  record_page_t* page = page_entry->rpage;
  slot_entry_t* slot_entry = malloc(sizeof(slot_entry_t));
  int key_size = (key_type == TYPE_INT ? 4 : 10);

  printf("======================================================\n");
  printf("Pid: %d\n", page_entry->pid);
  printf("Total slot number: %d\n", page->slot_num);
  printf("------------------------------------------------------\n");


  char* remained_record = calloc((record_size-key_size), sizeof(uint8_t));
  void* key = calloc(key_size, sizeof(uint8_t));

  for(uint16_t i=0; i<page->slot_num; i++) {
    memcpy(slot_entry,
           &page->buffer[(RECORD_PAGE_BUFFER_SIZE - 1) - (i*PAGE_ID_SIZE + 3)],
           sizeof(slot_entry_t));

    if(slot_entry->reclen != 0){
      // Get the record from the record page
      memcpy(key, &page->buffer[slot_entry->offset], sizeof(uint8_t)*key_size);
      memcpy(remained_record,
             &page->buffer[slot_entry->offset + key_size],
             sizeof(uint8_t)*(record_size-key_size));

      // Print the whole record
      if(key_type == TYPE_INT){
        printf("Slot-%d: key: %d, record: %s\n", i, *(int*)key, remained_record);
      } else {
        printf("Slot-%d: key: %s, record: %s\n", i, key, remained_record);
      }
      printf("        offset-%d, reclen-%d\n", slot_entry->offset, slot_entry->reclen);

    } else {
      printf("Slot-%d: is empty!\n", i);
    }
  }

  printf("------------------------------------------------------\n");
  printf("Remained free space: %d bytes\n", page->dirct_end_ptr -page->free_ptr);
  printf("Page is full: ");
  printf("%s", page_entry->is_full ? "True\n\n" : "False\n\n");

  free(slot_entry);
  free(remained_record);
  free(key);
}


uint16_t rpage__scan_full(record_page_entry_t* page_entry, slot_entry_t* target){

  record_page_t* page = page_entry->rpage;
  uint8_t free_page_count = 0;
  uint16_t sid;
  slot_entry_t* temp_slot_entry = malloc(sizeof(slot_entry_t));

  // Scan the whole page and find free slots
  for (uint16_t i=0; i<page->slot_num; i++){
    memcpy(temp_slot_entry,
           &page->buffer[(RECORD_PAGE_BUFFER_SIZE - 1) - (i*PAGE_ID_SIZE + 3)],
           sizeof(slot_entry_t));
    if(temp_slot_entry->reclen == 0){
      free_page_count++;
      if(free_page_count == 1 && target != NULL){
        sid = i;
        target->offset = temp_slot_entry->offset;
      }
    }
  }

  // Handle the full condition
  if(free_page_count == 0 || free_page_count == 1)
    page_entry->is_full = 1;
  else
    page_entry->is_full = 0;

  free(temp_slot_entry);
  return sid;
}
