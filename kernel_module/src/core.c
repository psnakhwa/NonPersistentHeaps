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
//   Description:
//     Skeleton of NPHeap Pseudo Device
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

extern struct miscdevice npheap_dev;



//
//**************************** Code Written by us Starts here************************************//
//

// struct for every object
typedef struct object {
   long unsigned int objectID;
   void * kernelAddress;
   struct object *next;
   long unsigned objectSize;
} object;

extern object *head;

object *head = NULL;

// Struct for implementing mutex
typedef struct mutex_node {
  struct mutex lock;
  long unsigned int offset;
  struct mutex_node *next;
} mutex_node;

extern mutex_node *mutex_head;


void insertFirst(long unsigned int objectid, void * kerneladdress, long unsigned objectsize) {
   //create a link
   object *link = (struct object*)kmalloc(sizeof(struct object), GFP_KERNEL);
   link->objectID = objectid;
   link->kernelAddress = kerneladdress;
   link->objectSize = objectsize;
   //point it to old first node
   link->next = head;
   //point first to new first node
   head = link;
}

object * mappingExists(long unsigned int objectid) {
   object *ptr = head;
   //start from the beginning
   while(ptr != NULL) {
      // Checking if object id is same as the offset
      if(objectid == ptr->objectID)
         return ptr; // Returning the pointer of allocated memory
      ptr=ptr->next;
   }
   return NULL;
}

int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
    void * getKernelSpace;
    object * existingObject= mappingExists(vma->vm_pgoff);

    if(existingObject!=NULL){
    // printk("mapping exists");
    // Mapping user space memory to kernel space memory
	  if (remap_pfn_range(vma, vma->vm_start,__pa((void *)existingObject->kernelAddress) >> PAGE_SHIFT ,vma->vm_end - vma->vm_start,vma->vm_page_prot))
            return -EAGAIN;
    }else{
        // printk("mapping doesnt exist");
        // Getting kernel space memory for desired size
        getKernelSpace = kmalloc(vma->vm_end - vma->vm_start, GFP_KERNEL);
        // Inserting linked list node for new object
        insertFirst(vma->vm_pgoff,(void *)getKernelSpace,vma->vm_end - vma->vm_start);
        // Mapping user space memory to kernel space memory
    	  if (remap_pfn_range(vma, vma->vm_start,__pa((void *)getKernelSpace) >> PAGE_SHIFT ,vma->vm_end - vma->vm_start,vma->vm_page_prot))
            return -EAGAIN;
    }
    return 0;
}


//**************************** Code End************************************//


int npheap_init(void)
{
    int ret;
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return ret;
}

void npheap_exit(void)
{
    object *ptr1 = head;
    object *temp1;
    mutex_node *ptr2= mutex_head;
    mutex_node *temp2;

    while(ptr1 != NULL) {
      temp1=ptr1;
      ptr1=ptr1->next;
      kfree(temp1->kernelAddress);
      kfree(temp1);
    }

    while(ptr2 != NULL) {
      temp2=ptr2;
      ptr2=ptr2->next;
      kfree(temp2);
    }

    misc_deregister(&npheap_dev);
}
