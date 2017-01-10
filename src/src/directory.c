#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/relation.h"


data_entry_t dpage__insert_record(directory_page_t* cur_dirct,
                                  uint16_t record_size,
                                  key_t key_type,
                                  index_t key,
                                  const char* remained_record)
{
  // Create a key-rid pair
  data_entry_t key_rid = {-1, -1};

  if(cur_dirct == NULL)
    return key_rid;

  if(cur_dirct->is_full && cur_dirct->next!=NULL) {
    return dpage__insert_record(cur_dirct->next, record_size, key_type, key, remained_record);

  } else if (cur_dirct->is_full && cur_dirct->next==NULL) {
    cur_dirct->next = calloc(1, sizeof(directory_page_t));
    cur_dirct->next->pid_base = cur_dirct->pid_base + DIRECTORY_ENTRY_NUM;

    return dpage__insert_record(cur_dirct->next, record_size, key_type, key, remained_record);

  } else {


    // Set the key of the key-rid pair
    if(key_type == TYPE_INT)
      key_rid.key.i = key.i;
    else
      strncpy(key_rid.key.str, key.str, 10);

    // Find a suitable page to insert
    for(int i=0; i<DIRECTORY_ENTRY_NUM; i++) {
      if(!cur_dirct->entry[i].is_full) {

        // If the page is a new page
        record_page_entry_t* cur_entry = &cur_dirct->entry[i];
        if(cur_entry->rpage == NULL) {
          cur_entry->pid = i + cur_dirct->pid_base;
          cur_entry->rpage = calloc(1, sizeof(record_page_t));
          cur_entry->rpage->dirct_end_ptr = RECORD_PAGE_BUFFER_SIZE-1;
        }

        // Set the pid and insert the record
        key_rid.pid = cur_entry->pid;
        key_rid.slot_num =  rpage__insert_record(&cur_dirct->entry[i], record_size, key_type, key, remained_record);

        // Check if the page is full after we insert a record
        if(cur_dirct->entry[i].is_full) {
          if(++cur_dirct->fullpage_num == DIRECTORY_ENTRY_NUM)
           cur_dirct->is_full = 1;
        }

        break;
      }
    }

    return key_rid;
  }
}


void dpage__find_record(directory_page_t* cur_dirct,
                        uint16_t record_size,
                        key_t key_type,
                        uint16_t pid,
                        uint16_t slot_number,
                        action_t action)
{
  int dirct_id = pid / DIRECTORY_ENTRY_NUM;
  int dirct_entry_id = pid % DIRECTORY_ENTRY_NUM;

  while(cur_dirct!=NULL && cur_dirct->pid_base != dirct_id*DIRECTORY_ENTRY_NUM)
    cur_dirct = cur_dirct->next;

  if(cur_dirct==NULL){
    fprintf(stderr, "Warning: Cannot find the record with <pid, slot#>: <%d, %d>\n", pid, slot_number);
    return;
  } else {
    if(action == ACTION_PRINT) printf("Pid: %d\n", pid);
    rpage__find_record(&cur_dirct->entry[dirct_entry_id], slot_number, record_size, key_type, action);
  }
}


// Insert when the page is NOT full, and return sid
uint16_t rpage__insert_record(record_page_entry_t* page_entry,
                              uint16_t record_size,
                              key_t key_type,
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
    memcpy(&page->buffer[RECORD_PAGE_BUFFER_SIZE - PAGE_ID_SIZE*(sid + 1)],
           slot_entry,
           sizeof(slot_entry_t));
  }


  // Insert the record to the page
  uint16_t key_size = (key_type == TYPE_INT ? 4 : 10);

  memcpy(&page->buffer[slot_entry->offset], (const void *)&key, sizeof(uint8_t)*key_size);
  memcpy(&page->buffer[slot_entry->offset+key_size],
         remained_record,
         sizeof(uint8_t)*(record_size - key_size));

  free(slot_entry);
  return sid;
}


void rpage__find_record(record_page_entry_t* page_entry,
                        uint16_t sid,
                        uint16_t record_size,
                        key_t key_type,
                        action_t action)
{
  if(page_entry==NULL)
    return;

  record_page_t* page = page_entry->rpage;
  slot_entry_t* slot_entry = calloc(1, sizeof(slot_entry_t));

  index_t key;
  int key_size = (key_type == TYPE_INT ? 4 : 10);
  char* remained_record = calloc((record_size-key_size), sizeof(uint8_t));

  memcpy(slot_entry,
         &page->buffer[RECORD_PAGE_BUFFER_SIZE - PAGE_ID_SIZE*(sid + 1)],
         sizeof(slot_entry_t));

  // PRINT the result after we find it
  if(action == ACTION_PRINT) {
    if(slot_entry->reclen != 0){
      // Get the record from the record page
      memcpy(&key, &page->buffer[slot_entry->offset], sizeof(uint8_t)*key_size);
      memcpy(remained_record,
             &page->buffer[slot_entry->offset + key_size],
             sizeof(uint8_t)*(record_size-key_size));

      // Print the whole record
      if(key_type == TYPE_INT){
        printf("Slot-%d: key: %d, record: %s\n", sid, key.i, remained_record);
      } else {
        printf("Slot-%d: key: %s, record: %s\n", sid, key.str, remained_record);
      }
      printf("        offset-%d, reclen-%d\n", slot_entry->offset, slot_entry->reclen);

    } else {
      fprintf(stderr, "Warning: Slot-%d is empty!\n", sid);
    }

  // DELETE the result after we find it
  } else {
    memset(&page->buffer[slot_entry->offset], 0, record_size);
    slot_entry->reclen = 0;
    memcpy(&page->buffer[RECORD_PAGE_BUFFER_SIZE - PAGE_ID_SIZE*(sid + 1)],
           slot_entry,
           sizeof(slot_entry_t));
  }

  free(slot_entry);
  free(remained_record);
}


void rpage__show_page(record_page_entry_t* page_entry, uint16_t record_size, key_t key_type)
{
  if(page_entry==NULL)
    return;

  record_page_t* page = page_entry->rpage;

  printf("======================================================\n");
  printf("Pid: %d\n", page_entry->pid);
  printf("Total slot number: %d\n", page->slot_num);
  printf("------------------------------------------------------\n");

  for(uint16_t i=0; i<page->slot_num; i++)
    rpage__find_record(page_entry, i, record_size, key_type, ACTION_PRINT);

  printf("------------------------------------------------------\n");
  printf("Remained free space: %d bytes\n", page->dirct_end_ptr -page->free_ptr);
  printf("Page is full: ");
  printf("%s", page_entry->is_full ? "True\n\n" : "False\n\n");
}


uint16_t rpage__scan_full(record_page_entry_t* page_entry, slot_entry_t* target)
{

  record_page_t* page = page_entry->rpage;
  uint8_t free_page_count = 0;
  uint16_t sid;
  slot_entry_t* temp_slot_entry = malloc(sizeof(slot_entry_t));

  // Scan the whole page and find free slots
  for (uint16_t i=0; i<page->slot_num; i++){
    memcpy(temp_slot_entry,
           &page->buffer[RECORD_PAGE_BUFFER_SIZE - PAGE_ID_SIZE*(i + 1)],
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
  if(free_page_count == 0 || (free_page_count == 1 && target!=NULL))
    page_entry->is_full = 1;
  else
    page_entry->is_full = 0;

  free(temp_slot_entry);
  return sid;
}
