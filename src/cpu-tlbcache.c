/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access
 * and runs at high speed
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#define init_tlbcache(mp, sz, ...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

// MOI ADD VAO
static int time_access = 0;
int TLB_SIZE;
tlb_entry* tlb_table;
void init_tlb_table(int tlbsz)
{
   TLB_SIZE= tlbsz/PAGING_PAGESZ;
   tlb_table = (tlb_entry*) malloc(TLB_SIZE * sizeof(tlb_entry));
    if (tlb_table == NULL) {
        perror("Failed to allocate memory for TLB table");
        exit(EXIT_FAILURE);
    }
   for (int i = 0; i < TLB_SIZE; ++i)
   {
      tlb_table[i].page = -1;
      tlb_table[i].frame = -1;
      tlb_table[i].valid = 0;
      tlb_table[i].pid = -1;
      tlb_table[i].last_used = 0;
   }
}

int check_in_tlb(int pid,int pgn){
   for(int i=0;i<TLB_SIZE;i++){
      if(tlb_table[i].valid == 1 && tlb_table[i].page == pgn && tlb_table[i].pid == pid ){
         return i;
      }
   }
   return -1;
}
/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   return 0;
}

int tlb_cache_read_123(int pid, int pgn, int *fpn)
{
   for (int i = 0; i < TLB_SIZE; i++)
   {
      if (tlb_table[i].valid == 1 && tlb_table[i].page == pgn && tlb_table[i].pid == pid)
      {
         *fpn = tlb_table[i].frame;
         tlb_table[i].last_used = ++time_access;
         return i; // return ve vi tri doc dc
      }
   }
   return -1;
}
/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
   /* TODO: the identify info is mapped to
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   return 0;
}
int tlb_cache_write_123(int pid, int pgn, int fpn)
{
   int k=check_in_tlb(pid,pgn);
   if(k!=-1){
      tlb_table[k].last_used= ++time_access;
      return 0;
   }
   // LRU
   int lru_id = 0;
   int min_time_access = tlb_table[0].last_used;
   for (int i = 1; i < TLB_SIZE; i++)
   {
      if (tlb_table[i].last_used < min_time_access)
      {
         lru_id = i;
         min_time_access = tlb_table[i].last_used;
      }
   }
   tlb_table[lru_id].page = pgn;
   tlb_table[lru_id].frame = fpn;
   tlb_table[lru_id].pid = pid;
   tlb_table[lru_id].valid = 1;
   tlb_table[lru_id].last_used = ++time_access;
   return 0;
}

void print_check_tlb_table(){
   printf("Check tlb table :\n");
   for(int i=0;i<TLB_SIZE;i++){
      if(tlb_table[i].valid){
         printf("PID: %u; PGNUM: %d; FRMNUM: %d; LRU: %d\n",tlb_table[i].pid,tlb_table[i].page,tlb_table[i].frame,tlb_table[i].last_used);
      }
   }
}
/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL)
      return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}

/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct *mp, int addr, BYTE data)
{
   if (mp == NULL)
      return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */

int TLBMEMPHY_dump(struct memphy_struct *mp)
{
   /*TODO dump memphy contnt mp->storage
    *     for tracing the memory content
    */
   printf("Tracing TLBMEMPHY content - (index,content): ");
   if (mp && mp->storage)
   {
      for (int i = 0; i < mp->maxsz; i++)
      {
         if (mp->storage[i] != (char)0)
            printf("(%d ; %d) ", i, mp->storage[i]);
      }
      printf("\n");
   }
   return 0;
}

/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
   mp->maxsz = max_size;

   mp->rdmflg = 1;

   return 0;
}

// #endif
