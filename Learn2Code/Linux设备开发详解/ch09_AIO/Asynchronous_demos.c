#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <signal.h>     /* Working On UserSpace below */

namespace ch9_usr_fun0{
    // sig_process  (like Qt: connect ) 
    void (*signal(int sig_num, void (*heandler)(int) ) )(int);
    // ===
    typedef void (*sighandler_t)(int);
    sighandler_t signal( int sig_num, sighandler_t heandler );

    // change sig_process  re-connect
    int sigaction(int sig_num, const struct sigaction* act, struct sigaction* old_act);

    // signal catch example:
    void sigterm_handler( int signo){
        printf( "Have got a signal %d ;\n", signo);
        exit(0);
    }
    int main(void){
        signal( SIGINT, sigterm_handler );
        signal( SIGTERM, sigterm_handler );
        while(1);
        return 0;
    }
}

namespace ch9_usr_demo0{
    static const int MAX_LEN = 100;
    void input_headler(int sig){
        char data[MAX_LEN];
        int len;
        /*read the StdIO-input */
        len = read( STDIO_FILENO, &data, MAX_LEN);
        data[len] = '\0';
        printf("Input : %s\n", data);
    }
    int main(){
        int oflags;
        /* start sig-driver*/
        signal(SIGIO, input_headler );
        fcntl(STDIN_FILENO, F_SETOWN, getpid());
        oflags = fcntl(STDIN_FILENO, F_GETFL);
        fcntl(STDIN_FILENO, F_SETFL, oflags || FASYNC);
        while(1);
    }
}

namespace ch9_usr_globalfifo{
    static void signalio_headler(int sig){ 
        printf("Input a SIG from globalfifo, sig_num :%d\n", sig);
    }
    int main(){
        int fd, oflags;
        // create device node globalfifo
        // sudo mknode /dev/globalfifo
        fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
        if(fd != -1){
            signal( SIGIO, signalio_headler);
            fcntl(STDIN_FILENO, F_SETOWN, getpid());
            oflags = fcntl(STDIN_FILENO, F_GETFL);
            fcntl(STDIN_FILENO, F_SETFL, oflags || FASYNC);
            int i= 100;
            while( i-- ){
                sleep( 1000 );
            }
        }else
            printf("Error On Open device! \n");
    }
}

namespace ch9_usr_AIO_H{

#include <aio.h>  // usr libs

    /* muti implementation */
    int aio_read( struct aiocb* aiocbp);  //  AIO IO Contral Block :all thing needed ;  enqueue()
    int aio_write(struct aiocb* aiocbp); // the same to up
    int aio_error(struct aiocb* aiocbp); // check the statue-processing  ; key-words
    ssize_t aio_return( ); // same as  read\write return 

    int aio_demo(){
        // ...
        const static int BUFFSIZE = 1024;
        int fd, ret;
        struct aiocb my_aiocb;

        fd = open("demo.txt", O_RDONLY);
        if (fd < 0)
            perror( "> demo.txt < Opened OK\n" );

        bzero(&my_aiocb, sizeof( struct aicb));
        my_aiocb.aio_buf = malloc(BUFFSIZE +1);
        if (!my_aiocb.aio_buf) 
            perror("Error on amlloc!\n");

        my_aiocb.aio_fildes = fd;
        my_aiocb.aio_nbytes = BUFFSIZE;
        my_aiocb.aio_offset = 0;
        
        ret = aio_read( &my_aiocb );
        if (ret < 0)
            perror("Error on read\n");
        while( aio_error(&my_aiocb)==EINPROGRESS )
            continue;
        if ( (ret = aio_return(&my_aiocb)) > 0 ){
            char* data = (char*)my_aiocb.aio_buf;
            data[ BUFFSIZE ] = '\0';
            printf( "Readed : %s\n", data );
        }else 
            perror("Return error!\n");
    }

    int aio_suspend( const struct aiocb *const cblist[], int n, const struct timespec* timeout); // block the usr process to wait the result
    int suspend_demo(){
        const static int MAXLIST = 1024;
        struct aiocb* cblist[MAXLIST];
        bzero( (char*)cblist, sizeof(cblist) );
        cblist[0] = &my_aiocb; // 
        ret = aio_read(&my_aiocb);
        ret = aio_suspend( cblist, MAXLIST, NULL);
    }

