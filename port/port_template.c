#include <stdint.h>
#include <stdbool.h>
#include <zbus_kernel.h>

void k_mutex_init(struct k_mutex * mutex){}

int k_mutex_lock(struct k_mutex * mutex, uint32_t timeout){return 0;}

int k_mutex_unlock(struct k_mutex * mutex){return 0;}

int k_msgq_put(struct k_msgq *msgq, const void *data, uint32_t timeout){return 0;}

int k_msgq_get(struct k_msgq *msgq, const void *data, uint32_t timeout){return 0;}
