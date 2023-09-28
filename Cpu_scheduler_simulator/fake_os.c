#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

void FakeOS_init(FakeOS* os) {
  List_init(&os->running);
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  //Verifico se il pid del processo che caricando sia diverso dal pcb di running

  /*-------------------------------------
  MODIFICAAA!!!!!
  assert( (!os->running || os->running->pid!=p->pid) && "pid taken"); 
  --------------------------------------*/

  ListItem* aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists
  //creo pcb
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);
    break;
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){
  
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started
  //and create all processes starting now
  //aux è lista dei processi attivi
  ListItem* aux=os->processes.first;
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){
      new_process=proc;//creare nuovo processo
    }
    aux=aux->next;
    if (new_process) { //se si deve creare un nuovo processo, si invoca la create process
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
      FakeOS_createProcess(os, new_process);
      free(new_process);
    }
  }

  // scan waiting list, and put in ready all items whose event terminates
  //aux è la lista dei processi in waiting
  aux=os->waiting.first;
  while(aux) {//scorro la lista
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO);
    e->duration--; //decremento durata eventi affioranti
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){//se evento è terminato elimino l'evento stesso
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);
      free(e);
      List_detach(&os->waiting, (ListItem*)pcb);
      if (! pcb->events.first) {//si verifica se evento sia l'ultimo, eventualmente termina processo
        // kill process
        printf("\t\tend process\n");
        free(pcb);
      } else {//altrimenti si prende evento successivo
        //handle next event
        e=(ProcessEvent*) pcb->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) pcb);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb);
          break;
        }
      }
    }
  }

  

  // decrement the duration of running
  // if event over, destroy event
  // and reschedule process
  // if last event, destroy running

  //STESSA COSA DI WAITING E READY

  //Consideriamo l'unico processo in running 
  
  ListItem* aux2 = os->running.first;
  FakePCB* running=(FakePCB*) aux2;
  printf("\trunning pid: %d\n", running?running->pid:-1);
  if (running) {
    //Essendo running una lista di eventi, prendiamo il primo evento che è di tipo CPU
    ProcessEvent* e=(ProcessEvent*) running->events.first;
    assert(e->type==CPU);
    //Decrementiamo durata
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        //se è nulla abbiamo consumato il burst e togliamo l'evento dalla lista
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        if (! running->events.first) {
          //se non ci sono più eventi del processo allora il processo è terminato
          printf("\t\tend process\n");
          free(running); // kill process
          } else {
            //si prende evento successivo  
            e=(ProcessEvent*) running->events.first;
            switch (e->type){
              case CPU:
              printf("\t\tmove to ready\n");
              List_pushBack(&os->ready, (ListItem*) running);
              break;
              case IO:
              printf("\t\tmove to waiting\n");
              List_pushBack(&os->waiting, (ListItem*) running);
              break;
              }
            }
       }
  }


  // call schedule, if defined
  if (os->schedule_fn /*&& ! &os->running*/){
    (*os->schedule_fn)(os, os->schedule_args); 
  }

/*
  // if running not defined and ready queue not empty
  // put the first in ready to run
  if (! os->running && os->ready.first) {
    os->running=(FakePCB*) List_popFront(&os->ready);
  }
*/


  ++os->timer;

}

void FakeOS_destroy(FakeOS* os) {
}