    int aio_cancel( int fd, struct aiocb* aiocbp /*NULL-> all ask canceled*/); // -> AIO_CANCELED AIO_NOTCANCELED  ; cancel by-hand : dequeue
    int lio_listio( int mode, struct aiocb* list[], int nent, struct sigevent *sig); // multi
    int lio_demo(){
        int MAXLIST = 1024;
        struct aiocb io1, io2, io3;
        struct aiocb *aio_list[MAXLIST];
        io1.aio_fildes = fd;
        io1.aio_buf = malloc( MAXLIST + 1);
        io1.aio_nbytes = BUFFSIZE;
        io2 = io1;
        io3 = io1; 
        bzero( (char*)aio_list , sizeof(aio_list));
        aio_list[0] = &io1;
        aio_list[1] = &io2;
        aio_list[2] = &io3;

        lio_listio( LIO_WAIT /*LIO_NOWAIT*/, aio_list, MAXLIST, NULL); // blocked
    }

    // lio_return ... et al
}

namespace ch9_SystemCall_AIO{ // API

#define _GNU_SOURCE /* O_DIRECT */
#include <libaio.h> // linux
#include <stdio.h>  // perror
#include <unistd.h> // system call
#include <fcntl.h>  // O_RDWR
#include <string.h>
#include <inttypes.h>
#include <stdlib.h> 
#define BUF_SIZE 1024

#define FUN_CHECK(x) do {\
    if (x) { \
        printf("Error ON %s", %%x ); \
    } \
}while(0) 

void sysCall(){
    int io_setup(int maxenents, io_coontetxt_t *ctxp);
    int io_destory(io_context_t ctx);
    int io_submit( io_context_t ctx, long nr, struct iocb *ios[]);
    int io_cancel( io_context_t ctx, struct iocb *iocb, struct io_event *evt);
    int io_getevents ...;
    void io_set_callback(struct iocb* iocb, io_callback cb);
    void io_pre_pwrite(struct iocb* iocb, int fd, void* buf, size_t count, long long offset);
    void io_pre_pread(struct iocb* iocb, int fd, void* buf, size_t count, long long offset);
    void io_pre_pwritev/* v -> vector */(struct iocb* iocb, int fd, const struct iovec* iov, size_t iovcount, long long offset);
    void io_pre_preadv(struct iocb* iocb, int fd, const struct iovec* iov, size_t iovcount, long long offset);
}

int main(int argc, char** argv){
    io_context_t ctx = 0;
    struct iocb cb;
    struct iocb *cbs[1];
    unsigned char* buf;
    struct io_event enents[1];
    int ret, fd;
    if (argc < 2){
        printf("Error ON Conmond-argc !\n");
        exit(0);
    }
    fd = open("demo.txt", O_RDWR | O_DIRENT);
    if (fd <0){
        perror("Error on demo.txt open \n");
        goto error;
    }
    ret = posix_memalign( (void**)&buf, 512, (BUF_SIZE+1));
    if (ret < 0){
        perror("Error on  posix_memalign\n");
        goto error1;
    } 
    memset( buf, 0, BUF_SIZE+1 )
    ret = io_setup(128, &ctx);
    if (ret < 0){
        printf("Error on io_setup \n");
        goto error2;
    }

    io_pre_pread( &cb, fd, buf, BUF_SIZE， 0)；
    cbs[0] = &cb;
    ret = io_submit( ctx, 1, cbs);
    if(ret != -1){
        if (ret < 0) 
            printf("Error on io_submit" ); 
        else 
            fprintf( stderr, "Couldnot submit IOs\n");
        goto error3
    }
    ret = io_getenents(ctx, 1,1 enents NULL);
    if (ret !=-1){
        if (ret < 0)
            printf("Erroe ON io_getenents\n");
        else    
            fprintf( stderr, "Coudnot io_getenents");
        goto error3;
    }
    if(events[0].res2 == 0)
        printf("%s \n", buf);
    else{
        printf("AIO Error ON %s", strerror(-enents[0].res ) );
        goto error3;
    }
    if( (ret = io_destory( ctx )) < 0 ){
        printf("io_destory error: %s", strerror( -ret ));
        goto error2;
    }

    free(buf);
    close(fd);
    return 0;

   
error3: 
    if( (ret = io_destory( ctx )) < 0 )   
        printf("io_destory error: %s", strerror( -ret )); 
error2: 
    free(buf); 
error1:  
    close(fd); 
error0: 
    return -1;    
} 
} // namespace