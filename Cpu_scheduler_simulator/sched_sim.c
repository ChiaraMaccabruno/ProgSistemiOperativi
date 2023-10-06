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
  float a;
  int cpu;
  int freecpu;
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
  printf("Ho trovato il burst minimo: %d\n", min);
  return mproc;
}

void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;
  int cont = args->cpu;

  //Se non è più presente processo in ready
  if(! os->ready.first)
    return;

  //Effettuare prelazione 
  //Tolgo dalla ready list il mproc

  //Inizia un ciclo che continua finché ci sono processi nella lista dei processi pronti
  while(os->ready.first && cont > 0){
    //Verifichiamo che ci siano cpu libere
    //Tolgo dalla ready list il mproc, il processo con il burst CPU minimo
    ListItem* elem = List_detach(&os->ready, procmin(os));

    //Se ci sono CPU libere
    //if(args->freecpu != 0){
      //if(os->running.size < args->cpu){
    
    //Lo inserisco 
    FakePCB* pcb = (FakePCB*)elem; 
    List_pushFront(&os->running, elem);
    //Il processo affiorante entra nello stato di running
    //pcb = (FakePCB*) List_popFront(&os->ready);
    os->running.first=(ListItem*)pcb;
    printf("okkkkkkkk\n");

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

    //Modifica
    if(e->duration != 0){
      printf("durata: %d\n", e->duration);
      e->proxburst = (e->duration)*(args->a)+(1-args->a)*e->precburst;
      printf("proxburst: %d\n", e->proxburst);
    }

    if (e->duration>args->quantum) {
      ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
      qe->list.prev=qe->list.next=0;
      qe->type=CPU;
      qe->duration=args->quantum;
      e->duration-=args->quantum;
      List_pushFront(&pcb->events, (ListItem*)qe);
    }
    
     cont--;
     printf("Il numero di cpu è: %d\n", cont);
    
    //}
 // }else{
   // printf("Cpu tutte occupate\n");
  //}
  /*else{
    //prendo elementi nella lista di running
    ListItem* runningElem = os->running.first;
    FakePCB* runningPCB = (FakePCB*)runningElem;
    
    ProcessEvent* runningEvent = (ProcessEvent*)runningPCB->events.first;

    runningEvent->proxburst = (runningEvent->duration)*(args->a)+(1-args->a)*runningEvent->precburst;

    //prendo nuovo processo in arrivo e ne calcolo 
    FakePCB* pcb = (FakePCB*)elem;
    //assert(pcb->events.first);
    ProcessEvent* e = (ProcessEvent*)pcb->events.first;
    //assert(e->type==CPU);
    e->proxburst = (e->duration)*(args->a)+(1-args->a)*e->precburst;
    
    //Trovo processo in esecuzione con il burst CPU maggiore
    ListItem* maxBurstElem = runningElem;
    FakePCB* maxBurstPCB = runningPCB;
    ProcessEvent* maxBurstEvent = runningEvent;
    maxBurstEvent->proxburst = runningEvent->proxburst;
    printf("-----------------: %d\n", runningEvent->proxburst);
    printf("-----------------: %d\n", maxBurstEvent->proxburst);

    while(runningElem!=NULL){//scorro lista running
      FakePCB* appoggioPCB = (FakePCB*)runningElem;
      printf("runningElem: %d\n", appoggioPCB->pid);
      printf("aaaaaaa\n");
      ProcessEvent* appoggioEvent = (ProcessEvent*)appoggioPCB->events.first;
      printf("bbbbbbb\n");
      appoggioEvent->proxburst = (appoggioEvent->duration)*(args->a)+(1-args->a)*appoggioEvent->precburst;
      printf("ccccc: %d\n", appoggioEvent->proxburst);
      maxBurstEvent->proxburst = (maxBurstEvent->duration)*(args->a)+(1-args->a)*maxBurstEvent->precburst;
      printf("eeeee: %d\n", maxBurstEvent->proxburst);
      if(appoggioEvent->proxburst > maxBurstEvent->proxburst){
        printf("ddddddd\n");
        maxBurstElem = runningElem;
        maxBurstPCB = appoggioPCB;
        maxBurstEvent = appoggioEvent;
      }else{
        printf("ggggggg\n");
      }
      printf("fffffff\n");
      runningElem = runningElem->next;
      
    }

    //se burst CPU del processo appena arrivato è minore del massimo avviene prelazione
    if(e->proxburst < maxBurstEvent->proxburst){
      List_detach(&os->running, maxBurstElem);
      List_pushFront(&os->ready, maxBurstElem);
      List_pushFront(&os->running, elem);
    }else{
      List_pushBack(&os->ready, elem);
    }
  }*/
  }  

};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  //SchedRRArgs srr_args;
  SchedSJFArgs srr_args2;
  //srr_args.quantum=5;
  srr_args2.quantum=5;
  srr_args2.a=0.5;
  srr_args2.cpu=1;
  srr_args2.freecpu=srr_args2.cpu;
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
  while(os.running.first
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
