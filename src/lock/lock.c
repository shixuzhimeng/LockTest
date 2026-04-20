#include "lock.h"

#include <stdio.h>

//初始化 lock_t 结构
void amountInit(lock_t* Account) {
  Account->amount=0;
  // 初始化互斥锁，第二个参数为 NULL 表示使用默认属性
  pthread_mutex_init(&Account->mutex,NULL);
}

//用于进行资金收入操作，金额为 amount
void Income(lock_t* Account, int amount) {
  pthread_mutex_lock(&Account->mutex);//加锁
  Account->amount+=amount;
  pthread_mutex_unlock(&Account->mutex);//解锁
}
//用于进行资金支出操作
void Expend(lock_t* Account, int amount) {
  pthread_mutex_lock(&Account->mutex);
  Account->amount-=amount;
  pthread_mutex_unlock(&Account->mutex);
}