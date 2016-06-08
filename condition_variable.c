#include "condition_variable.h"

condition_variable_p condition_variable_construct() {
  return calloc(1, sizeof(condition_variable));
}

void condition_variable_init(condition_variable_p the_condition_variable) {
  the_condition_variable->id = 0;
  the_condition_variable->size = 0;
  the_condition_variable->queue = calloc(1, sizeof(thread_lockq));
  // the_condition_variable->queue_head = NULL;
}

condition_variable_p condition_variable_create() {
  condition_variable_p temp = condition_variable_construct();
  condition_variable_init(temp);
}

void condition_variable_enqueue(condition_variable_p the_cond_var, PCB_p the_thread, Mutex_p the_lock) {
  tl_lock_node_p new_node= calloc(1, sizeof(tl_lock_node));


  if (the_cond_var->queue->size == 0) {
    the_cond_var->queue->head =
  }
}

void cond_wait(condition_variable_p the_cond_var, Mutex_p the_lock, PCB_p the_calling_thread, PRIORITYq_p the_queue) {
  Mutex_unlock(the_lock, the_queue); // unlock the lock on behalf of the_thread

}
