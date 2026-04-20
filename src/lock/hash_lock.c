#include "hash_lock.h"

#include <stdio.h>
#include <stdlib.h>

//初始化 hash_lock_t 结构
void hashInit(hash_lock_t* bucket) {
  for(int i=0;i<HASHNUM;i++){
    bucket->table[i].head=NULL;
    pthread_mutex_init(&bucket->table[i].mutex,NULL);
  }
}
//通过 key 值来获取对应的 value
int getValue(hash_lock_t* bucket, int key) {
  //计算键所在的桶索引
  int index=HASH(key);
  int Value=-1;
  //锁定对应的桶
  pthread_mutex_lock(&bucket->table[index].mutex);
  Hnode* curr=bucket->table[index].head;
  //在桶的链表中查找
  while(curr!=NULL){
    if(curr->key==key){
      Value=curr->value;
      pthread_mutex_unlock(&bucket->table[index].mutex);
      return Value;
    }
    curr=curr->next;
  }
  //找到后解锁并返回
  pthread_mutex_unlock(&bucket->table[index].mutex);
  return Value;
}
//向哈希桶中添加一个节点，通过key来索引对应的哈希序列,如果键已经存在，则覆盖现有值
void insert(hash_lock_t* bucket, int key, int value) {
  int index=HASH(key);
  pthread_mutex_lock(&bucket->table[index].mutex);
  Hnode* curr=bucket->table[index].head;
  while(curr!=NULL){
    if(curr->key==key){
      curr->value=value;
      pthread_mutex_unlock(&bucket->table[index].mutex);
      return ;
    }
    curr=curr->next;
  }
  //不存在时创建新的节点
  Hnode* newnode=(Hnode*)malloc(sizeof(Hnode));
  newnode->key=key;
  newnode->value=value;
  newnode->next=bucket->table[index].head;
  bucket->table[index].head=newnode;
  pthread_mutex_unlock(&bucket->table[index].mutex);
}
//重新设置一个节点的key,并将其移动到对应的序列，如果键不存在，则返回-1,成功返回0
int setKey(hash_lock_t* bucket, int key, int new_key) {
  if(key==new_key) return 0;
  int index_old=HASH(key);
  int index_new=HASH(new_key);
  if(index_old==index_new){
    pthread_mutex_lock(&bucket->table[index_old].mutex);
    Hnode* curr=bucket->table[index_old].head;
    while(curr!=NULL){

      if(curr->key==key){
        curr->key=new_key;
        pthread_mutex_unlock(&bucket->table[index_old].mutex);
        return 0;
      }
      curr=curr->next;
    }
    pthread_mutex_unlock(&bucket->table[index_old].mutex);
    return -1;
  }
  
  int first=(index_old<index_new)?index_old:index_new;
  int second=(index_old<index_new)?index_new:index_old;
  pthread_mutex_lock(&bucket->table[first].mutex);
  pthread_mutex_lock(&bucket->table[second].mutex);//按顺序加锁,防止死锁
  Hnode* curr=bucket->table[index_old].head;
  Hnode* prev=NULL;
  while(curr!=NULL){
    if(curr->key==key){//在旧链表找到目标
      if(prev==NULL){
        bucket->table[index_old].head=curr->next;//如果是头节点,桶的头指针指向下一个
      }else{
        prev->next=curr->next;//如果是中间节点,前一个直接指向后一个,跳过curr
      }
      Hnode* newnode=(Hnode*)malloc(sizeof(Hnode));
      newnode->key=new_key;
      newnode->value=curr->value;
      newnode->next=bucket->table[index_new].head;
      bucket->table[index_new].head=newnode;
      
      free(curr);
      pthread_mutex_unlock(&bucket->table[second].mutex);
      pthread_mutex_unlock(&bucket->table[first].mutex);
      return 0;
    }
    prev=curr;
    curr=curr->next;
  }
  pthread_mutex_unlock(&bucket->table[first].mutex);
  pthread_mutex_unlock(&bucket->table[second].mutex);
  return -1;
}