#include "co.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <setjmp.h>

#define CO_STACK_SIZE 102400

struct co {
  int id;
  char *name;
  void (*func)(void *);
  void *arg;
  int state;
  ucontext_t uc;
};

static struct node *co_list_head, *co_list_tail;
static struct co *cur_co, *main_co;
static int co_id_base;
static int initmain = 1;

enum {WAIT, READY, DEAD};
enum {NOTMAIN, ISMAIN};

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

static void wrapper_func(void *arg) {
  struct co *co = (struct co*)arg;
  co->func(co->arg);
  co->state = DEAD;
}

static struct co *co_init(const char *name, void (*func)(void *), void *arg, int ismain) {
  struct co *co = malloc(sizeof(struct co));
  if (name) {
    co->name = malloc(strlen(name) + 1);
    strcpy(co->name, name);
  } else {
    co->name = NULL;
  }
  co->func = func;
  co->arg = arg;
  co->id = co_id_base++;

  if (getcontext(&co->uc) != 0)
    handle_error("getcontext");
  
  co->state = READY;
  if (!ismain) {
    co->uc.uc_stack.ss_sp = malloc(CO_STACK_SIZE);
    co->uc.uc_stack.ss_size = CO_STACK_SIZE;
    co->uc.uc_link = &cur_co->uc;
    makecontext(&co->uc, (void (*)(void))wrapper_func, 1, co);
  }

  return co;
}

static void co_deinit(struct co *co) {
  if (!co) return;
  free(co->name);
  free(co->uc.uc_stack.ss_sp);
  free(co);
}

struct node {
  struct co *co;
  struct node *next;
};

static void co_list_add(struct co *co) {
  struct node *newnode = calloc(1, sizeof(struct node));
  newnode->co = co;
  co_list_tail->next = newnode;
  co_list_tail = newnode;
}

static int co_list_index(struct co *co) {
  int ret = 0;
  for (struct node *p = co_list_head->next; p; p = p->next) {
    if (co == p->co) break;
    ret++;
  } 
  return ret;
}

static struct co *co_list_get(int co_list_index) {
  int i = 0;
  for (struct node *p = co_list_head->next; p; p = p->next) {
    if (i == co_list_index) 
      return p->co;
    i++;
  } 
  return NULL;
}

static int co_list_len() {
  int i = 0;
  for (struct node *p = co_list_head->next; p; p = p->next) {
    i++;
  } 
  return i;
}

static void co_list_del(struct co *co) {
  for (struct node *p = co_list_head->next; p; p = p->next) {
    if (p->next->co == co) {
      struct node *deleted = p->next;
      if (deleted == co_list_tail) co_list_tail = p;
      p->next = p->next->next;
      free(deleted); 
      break;
    }
  } 
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  if (initmain) {
    int main();
    main_co = co_init("main", NULL, NULL, ISMAIN);
    co_list_add(main_co);
    cur_co = main_co;
    initmain = 0;
  }
  struct co *co = co_init(name, func, arg, NOTMAIN);
  co_list_add(co);
  return co;
}

static void call(struct co *co) {
  if (cur_co == co) 
    return; 
  struct co *cur_co_bak = cur_co;
  cur_co = co;
  if (swapcontext(&cur_co_bak->uc, &co->uc) != 0) 
    handle_error("swapcontext");
}

static void scheduler() {
  struct co *to_be_called;
  int nr_coroutine = co_list_len();
  while (1) {
    int random_co_list_index = rand() % nr_coroutine;
    struct co *random_co = co_list_get(random_co_list_index);
    if (random_co->state == READY) {
      to_be_called = random_co;
      break;
    }
  }
  call(to_be_called);
}

void co_wait(struct co *co) {
  if (!cur_co) {
    assert(0 && "unexpected error!");
  }
  struct co *cur_co_bak = cur_co;
  cur_co->state = WAIT;
  co->uc.uc_link = &cur_co->uc;
  while (co->state != DEAD) {
    scheduler();
  }
  co_list_del(co);
  co_deinit(co);
  cur_co = cur_co_bak;
}

void co_yield() {
  if (!cur_co) {
    assert(0 && "unexpected error!");
  }
  cur_co->state = READY;
  scheduler();
}

static void __attribute__((constructor)) init() {
  srand(time(NULL));
  co_list_head = calloc(1, sizeof(struct node));
  co_list_tail = co_list_head;
}