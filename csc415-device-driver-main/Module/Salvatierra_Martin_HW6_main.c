/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Name:Martin Salvatierra
* Student ID:920000611
* GitHub UserID:Salvatim007
* Project: Assignment 6 – Device Driver
*
* File: Salvatierra_Martin_HW2_main.c
*
* Description: Assignment 6 is about making a device driver that stores user 
*              A user String and ecrypts the data. also decrypts that data        
*
**************************************************************/
// #include <stdio.h> // 

#include <linux/module.h> // 
#include <linux/string.h> // 
#include <linux/kernel.h> // 
#include <linux/fs.h> // open close read write to device driver
#include <linux/cdev.h> // help reg to kernel
#include <linux/sched.h> // help reg to kernel
#include <linux/vmalloc.h> // 
#include <linux/uaccess.h> // 


//used to identify the Device Driver
#define MY_MAJOR 420
#define MY_MINOR 0
#define DEVICE_NAME "Salvatierra_Martin_HW6_main"

//Macros to check character in correct upper or lower case
#define islower(c) ((c) >= 'a' && (c) <= 'z')//false if not a letter
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')//false if not a letter


MODULE_AUTHOR("Martin Salvatierra");
MODULE_DESCRIPTION("unknown driver");
MODULE_LICENSE("GPL");
//used to error check read data from user
int readStat;
int major,minor;

struct cdev my_cdev;

//used to store in kernel memory
struct myMessage{
    int messageLength;
    int encryptionYN;
    char messageKey;
    char dataStored[100];
}myMessage;

static ssize_t myWrite(struct file *fs, const char __user *buf, size_t hsize, loff_t *off){
    //size of user Buffer in struct #of bytes = #char
    myMessage.messageLength = hsize;
    //safe way to right to kernel with linux library
    //Move raw user data to kernel with hsize bytes,
    readStat = copy_from_user( myMessage.dataStored, buf, hsize);
    //0 means no missing characters to read, success is 0
    printk(KERN_INFO "in myWrite readStat %d\n",readStat);
    //destin, source, size of transfer
    return readStat;//amount read
}

//(destination ,provided by user,space available in user buf, curr position in open file )
static ssize_t myRead(struct file *fs, char __user *buf, size_t hsize, loff_t *off){
    //used to cycle through user buffer
    int i = 0;

  //checking to make sure message is not too long
    if(hsize >= 100){
       printk(KERN_ERR "My write buffer too big. %d\n", myMessage.messageLength);
        return -1;
    }
    //check key and encoding state before entering loop
    printk(KERN_INFO "**in kernel, encrytpion state**: %d encrytion key:%d \n", myMessage.encryptionYN, myMessage.messageKey);
    
    /**Main loop to encrypt data using key provided by the user or set manually*/
    if( myMessage.encryptionYN == 1){//encryption is on

       //handle upper and lower case cycling through alphabet
        for(i = 0 ; i < hsize; i++){
            if(islower( myMessage.dataStored[i])){
                if(myMessage.dataStored[i] == 'z'){
                    myMessage.dataStored[i] = 'a';
                }else{
                    //messagekey is dynamic, set by the user
                    myMessage.dataStored[i] += myMessage.messageKey;
                }
            }else if(isupper(myMessage.dataStored[i])){
                if(myMessage.dataStored[i] == 'Z'){
                    myMessage.dataStored[i] = 'A';
                }else{
                    
                    myMessage.dataStored[i] += myMessage.messageKey;
                }
            }
        }

    }else{                          //decryption is on

        for(i = 0 ; i < hsize; i++){
            if(islower( myMessage.dataStored[i])){
                if(myMessage.dataStored[i] == 'a'){
                    myMessage.dataStored[i] = 'z';
                }else{
                    // --myMessage.dataStored[i];
                    myMessage.dataStored[i] -= myMessage.messageKey;
                }
            }else if(isupper(myMessage.dataStored[i])){
                if(myMessage.dataStored[i] == 'A'){
                    myMessage.dataStored[i] = 'Z';
                }else{
                    // --myMessage.dataStored[i];
                    myMessage.dataStored[i] -= myMessage.messageKey;
                }
            }
        }

    }

    //destin, source, size of transfer
    //safe way to transfer from kernel space to user space buffer
    readStat = copy_to_user(buf, myMessage.dataStored, hsize);
    //system check for coundaries, key, state, buffer
    printk(KERN_INFO "in myRead read stat : %d, hsize :%ld\n", readStat, hsize);
    printk(KERN_INFO "in myRead my message :%s\n",myMessage.dataStored);
    printk(KERN_INFO "in myRead my message key :%d\n",myMessage.messageKey);
    printk(KERN_INFO "in myRead Buf :%s\n", buf);
    return readStat;//amount not read, 0 means success
}

