#include "list_lock.h"

#include <stdio.h>
#include <stdlib.h>

//初始化 list_lock_t 结构
void listInit(list_lock_t* list) {
  list->head=NULL;
  pthread_mutex_init(&list->mutex,NULL);
  pthread_cond_init(&list->cond,NULL);
}

//将生成的数据 value 放入 list 链表中，你可能需要为此分配一个堆区资源。
void producer(list_lock_t* list, DataType value) {
  LNode* newnode=(LNode*)malloc(sizeof(LNode));
  pthread_mutex_lock(&list->mutex);
  newnode->value=value;
  newnode->next=list->head;//把新生成的数据所在的节点插在链表头部
  list->head=newnode;
  pthread_cond_signal(&list->cond);
  pthread_mutex_unlock(&list->mutex);
}

//从 list 链表中消耗一个数据，并释放其占有的资源
void consumer(list_lock_t* list) {
  pthread_mutex_lock(&list->mutex);
  while(list->head==NULL){//生产者(producer)无数据放入链表list时,消费者等待
    pthread_cond_wait(&list->cond,&list->mutex);
   }
  //有数据时
  LNode* a=list->head;//取头部数据
  list->head=list->head->next;//head右移
  free(a);//释放
  pthread_mutex_unlock(&list->mutex);
}

//获取当前 list 中的资源个数，并返回该个数
int getListSize(list_lock_t* list) {
  int cnt=0;
  pthread_mutex_lock(&list->mutex);
  LNode* curr=list->head;
  while(curr!=NULL){
    curr=curr->next;
    cnt++;
  }
  pthread_mutex_unlock(&list->mutex);
  return cnt;
}