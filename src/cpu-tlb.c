/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
// #ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */

#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
static pthread_mutex_t tlb_lock = PTHREAD_MUTEX_INITIALIZER;
int tlb_change_all_page_tables_of(struct pcb_t *proc, struct memphy_struct *mp)
{
  /* TODO update all page table directory info
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct *mp)
{
  /* TODO flush tlb cached*/
  if (!proc || !mp)
    return -1;
  for (int i = 0; i < TLB_SIZE; i++)
  {
    tlb_table[i].page = -1;
    tlb_table[i].frame = -1;
    tlb_table[i].valid = 0;
    tlb_table[i].last_used = 0;
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;
  
  /* By default using vmaid = 0 */ 
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  
  if (val < 0)
    return -1;
  pthread_mutex_lock(&tlb_lock);
  int pgn = PAGING_PGN(addr);

  int pgnum = PAGING_PAGE_ALIGNSZ(size) / PAGING_PAGESZ;
  for (int i = 0; i < pgnum; i++)
  {
    uint32_t pte = proc->mm->pgd[pgn + i];
    int fpn = PAGING_FPN(pte);
    tlb_cache_write_123(proc->pid, pgn + i, fpn);
  }
  pthread_mutex_unlock(&tlb_lock);
  return 0;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{

  __free(proc, 0, reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  pthread_mutex_lock(&tlb_lock);
  int start = proc->mm->symrgtbl[reg_index].rg_start;
  int end = proc->mm->symrgtbl[reg_index].rg_end;
  int size = end = start;
  int pgn = PAGING_PGN(start);
  int pgnum = PAGING_PAGE_ALIGNSZ(size) / PAGING_PAGESZ;
  for (int i = 0; i < pgnum; i++)
  {
    int t = check_in_tlb(proc->pid, pgn + i);
    if (t >= 0)
    {
      tlb_table[i].page = -1;
      tlb_table[i].frame = -1;
      tlb_table[i].last_used = 0;
      tlb_table[i].valid = 0;
    }
  }
  pthread_mutex_unlock(&tlb_lock);
  return 0;
}

/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t *proc, uint32_t source,
            uint32_t offset, uint32_t destination)
{
  BYTE data;
  int frmnum = -1;
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/

  pthread_mutex_lock(&tlb_lock);
  struct vm_rg_struct *currg = get_symrg_byid(proc->mm, source);
  int addr = currg->rg_start + offset;
  int pgn = PAGING_PGN(addr);
  tlb_cache_read_123(proc->pid, pgn, &frmnum);
  print_check_tlb_table();
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n",
           source, offset);
  else
    printf("TLB miss at read region=%d offset=%d\n",
           source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  pthread_mutex_unlock(&tlb_lock);
  int val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t)data;

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t *proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  int frmnum = -1;
  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/
  pthread_mutex_lock(&tlb_lock);
  struct vm_rg_struct *currg = get_symrg_byid(proc->mm, destination);
  int addr = currg->rg_start + offset;
  int pgn = PAGING_PGN(addr);
  tlb_cache_read_123(proc->pid, pgn, &frmnum);
  print_check_tlb_table();
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
           destination, offset, data);
  else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
           destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  pthread_mutex_unlock(&tlb_lock);
  val = __write(proc, 0, destination, offset, data);

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

// #endif
