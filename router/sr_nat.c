
#include <signal.h>
#include <assert.h>
#include "sr_nat.h"
#include <unistd.h>

int sr_nat_init(struct sr_nat *nat) { /* Initializes the nat */

  assert(nat);

  /* Acquire mutex lock */
  pthread_mutexattr_init(&(nat->attr));
  pthread_mutexattr_settype(&(nat->attr), PTHREAD_MUTEX_RECURSIVE);
  int success = pthread_mutex_init(&(nat->lock), &(nat->attr));

  /* Initialize timeout thread */

  pthread_attr_init(&(nat->thread_attr));
  pthread_attr_setdetachstate(&(nat->thread_attr), PTHREAD_CREATE_JOINABLE);
  pthread_attr_setscope(&(nat->thread_attr), PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setscope(&(nat->thread_attr), PTHREAD_SCOPE_SYSTEM);
  pthread_create(&(nat->thread), &(nat->thread_attr), sr_nat_timeout, nat);
  /* CAREFUL MODIFYING CODE ABOVE THIS LINE! */
  nat->mappings = NULL;
  /* Initialize any variables here */

  return success;
}


int sr_nat_destroy(struct sr_nat *nat) {  /* Destroys the nat (free memory) */
  pthread_mutex_lock(&(nat->lock)); 
  /* free nat memory here */
  struct sr_nat_mapping* current = nat->mappings;
  while(current != NULL){
    struct sr_nat* next = current->next;
    /* free connections in the sr_nat_mapping*/
    struct sr_nat_connection* connection = current->conns;
    while(connection != NULL){
      struct sr_nat_connection* next_conns = connection -> next;
      free(connection);
      connection = next_conns;
    }
    /*free current sr_nat_mapping*/
    free(current);
    current = next;
  }
  pthread_kill(nat->thread, SIGKILL);
  return pthread_mutex_destroy(&(nat->lock)) && pthread_mutexattr_destroy(&(nat->attr));

}


void *sr_nat_timeout(void *nat_ptr) {  /* Periodic Timout handling */
  struct sr_nat *nat = (struct sr_nat *)nat_ptr;
  while (1) {
    sleep(1.0);
    pthread_mutex_lock(&(nat->lock));
    time_t curtime = time(NULL);
    /* handle periodic tasks here */
    struct sr_nat_mapping* map = nat -> mappings; 
    struct sr_nat_mapping* top = map;
    struct sr_nat_mapping* next = NULL;
    while(map -> != NULL){
      /*handle imcp*/
      struct sr_nat_mapping* next =  map->next;
      if(map -> type == nat_mapping_icmp){
        if(difftime(curtime, map->last_updated) >= 60){
           next = map -> next;
           

          /* free imcp mappings */


        }
          
      }
    }
    pthread_mutex_unlock(&(nat->lock));
  }
  return NULL;
}


/* Get the mapping associated with given external port.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_external(struct sr_nat *nat,
    uint16_t aux_ext, sr_nat_mapping_type type ) {

  pthread_mutex_lock(&(nat->lock));

  /* handle lookup here, malloc and assign to copy */
  struct sr_nat_mapping *current = nat->mappings;
  struct sr_nat_mapping *copy = (struct sr_nat_mapping*) malloc (sizeof struct sr_nat_mapping);
  while(current != NULL){
    if(current->type==type && current->aux_ext==aux_ext){
      copy->type = current->type;
      copy->ip_int =current->ip_int;
      copy->ip_ext =current->ip_ext;
      copy->aux_int = current->aux_int;
      copy->aux_ext =current->aux_ext;
      copy->last_updated = current->last_updated;
      struct sr_nat_connection *connection = (struct sr_nat_connection*) malloc(sizeof struct sr_nat_connection);
      connection = current -> conns;
      copy->conns = connection;
      copy->next = NULL;
      break;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&(nat->lock));
  return copy;
}

/* Get the mapping associated with given internal (ip, port) pair.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_internal(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type ) {

  pthread_mutex_lock(&(nat->lock));

  /* handle lookup here, malloc and assign to copy. */
  struct sr_nat_mapping *current = nat->mappings;
  struct sr_nat_mapping *copy = (struct sr_nat_mapping*) malloc (sizeof(struct sr_nat_mapping));
  while(current != NULL){
    if(current->type==type && current->aux_int==aux_int && current->ip_int=ip_int){
      copy->type = current->type;
      copy->ip_int =current->ip_int;
      copy->ip_ext =current->ip_ext;
      copy->aux_int = current->aux_int;
      copy->aux_ext =current->aux_ext;
      copy->last_updated = current->last_updated;
      struct sr_nat_connection *connection=(struct sr_nat_connection*)malloc(sizeof(struct sr_nat_connection));
      connection = current -> conns;
      copy->conns = connection;
      copy->next = NULL;
      break;
    }
    current = current->next;
  }
  pthread_mutex_unlock(&(nat->lock));
  return copy;
}

/* Insert a new mapping into the nat's mapping table.
   Actually returns a copy to the new mapping, for thread safety.
 */
struct sr_nat_mapping *sr_nat_insert_mapping(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type ) {
  
  pthread_mutex_lock(&(nat->lock));

  /* handle insert here, create a mapping, and then return a copy of it */
  struct sr_nat_mapping *mapping = (struct sr_nat_mapping*) malloc (sizeof (struct sr_nat_mapping));
  struct sr_nat_mapping *current = nat->mappings;

  /*loop to the end of list and get the largest external port number */
  int port = 1024;
  while(current != NULL){
    if(port < current -> aux_ext){
      port = current -> aux_ext;
    }
    current = current -> next;
  }
  /* create a new external prot number*/
  port = port + 1;
  /*make sure port is unique*/
  while(port == aux_int){
    prot++;
  }
  /* update new mapping data */
  mapping -> type = type;
  mapping -> ip_int = ip_int;
  mapping -> ip_ext = NULL;
  mapping -> aux_int = aux_int;
  mapping -> aux_ext = port;
  time_t now = time(NULL);
  mapping -> last_updated = now;
  /* handle icmp */
  if(type == nat_mapping_icmp){
    mapping -> conns = NULL;
  }
  /* handle tcp */
  else{

  }
  
  mapping - > next = NULL;
  /* insert new mapping into nat*/
  current = mapping;

  

  pthread_mutex_unlock(&(nat->lock));
  return mapping;
}