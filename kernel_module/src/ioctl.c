//
// Project 1: Anuraag Motiwale, asmotiwa; Abhishek Singh, aksingh5; Parag Nakhwa, psnkahwa
//
//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////

#include "npheap.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/delay.h>


//
//**************************** Code Written by us Starts here************************************//
//


// Struct for mutex
typedef struct mutex_node {
  struct mutex lock;
  long unsigned int offset;
  struct mutex_node *next;
} mutex_node;

mutex_node *mutex_head = NULL;

// Struct for Linked list node
typedef struct object {
   long unsigned int objectID;
   void * kernelAddress;
   struct object *next;
   long unsigned objectSize;
} object;

extern object *head;



// Check if the offset has been assigned a list node or not.
mutex_node * mappingExist(long unsigned int objectid) {
   mutex_node *ptr = mutex_head;
   //start from the beginning
   while(ptr != NULL) {
      // Checking if object id is same as the offset
      if(objectid == ptr->offset)
         return ptr; // Returning the pointer of allocated memory
      ptr=ptr->next;
   }
   return NULL;
}


// If exist, return the data.
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    struct npheap_cmd *kernelBuffer = (struct npheap_cmd *)kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    mutex_node *tempNode, *mapping;
    copy_from_user(kernelBuffer, user_cmd, sizeof(*user_cmd));

    //printk("in lock %llu ",kernelBuffer->offset/PAGE_SIZE);

    // Check for mapping of offset in the already present linked list
    mapping = mappingExist((*kernelBuffer).offset / PAGE_SIZE);
    if(mapping){
      mutex_lock(&(mapping->lock)); // Lock the mutex struct
    }
    else{
        //create a new mutex_node and initialize it.
        tempNode = (struct mutex_node *)kmalloc(sizeof(struct mutex_node), GFP_KERNEL);
        // Initialize the mutex node
        tempNode->offset = ((*kernelBuffer).offset / PAGE_SIZE);
        mutex_init(&(tempNode->lock));
	      mutex_lock(&(tempNode->lock));
        tempNode->next = mutex_head;
        mutex_head = tempNode;
    }
    kfree(kernelBuffer);
    return 0;
}

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
  mutex_node *ptr = mutex_head;
  struct npheap_cmd *kernelBuffer = (struct npheap_cmd *)kmalloc(sizeof(*user_cmd), GFP_KERNEL);
  copy_from_user(kernelBuffer, user_cmd, sizeof(*user_cmd));
  printk("in unlock %llu ",kernelBuffer->offset/PAGE_SIZE);
  // Linked list traversal to get the correct size of correct offset
  while(ptr != NULL) {
      //Check if current offset has a node assigned in the linked list
      if(((*kernelBuffer).offset / PAGE_SIZE) == ptr->offset){
        mutex_unlock(&(ptr->lock)); // Unlock the mutex struct for current offset
	break;
      }
      ptr=ptr->next;
  }
  kfree(kernelBuffer);
  return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
  object *ptr = head;
  struct npheap_cmd *kernelBuffer = (struct npheap_cmd *)kmalloc(sizeof(*user_cmd), GFP_KERNEL);
  copy_from_user(kernelBuffer, user_cmd, sizeof(*user_cmd));

  // Linked list traversal to get the correct size of correct offset
  while(ptr != NULL) {
      //Check if current offset has a node assigned in the linked list
      if(((*kernelBuffer).offset / PAGE_SIZE) == ptr->objectID){
          if(ptr->objectSize > PAGE_SIZE)
		          return (2 * PAGE_SIZE);
	        return PAGE_SIZE;
      }
      ptr=ptr->next;
  }
  return 0;
}
long npheap_delete(struct npheap_cmd __user *user_cmd)
{
    object *ptr = head, *prev = NULL;
    struct npheap_cmd *kernelBuffer = (struct npheap_cmd *)kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    copy_from_user(kernelBuffer, user_cmd, sizeof(*user_cmd));

    //Traverse the linked list and delete the node at current offset
    while(ptr != NULL) {
        //Check if current offset has a node assigned in the linked list
        if(((*kernelBuffer).offset / PAGE_SIZE) == ptr->objectID){
          if(!prev){
            //prev is empty
            head = ptr->next;
            return 0;
          }
          //prev is not empty
          prev->next = ptr->next;
          kfree(ptr);
          return 0;
        }
        prev = ptr;
        ptr=ptr->next;
    }
    return 0;
}


//**************************** Code End************************************//


long npheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case NPHEAP_IOCTL_LOCK:
        return npheap_lock((void __user *) arg);
    case NPHEAP_IOCTL_UNLOCK:
        return npheap_unlock((void __user *) arg);
    case NPHEAP_IOCTL_GETSIZE:
        return npheap_getsize((void __user *) arg);
    case NPHEAP_IOCTL_DELETE:
        return npheap_delete((void __user *) arg);
    default:
        return -ENOTTY;
    }
}
