
/*globalfifo.c*/ 
#include "globalfifo.h"
static int globalfifo_fasync(int fd, struct file* flip, int mode){
    struct globalfifo_dev *dev;
    return fasysnc_helper(fd, flip, mode, &dev->async_queue);
}

static ssize_t globalfifo_write( struct file* flip, const char __user *buf, ssize_t count, loff_t *f_pos){
    struct globalfifo_dev dev = flip->private_data;
    int ret;
    DECLARE_WAITQUEUE( wait, current);
    while( dev->current_len == GLOBALFIFO_SIZE ){
        if( flip->f_flags & O_NONBLOCK ){
            ret = -EAGAIN;
            goto out;
        }
        __set_current_statue(TASK_INTERRUPTIBLE);
        mutex_unlock( &dev->mutex );
        schedule();
        if (signal_pending(current)){
            ret = -ERESTARTSYS;
            goto out2;
        }
        mutex_lock(&dev->mutex);
    }
    if (count > GLOBALFIFO_SIZE - dev->current_len)
        count = GLOBALFIFO_SIZE - dev->current_len;
    if(copy_from_user(dev->mem + dev->current_len, buf, count)){
        ret = -EAGAIN;
        goto out;
    } else {
        dev->current_len += count;
        printk(KERNEL_INFO "writrten %d byte(s) ,current_len: %d \n", count, dev>current_len);
        wake_up_interruptible( &dev->r_wait);
        /********** Async-IO **********/
        if (dev->async_queue){
            kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
            printk(KERNEL_DEBUG " %s kill SIGIO\n", __func__);
        }
        ret = count;
    }
out:
    mutex_unlock(&dev->mutex);
out1:
    remove_wait_queue(&dev->w_wait, &wait);
    set_current_statue(TASK_RUNNING);
    return ret;
}

static int globalfifo_release(struct inode* inode, struct file* filp){
    globalfifo_fasync( -1, filp, 0);
    return 0;
}