#include <time.h>
#include <stdio.h>

main(){
    struct tm ts;
    time_t  now;
    unsigned int timenow=0;
    timenow = (unsigned)time(NULL);
    time(&now);

    printf("%d",timenow);
}