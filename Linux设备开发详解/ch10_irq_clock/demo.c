#include <linux/irq.h>

namespace base{

// 10.3.1 : ask for irq
int request_irq( unsigned int irq, irq_handler headler/*顶半部*/, unsigned long flags, const char* name, void *dev/*private data*/);
int devm_request_irq(struct device *dev, unsigned int irq, irq_handler headler/*顶半部*/, unsigned long flags, const char* name,  void *dev_id); // linux managed
void free_irq( unsigned int irq, void *dev );

// 10.3.2 :  enable irq
void disable_irq(int irq);
void disable_irq_nosync(int irq);
void enable_irq(int irq); 
    // 屏蔽 all irq  at this CPU
#define local_irq_save(flags) ...
void local_irq_store_disable(void);
    // Open all irq  at this CPU 
#define local_irq_restore(flags) ...
void local_irq_store_enable(void); 


// 10.3.2 : 底半部: tasklet work-queue soft-irq thread-irq
    // 1. tasklet : based on soft-irq
    // void myTasklet_func(unsigned long ); // process data  
    // DECLARE_TASKLET(myTasklet, myTasklet_func, data);
    // tasklet_schedule( &myTasklet); 
    namespace tasklet{
        struct tasklet xxx_tasklet;
        void xxx_do_tasklet(unsigned long);
        DECLARE_TASKLET(xxx_tasklet, xxx_do_tasklet, 0 );
        /* 底半部 */
        void xxx_do_tasklet(unsigned long ul){\
        }
        /* 顶半部 */
        irqreturn_t xxx_interrupt(int irq, void* dev_id){ 
         
// 10.3.4  /****************    Shared - irq      ****************/
            int statue = read_int_status();
            if( !ismyint( dev_id, statue))
                return IRQ_NONE;
            ...;
            tasklet_schedule( &myTasklet); 
            ...;            
            return IRQ_HANDLED;
        }
        /*  init  */
        static int __init xxx_init( void ){ // ...
            // ask for irq
            request_irq( xxx_irq, xxx_interrupt, 0 /*IRQF_SHARED*/, "xxx", 0 /*xxx_dev*/ ); // ...
        }
        /*  _exit  */
        static int __exit xxx_exit( void ){ 
            free_irq( xxx_irq, xxx_interrupt, 0, "xxx", NULL ); 
        };
        // modile_init( xxx_init ); ...
    }

    // 2. work-queue
    // struct work_struct my_wq;
    // void my_wq_func(struct work_struct *work);
    // INIT_WORK( &my_wq, my_wq_func);
    // schedule_work(&my_wq ); 
    namespace work-queue{
        //  tong tasklet 
    }

    // 3. soft-irq : based on atomic-ctx 底层 底层 底层 
    // struct softirq_action sirq;
    // open_softirq() 注册
    // raise_softirq() 触发 irq
    // local_bh_disable() 禁用 soft-irq 底层
    // local_bh_enable()
    
    // 4. threaded-irq
    // int request_threaded_irq( unsigned int irq, irq_handler headler /*顶半部 可为NULL ---> IRQF_ONESHOT 开启*/, 
    // irq_handler threaded_fn,  /* 底半部 */
    // unsigned long flags, const char* name, void *dev/*private data*/);


// 10.3.5 : 内核定时器:
    DEFINE_TIMER(_name, _func, _expire/*time remain*/, _data);
    __setup_timer( _timer, _fun, _data, _flags);
    void add_timer( struct timer_list *timer);
    int del_timer( struct timer_list * timer); // _sync 
    int mod_timer( struct timer_list * timer);

    // 高精度 timer

    // struct delayed_work        
        { work, timer, cpu, work_queue  } 
      // schedule_delayed_work( )
      // cancel_delayed_work( )

      namespace second-device{
          /*  定时IO操作于  file-device  */
            #include <linux/module.h>
            #include <linux/fs.h>
            #include <linux/mm.h>
            #include <linux/init.h>
            #include <linux/cdev.h>
            #include <linux/slab.h> 
            #include <linux/uaccess.h> 
            #define SECOND_MAJOR 248

            static int second_major = SECOND_MAJOR;
            module_param(second_major, int, S_IRUGO);
            struct second_dev{
                struct cdev cdev;
                atomic_t counter;
                struct timer_list s_timer;
            } 
            static struct second_dev *second_devp;

            static void second_timer_handler( unsigned ling arg){
                mod_timer( &second_devp->s_timer, jiffies + HZ);
                printk(KERN_INFO "current jiffies is %ld \n", jiffies);
            }
            static int second_open(struct inode *inode, struct fime* filp){
                init_timers(&second_devp);
                second_devp->s_timer.function = &second_timer_handler;
                second_devp->s_timer.expires = jiffies + HZ;
                add_timer( &second_devp->s_timer);
                atomic_set(&second_devp->counter, 0);
                return 0;
            }
            static int second_release( struct inode *inode, struct file* filp ){
                del_timer( &second_devp->timer );
                return 0;
            }
            static ssize_t second_read( struct file* filp, char __user *buff, size_t, count, loff_t *ppos){
                int coounter;
                counter = atomic_read( &secnd_devp->counter );
                if(put_user(counter, (int*) buf ))
                    return -EFAULT;
                else
                    return sizeof( unsigned int );
            }
            struct const struct file_operations second_fops = {
                .ower = THIS_MODULE;
                .open = second_open;
                .release = second_release;
                .read = second_read; 
            }
            static void second_setup_cdev(struct second_dev *dev, int index){
                int err, devnno = MKDEV(second_major, ondex);
                cdev_init( &dev->cdev, devno, 1);
                
            }
            


      }





}