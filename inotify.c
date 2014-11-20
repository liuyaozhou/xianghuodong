#include <stdio.h>//printf
#include <string.h> //strcmp
#include <sys/inotify.h>//inotify_init inotify_add_watch....
#include <sys/select.h>//select timeval
#include <unistd.h>//close

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define ERR_EXIT(msg,flag)  {perror(msg);goto flag;}

int main( int argc, char **argv ) 
{
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    if((fd = inotify_init()) < 0)
        ERR_EXIT("inotify_init",inotify_init_err);

    if( (wd = inotify_add_watch( fd, "/root/lyz/test/inotify.txt",    IN_MODIFY | IN_CREATE | IN_DELETE ) ) < 0)
        ERR_EXIT("inofity_add_watch", inotify_add_watch_err);
    
    char test_buf[10];
    FILE * fp;
    fp = fopen("/root/lyz/test/inotify.txt", "r");
    if (NULL == fp) {
        printf("open file failed.\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    //int file_length = 0;
    //file_length = ftell(fp);
    
    
    fd_set rfd;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;//10millsecond
    while(true)
    {
        int retval;
        FD_ZERO(&rfd);
        FD_SET(fd, &rfd);
        retval = select(fd + 1, &rfd, NULL, NULL, &tv);
        if(retval == 0) continue;
        else if(retval == -1)
            ERR_EXIT("select",select_err);

        // retval > 0
        length = read( fd, buffer, BUF_LEN );  
        if(length < 0)
            ERR_EXIT("read",read_err);

        //length >= 0
        int i = 0;
        while ( i < length ) 
        {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->len ) 
            {
                if ( event->mask & IN_CREATE ) 
                {
                    if ( event->mask & IN_ISDIR ) 
                        printf( "The directory %s was created.\n", event->name );       
                    else
                        printf( "The file %s was created.\n", event->name );
                    if(strcmp(event->name,"kill") == 0)
                        ERR_EXIT("success exit",success_exit);

                }
                else if ( event->mask & IN_DELETE ) 
                {
                    if ( event->mask & IN_ISDIR ) 
                        printf( "The directory %s was deleted.\n", event->name );       
                    else
                        printf( "The file %s was deleted.\n", event->name );
                }
                else if ( event->mask & IN_MODIFY ) 
                {
                    if ( event->mask & IN_ISDIR )
                        printf( "The directory %s was modified.\n", event->name );
                    else
                        printf( "The file %s was modified.\n", event->name );
                }
            }else
            {
                printf( "The file [%s] was modified.\n", event->name );
                fread(test_buf, 5, 1, fp);
                printf("read:%s\n", test_buf);
                memset(test_buf, '\0', 10);
                //TODO
                //when only a file(not directory) is specified by add watch function, event->len's value may be zero, we can handle it here
            }
            i += EVENT_SIZE + event->len;
        }
        
    }
success_exit:
    ( void ) inotify_rm_watch( fd, wd );
    ( void ) close( fd );
    return 0;

read_err:
select_err:
inotify_add_watch_err:
    ( void ) inotify_rm_watch( fd, wd );
inotify_init_err:
    ( void ) close( fd );

    fclose(fp);
    return -1;
}
