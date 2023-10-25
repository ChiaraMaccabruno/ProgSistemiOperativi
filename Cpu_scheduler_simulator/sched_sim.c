#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "fake_os.h"

FakeOS os;

//scheduler SJF
typedef struct {
  int quantum;
  float a;
  int cpu;
} SchedSJFArgs;

ListItem* procmin(FakeOS* os){
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
    if(e->proxburst >= 0 && e->proxburst < min){
      min = e->proxburst;
      mproc = pproc;
    }
    //vado al processo successivo
    pproc = pproc->next;
  }
  FakePCB* elem = (FakePCB*) mproc;
  printf("Il processo con il burst minimo è: %d\n", elem->pid);
  return mproc;
}

int lenghtrunning(FakeOS* os){
  int lunghezza = 0;
  ListItem* elem = os->running.first;
  while(elem!=NULL){
    lunghezza++;
    elem = elem->next;
  }
  return lunghezza;
}

void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;
  //Se non è più presente processo in ready
  if(! os->ready.first)
    return;

  //Effettuare prelazione 
  //Tolgo dalla ready list il mproc

  //Inizia un ciclo che continua finché ci sono processi nella lista dei processi pronti
  while(os->ready.first && lenghtrunning(os) < args->cpu){
    printf("Lunghezza della lista di running: %d\n", lenghtrunning(os));
    printf("Numero di cpu: %d\n", args->cpu);
    //Verifichiamo che ci siano cpu libere
    //Tolgo dalla ready list il mproc, il processo con il burst CPU minimo
    ListItem* elem = List_detach(&os->ready, procmin(os));
    
    //Lo inserisco 
    FakePCB* pcb = (FakePCB*)elem; 
    List_pushFront(&os->running, elem);
    //Il processo affiorante entra nello stato di running
    //pcb = (FakePCB*) List_popFront(&os->ready);
    os->running.first=(ListItem*)pcb;
    printf("okkkkkkkk\n");
    printf(" \n");

    assert(pcb->events.first);
    ProcessEvent* e = (ProcessEvent*)pcb->events.first;
    assert(e->type==CPU);


    if (os->running.first) {
      ListItem* item = os->running.first;
      printf("Lista running: ");
      while(item){
        FakePCB* elem = (FakePCB*) item;
        printf("%d ", elem->pid);
        item = item->next;
      }
      printf("\n");
    }

    
    if(e->duration != 0){
      printf("durata: %d\n", e->duration);
      e->proxburst = (os->cont)*(args->a)+(1-args->a)*e->precburst;
      printf("Prova: %d\n", os->cont);
      printf("proxburst: %d\n", e->proxburst);
      os->cont = 0;
    }

    if (e->duration>args->quantum) {
      ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
      qe->list.prev=qe->list.next=0;
      qe->type=CPU;
      qe->duration=args->quantum;
      e->duration-=args->quantum;
      List_pushFront(&pcb->events, (ListItem*)qe);
    }
  } 
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  
  SchedSJFArgs srr_args2;
  srr_args2.quantum=5;
  srr_args2.a=0.5;
  srr_args2.cpu=atoi(argv[1]);
  os.schedule_args=&srr_args2;
  os.schedule_fn=schedSJF;
  
  for (int i=2; i<argc; ++i){
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
  while(os.running.first
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
