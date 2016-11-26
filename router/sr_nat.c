
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

  /* initialize bitmap for ports number
  for(int i=0; i<=1000; i++){
    nat->bitmap[i] = 0;
  }
  */
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
    /*update bitmap
    nat->bitmap[current->aux_ext - 2000] = 0;
    */
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
    struct sr_nat_mapping* map; 
    struct sr_nat_mapping* prev = NULL;
    struct sr_nat_mapping* next = NULL;
    for(map = nat-> mappings; map != NULL; map = map->next){
      /* handle imcp timeout*/
      if(map->type == nat_mapping_icmp && difftime(curtime, map->last_updated) >= 60.0){
        /* free mapping in the middle of linked list*/
        if(prev){
          next = map->next;
          prev->next = next;
          /*update bitmap
          nat->bitmap[map->aux_ext -2000] = 0;
          */
          free(map);
        }
        /* free top of the linked list*/
        else{
          next = map->next;
          nat->mapping = next;
          /*update bitmap
          nat->bitmap[map->aux_ext -2000] = 0;
          */
          free(map);
        }
        break;
      }
      /* handle tcp timeout*/
      else if(){



      }

      prev = map;

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
  struct sr_nat_mapping *copy = (struct sr_nat_mapping*) malloc (sizeof (struct sr_nat_mapping));
  while(current != NULL){
    if(current->type==type && current->aux_ext==aux_ext){
      copy->type = current->type;
      copy->ip_int =current->ip_int;
      copy->ip_ext =current->ip_ext;
      copy->aux_int = current->aux_int;
      copy->aux_ext =current->aux_ext;
      copy->last_updated = current->last_updated;
      struct sr_nat_connection *connection;
      /*copy tcp connections*/
      if(current ->conns != NULL) {
        connection = (struct sr_nat_connection*) malloc(sizeof(struct sr_nat_connection));
        memcpy(connection, current->conns, sizeof(struct sr_nat_connection));
        struct sr_nat_connection *next_conn = current->conns->next;
        struct sr_nat_connection *result = connection;
        /*loop over each tcp connection*/
        while(next_conn != NULL){
          struct sr_nat_connection *nested = (struct sr_nat_connection*) malloc(sizeof(struct sr_nat_connection));
          memcpy(nested, next, sizeof(struct sr_nat_connection));
          result-> next = nested;
          result = result-> next;
          next_conn = next_conn->next;
        }
       
      }
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
  struct sr_nat_mapping *copy = (struct sr_nat_mapping*)malloc(sizeof(struct sr_nat_mapping));
  while(current != NULL){
    if(current->type==type && current->aux_int==aux_int && current->ip_int=ip_int){
      copy->type = current->type;
      copy->ip_int =current->ip_int;
      copy->ip_ext =current->ip_ext;
      copy->aux_int = current->aux_int;
      copy->aux_ext =current->aux_ext;
      copy->last_updated = current->last_updated;
      struct sr_nat_connection *connection;
      /*copy tcp connections*/
      if(current->conns != NULL) {
        connection = (struct sr_nat_connection*)malloc(sizeof(struct sr_nat_connection));
        memcpy(connection, current->conns, sizeof(struct sr_nat_connection));
        struct sr_nat_connection *next_conn = current->conns->next;
        struct sr_nat_connection *result = connection;
        /*loop over each tcp connection*/
        while(next_conn != NULL){
          struct sr_nat_connection *nested = (struct sr_nat_connection*) malloc(sizeof(struct sr_nat_connection));
          memcpy(nested, next, sizeof(struct sr_nat_connection));
          result-> next = nested;
          result = result-> next;
          next_conn = next_conn->next;
        } 
      }

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
  struct sr_nat_mapping *mapping = (struct sr_nat_mapping*)malloc(sizeof (struct sr_nat_mapping));
  struct sr_nat_mapping *current = nat->mappings;

  

  /*loop to the end of list and get the largest external port number */
  int port = 1024;
  while(current != NULL){
    if(port < current -> aux_ext){
      port = current->aux_ext;
    }
    current = current -> next;
  }
  /* create a new external port number */
  port = port + 1;

  /* update new mapping data */
  mapping->type = type;
  mapping->ip_int = ip_int;
  mapping->ip_ext = NULL;
  mapping->aux_int = aux_int;
  mapping->aux_ext = port;
  time_t now = time(NULL);
  mapping->last_updated = now;
  /* handle icmp */
  if(type == nat_mapping_icmp){
    mapping->conns = NULL; 
  }

  /* handle tcp */
  else if(type == nat_mapping_tcp){

  }
  
  mapping->next = NULL;
  /* insert new mapping into nat*/
  current = mapping;

  pthread_mutex_unlock(&(nat->lock));
  return mapping;
}




/*check produce an unused ports number staring from 2000 */
int produce_port_num(struct sr_nat *nat){
  int *list = nat->ports;
  for(int i= 0; i < 1000 i++){
    if(nat->bitmap[i] == 0){
      /*update bitmap*/
      nat->bitmap[i] == 1;
      /*return the port number*/
      return i+2000;
    }
  }
  return 0;
}

