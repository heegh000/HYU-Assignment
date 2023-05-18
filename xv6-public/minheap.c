#include "types.h"
#include "defs.h"
#include "param.h"
#include "minheap.h"

struct node strheap[MAXCAPACITY];

int heapcapa = 0;

//insert (idx, passval) to min heap
//return the idx of root node on success or -1 on failure 
int 
heapinsert(int idx, int passval)
{
  if (heapcapa == MAXCAPACITY - 1)
    return -1;
    
  if(heapcapa != 0) {
    for(int i = 0; i < heapcapa; i++) 
      if(strheap[i].idx == idx)
          return -1;
  }

  int idxtemp;
  int passtemp;
  
  heapcapa++;
  strheap[heapcapa].idx = idx;
  strheap[heapcapa].passval = passval;

  int here = heapcapa;
  int parent = here / 2;

  while (1) {
    if (here == 1 || strheap[here].passval > strheap[parent].passval)
      break;

    idxtemp = strheap[here].idx;
    passtemp = strheap[here].passval;

    strheap[here].idx = strheap[parent].idx;
    strheap[here].passval = strheap[parent].passval;

    strheap[parent].idx = idxtemp;
    strheap[parent].passval = passtemp;

    here = parent;
    parent = here / 2;
  }

  return idx;
}

//delete root node of min heap 
//return the idx of root node on success of -1 on failure
int
heapdelete()
{
  if (heapcapa == 0) {
    return -1;
  }

  int reval = strheap[1].idx;
  int	idxtemp;
  int passtemp;
  int here = 1;
  int child = here * 2;
  
  strheap[1].idx = strheap[heapcapa].idx;
  strheap[1].passval = strheap[heapcapa].passval;

  strheap[heapcapa].idx = -1;
  strheap[heapcapa].passval = -1;

  heapcapa--;

  while (1) {
    if (child + 1 <= heapcapa && strheap[child].passval > strheap[child+1].passval)
      child++;

    if (child > heapcapa || strheap[child].passval > strheap[here].passval)
      break;

    idxtemp = strheap[here].idx;
    passtemp = strheap[here].passval;

    strheap[here].idx = strheap[child].idx;
    strheap[here].passval = strheap[child].passval;

    strheap[child].idx = idxtemp;
    strheap[child].passval = passtemp;

    here = child;
    child = here * 2;
  }

  return reval;
}

//get the idx of root node
int
getidxheap() 
{
  if(heapcapa != 0) 
    return strheap[1].idx;
  return -1;
}

//get thep pass value of root node
int
getpvheap()
{
  if(heapcapa != 0) 
    return strheap[1].passval;
  return -1;
}

void
showheap() 
{	
  int i =0;
  for(i = 1; i < heapcapa+1; i++)
    cprintf("(idx: %d, pv: %d), ", strheap[i].idx, strheap[i].passval);
  cprintf("\n");
}