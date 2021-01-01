#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "paging.h"
#include "fs.h"


static pte_t * walkpgdir(pde_t *pgdir, const void *va, int alloc);
int deallocuvmXV7(pde_t *pgdir, uint oldsz, uint newsz);
static int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);

struct proc*
myprocXV7(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

/* Allocate eight consecutive disk blocks.
 * Save the content of the physical page in the pte
 * to the disk blocks and save the block-id into the
 * pte.
 */
void
swap_page_from_pte(pte_t *pte)
{
	//************xv7*************

  uint physicalAddress=PTE_ADDR(*pte);          //PTE_ADDR returns address in pte
  if(physicalAddress==0)
    cprintf("physicalAddress address is zero\n");
  uint diskPage=balloc_page(ROOTDEV);

//  char *vaSwapPage=kalloc();                 //virtual address of physical page which needs to be swapped to disk
  // char vaSwapPage[4096]="";

  // memmove(vaSwapPage,(char*)P2V(physicalAddress),PGSIZE); //copy va to vaSwapPage : copied from 2071: copyuvm()

  write_page_to_disk(ROOTDEV,(char*)P2V(physicalAddress),diskPage);    //write this page to disk
  //*pte &= ~PTE_P;                 //clear present bit as the page has been swapped to the disk
  //uint blockid=diskPage<<12;

  /*
    Store block id and swapped flag in the pte entry whose page was swapped to the disk
    So, when next time this pte is dereferenced, we know that the page has been swapped to
    the disk and we can bring this page again to memory
  */
  *pte = (*pte & 0x000000);     //make pte = null;
  *pte = (diskPage << 12)| PTE_SWAPPED;
  *pte = *pte & ~PTE_P;
//  *pte = *pte | PTE_A;

  /*PTE_SWAPPED (0x200) : created by me , needs to be set when swapping a page. */

  /*WHEN PAGE TABLE ENTRIES ARE MODIFIED, THE HARDWARE STILL USES CACHED ENTRIES IN TLB,
    SO WE NEED TO INVALIDATE TLB ENTRY USING EITHER invlpg INSTRUCTION OR
    lcr3
  */

  kfree(P2V(physicalAddress));
  cprintf("\nReturning from swap page from pte\n");

}

/* Select a victim and swap the contents to the disk.
 */
int
swap_page(pde_t *pgdir)
{
  pte_t* pte=select_a_victim(pgdir);         //returns *pte
  if(pte==0){                                     //If this is true, victim is not found in 1st attempt. Inside this function
    cprintf("No victim found in 1st attempt. Clearing access bits.");
    clearaccessbit(pgdir);                        //Accessbits are cleared,

    cprintf("Finding victim again, after clearing access bits of 10%% pages.");
    pte=select_a_victim(pgdir);                   //then victim is selected again. Victim is found this time.

    if(pte!=0) cprintf("victim found");
    else cprintf("Not found even in second attempt." );
  }
  else{                                           //This else is true, then victim is found in first attempt.
    cprintf("Victim found in 1st attempt.");
  }

  swap_page_from_pte(pte);  //swap victim page to disk
  lcr3(V2P(pgdir));         //This operation ensures that the older TLB entries are flushed
	//panic("swap_page is not implemented");
	return 1;
}

/* Map a physical page to the virtual address addr.
 * If the page table entry points to a swapped block
 * restore the content of the page from the swapped
 * block and free the swapped block.
 */

//***************xv7*****************
/*
i) kalloc a physical page
ii) map physical page to virtual page (addr)
iii) Set the access bit of the page (last 12 bits are same in physical and virtual page),
			so they share the access bit
*/
void
map_address(pde_t *pgdir, uint addr)
{
	struct proc *curproc = myprocXV7();
	// cprintf(curproc->name);
	uint cursz= curproc->sz;
	uint a= PGROUNDDOWN(rcr2());			//rounds the address to a multiple of page size (PGSIZE)

  pte_t *pte=walkpgdir(pgdir, (char*)a, 0);
  int blockid=-1;                 //disk id where the page was swapped

	char *mem=kalloc();    //allocate a physical page

  if(mem==0){
		//************xv7 swapping yha krni hai**************
    swap_page(pgdir);
    mem=kalloc();             //now a physical page has been swapped to disk and free, so this time we will get physical page for sure.
    cprintf("kalloc success\n");
    // panic("allocuvm out of memory xv7 in mem==0/n");
		// deallocuvmXV7(pgdir,cursz+PGSIZE, cursz);
	}

  if(pte!=0){
    if(*pte & PTE_SWAPPED){
      //cprintf("\npage was swapped\n");
      blockid=getswappedblk(pgdir,a);      //disk id where the page was swapped
      read_page_from_disk(ROOTDEV, mem, blockid);

      *pte=V2P(mem) | PTE_W | PTE_U | PTE_P;
      *pte &= ~PTE_SWAPPED;
      lcr3(V2P(pgdir));
      bfree_page(ROOTDEV,blockid);
    }
    else{
      memset(mem,0,PGSIZE);
    	if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_P | PTE_W | PTE_U )<0){
    		panic("allocuvm out of memory xv7 in mappages/n");
    		deallocuvmXV7(pgdir,cursz+PGSIZE, cursz);
    		kfree(mem);
    	}
    	else{
    		cprintf("mappages working");
    	}
    }
  }
  // panic("map_address is not implemented");
}

/* page fault handler */
void
handle_pgfault()
{
	unsigned addr;
	struct proc *curproc = myprocXV7();

	asm volatile ("movl %%cr2, %0 \n\t" : "=r" (addr));
	addr &= ~0xfff;

	// cprintf("hello  1 ");
	// cprintf((char*) addr);

	map_address(curproc->pgdir, addr);
}

//********xv7***********
// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
// If the page was swapped free the corresponding disk block.
int
deallocuvmXV7(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if((*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}
// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    // if(*pte & PTE_P)
    //   panic("remap in mappages in paging.c");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}
