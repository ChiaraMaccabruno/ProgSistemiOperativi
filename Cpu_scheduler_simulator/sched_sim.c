#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "fake_os.h"

FakeOS os;

/*
typedef struct {
  int quantum;
} SchedRRArgs;
*/

//scheduler SJF
typedef struct {
  int quantum;
  int a;
} SchedSJFArgs;

/*
void schedRR(FakeOS* os, void* args_){
  SchedRRArgs* args=(SchedRRArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;

  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  os->running=pcb;
  
  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
};
*/

ListItem* procmin(FakeOS* os, void* args_){
  if(! os->ready.first)
    return NULL;

  //Se presente, considero processo affiorante nella lista ready
  ListItem* pproc = os->ready.first;
  //Con SJF entra in running il processo che si trova in ready con il burst più piccolo
  //Troviamo il processo con il burst minore
  int min = INT_MAX;
  ListItem* mproc;

  while(pproc != NULL){
    //considero pcb del processo
    FakePCB* pcb = (FakePCB*) pproc;
    //considero primo evento del processo
    ProcessEvent* e = (ProcessEvent*)pcb->events.first;

    //cerco il CPU burst minimo
    if(e->duration < min){
      min = e->duration;
      mproc = pproc;
    }
    //vado al processo successivo
    pproc = pproc->next;
  }
  return mproc;
  FakePCB* elem = (FakePCB*) mproc;
  printf("Il processo con il burst minimo è: %d\n", elem->pid);
  printf("Ho trovato il burst minimo: %d\n", min);
}

void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  //Se non è più presente processo in ready
  if(! os->ready.first)
    return;

  //Effettuare prelazione 

  

};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  //SchedRRArgs srr_args;
  SchedSJFArgs srr_args2;
  //srr_args.quantum=5;
  srr_args2.quantum=5;
  srr_args2.a=0;
  //os.schedule_args=&srr_args;
  os.schedule_args=&srr_args2;
  //os.schedule_fn=schedRR;
  os.schedule_fn=schedSJF;
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
