#include "csapp.h"

int main(int argc, char **argv) 
{
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    /* hint 구조체 세팅 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error : %s\n", gai_strerror(rc));
        exit(1);
    }

    /*
        addrinfo 리스트 하나하나 살펴보면서 ai_addr, 즉 IP주소를 반환한다. 
        domain은 getaddrinfo에서 받았고, 
        해당 domain에 대응되는 많은 IP주소들이 addrinfo 리스트에 나온다.
        각각의 IP주소를 domain으로 변환시키지 않고 그냥 출력한다.
    */
    flags = NI_NUMERICHOST;  // 도메인 이름을 리턴하지 않고 10진수 주소 스트링을 대신 리턴한다.
    for (p = listp; p; p = p->ai_next) {
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    /* clean up - addrinfo 구조체를 free */
    Freeaddrinfo(listp);

    exit(0);
}