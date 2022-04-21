#include <irq.h>

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
    


}