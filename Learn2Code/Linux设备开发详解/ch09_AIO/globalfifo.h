/*globalfifo.h*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <signal.h>     /* Working On UserSpace below */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>


#define GLOBALFIFO_SIZE 1024

struct globalfifo_dev{
    struct cdev cdev;
    unsigned int current_len;
    unsigned int mem[GLOBALFIFO_SIZE];

    struct file_oprations *ops;


    struct mutex mutex;  // 互斥
    wait_queue_head_t r_wait; // 中断，睡眠
    wait_queue_head_t w_wait; // 
    struct fasync_struct *async_queue; // 异步
} * globalfifo_cdev


static int globalfifo_init(int fd, struct file* flip, int mode);
static int globalfifo_exit(int fd, struct file* flip, int mode);

static int globalfifo_fasync(int fd, struct file* flip, int mode);
static int globalfifo_release(struct inode* inode, struct file* filp);
static ssize_t globalfifo_write( struct file* flip, const char __user *buf, ssize_t count, loff_t *f_pos);
static int globalfifo_open(struct inode* inode, struct file* filp);

globalfifo_cdev->ops = {
    .open = globalfifo_open,
    .fasync = globalfifo_fasync,
    .write = globalfifo_write,
    .release = globalfifo_release
}

module_init( globalfifo_init );
module_exit( globalfifo_exit );
MODULE_LICENSE( "GPL" );