//setting up space in the kernel
static int myOpen(struct inode * inode, struct file *fs){
    struct myMessage * ds;
    //kernel heap space used
    ds = vmalloc(sizeof(struct myMessage));
    //failure to allocate memory
    if(ds == 0){
        printk(KERN_INFO "Cannot vmalloc, File not opened.\n");
        return -1;
    }
    
    myMessage.encryptionYN = 1;
    myMessage.messageKey = 2;
    myMessage.messageLength = 0;
    fs->private_data = ds;
    return 0;
}
/**unregister and remove device from the kernel*/
static int myClose(struct inode * inode, struct file *fs){
   //free maually allocated memory
    struct myMessage * ds;
    ds = (struct myMessage *) fs-> private_data;
   //deallocate manually set memmory
    vfree(ds);
    return 0;
}

//backdoor to the kernel device driver to comminucate and change parameters
static long myIoctl (struct file *fs, unsigned int command,unsigned long data ){

    int messageKey;
    int *key;
    struct myMessage * ds = (struct myMessage *) fs->private_data;

    if(command == 1){
        myMessage.encryptionYN = 1;
        printk(KERN_INFO "in ioctl encryption is on. %d\n", myMessage.encryptionYN);
    }else if(command == 0){
         myMessage.encryptionYN = 0;
        printk(KERN_INFO "in ioctl decryption is on. %d\n", myMessage.encryptionYN);

    }else{
        printk(KERN_ERR "error in ioctl.\n");
        return -1;
    }
    messageKey =(int) data;
    // *messageKey = ds->messageKey;
    key = (int*) data;
    *key = ds->messageKey;
    printk(KERN_INFO "*in myIOCTL testing: %d \n",ds->messageKey);

    printk(KERN_INFO "**in myIOCTL, encrytpion state**: %d encrytion key:%d \n", myMessage.encryptionYN, myMessage.messageKey);


    return 0;
}



/**file structure*/
//operations i support
struct file_operations fops ={
    .open = myOpen,
    .release = myClose,
    .write = myWrite,
    .read = myRead,
    //set key and encoding flag
    .unlocked_ioctl = myIoctl,//back door to fs
    .owner = THIS_MODULE,
};

int init_module (void){
    int result, registers;
    dev_t devno;

    //create a devise number
    devno = MKDEV(MY_MAJOR, MY_MINOR);
    //register as char device driver
    registers = register_chrdev_region(devno, 1, DEVICE_NAME);
    printk(KERN_INFO "register chardev succeeded 1: %d\n", registers);
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    
    //add d d to system kernel
    result = cdev_add(&my_cdev, devno,1);
    printk(KERN_INFO "dev chardev succeeded 2: %d\n", registers);
    printk(KERN_INFO "NULL driver is loaded: ");

    if(result < 0){
    printk(KERN_ERR "registering char failed  %d\n",result);

    }

    return result;
}
//unregister Device Driver from Kernel
void cleanup_module(void){
    dev_t devno;
    devno = MKDEV(MY_MAJOR,MY_MINOR);
    unregister_chrdev_region(devno, 1);
    cdev_del(&my_cdev);
    printk(KERN_INFO "cleaned up the made up driver.\n");


}


