#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct addrinfo hints;
struct addrinfo *res;
int status;
int s;

int main(){
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo("127.0.0.1", "80", &hints, &res);
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    printf("%d",s);
    return 1;
}