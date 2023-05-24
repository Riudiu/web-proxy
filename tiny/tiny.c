/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and client_error().
 */
#include "csapp.h"

void doit(int fd);
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
void serve_static(int fd, char *filename, int filesize, char *method);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void get_filetype(char *filename, char *filetype);
int parse_uri(char *uri, char *filename, char *cgiargs);


/* port 번호를 인자로 받는다 */
int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  // Check command line args 
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 해당 포트 번호에 해당하는 듣기 소켓 식별자를 열어준다 
  listenfd = Open_listenfd(argv[1]);

  // 클라이언트의 요청이 올 때마다 새로 연결 소켓을 만들어 doit() 호출 
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

/*
  doit(serverfd) - 트랜잭션 처리 함수
  요청라인을 읽고 분석(rio_readlineb함수 사용)
  클라이언트의 요청 라인을 확인해 정적, 동적 컨텐츠를 구분하고 각각의 서버에 보낸다.
*/
void doit(int fd) {
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];

  // Rio Package: 짧은 카운트에서 발생할 수 있는 네트워크 프로그램 같은 응용에서 편리하고, 안정적이고 효율적인 I/O 제공
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers: \n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "HEAD") == 0)) {
    client_error(fd, method, "501", "Not implemented", 
      "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  /* GET 요청에서 URI 구문 분석 */
  /* parse_uri - 클라이언트 요청 라인에서 받아온 uri를 이용해 정적/동적 컨텐츠를 구분한다. */
  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0) {
    client_error(fd, filename, "404", "Not found", 
      "Tiny couldn't find this file");
    return;
  }

  if (is_static) { /* 정적 콘텐츠 제공 */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      client_error(fd, filename, "403", "Forbidden", 
        "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size, method);
  }
  /* 동적 콘텐츠 제공 */
  else {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      client_error(fd, filename, "403", "Forbidden", 
        "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs, method);
  }
}

/*
  clienterror - 에러 메세지와 응답 본체를 서버 소켓을 통해 클라이언트에 보낸다
*/
void client_error(int fd, char *cause, char *errnum, 
    char *shortmsg, char *longmsg) 
{
  char buf[MAXLINE], body[MAXBUF];

  // Build the HTTP response body
  sprintf(body, "<html><title>Tindy Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s : %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s : %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  // Print the HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-Type : text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-Length : %d\r\n\r\n", (int)strlen(body));

  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

/* 클라이언트가 버퍼 rp에 보낸 나머지 요청 헤더들을 무시한다(그냥 프린트한다) */
void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);

  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

/* uri를 받아 filename과 cgiarg를 채워준다 */
int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;

  /* static */
  if(!strstr(uri, "cgi-bin")) {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;
  }
  /* dynamic */
  else {
    ptr = index(uri, '?');
    if(ptr) {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    } 
    else {
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

/*
  serve_static - 클라이언트가 원하는 파일 디렉토리를 받아오고, 응답 라인과 헤더를 작성하고 서버에게 보낸다
*/
void serve_static(int fd, char *filename, int filesize, char *method) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  // 클라이언트에게 응답 header 전달
  get_filetype(filename, filetype);  // 파일 타입 결정
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection : close\r\n", buf);
  sprintf(buf, "%sContent-Length : %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-Type : %s\r\n\r\n", buf, filetype);
  
  Rio_writen(fd, buf, strlen(buf));
  printf("response headers: \n");
  printf("%s", buf);

  // HEAD 메소드 지원
  if (strcasecmp(method, "HEAD") == 0) {
    return; 
  }

  // 클라이언트에게 응답 body 전달 
  srcfd = Open(filename, O_RDONLY, 0);
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);  // mmap() - 요청한 파일을 가상 메모리 영역으로 매핑
  srcp = malloc(filesize);

  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  // Munmap(srcp, filesize);  // 매핑된 가상메모리 주소 반환
  free(srcp);  // 메모리 누수 방지
}

/*
  get_filetype - Derive file type from filename
  filename을 조사해 각각의 식별자에 맞는 MIME 타입을 filetype에 입력해준다
  -> Response header의 Content-type에 필요함
*/
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  }
  else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  }
  else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  }
  else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  }
  else if (strstr(filename, ".mp4")) {
    strcpy(filetype, "video/mp4");
  }
  else 
    strcpy(filetype, "text/plain");
}

/*
  serve_dynamic() -  클라이언트가 원하는 동적 컨텐츠 디렉토리를 받아오고, 응답 라인과 헤더를 작성하고 서버에게 보낸다
  CGI 자식 프로세스를 fork하고 그 프로세스의 표준 출력을 클라이언트 출력과 연결한다
*/
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method) {
  char buf[MAXLINE], *emptylist[] = { NULL };

  // Return first part of HTTP response
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) {
    // real server would set all CGI vars here 
    setenv("QUERY_STRING", cgiargs, 1);
    // HEAD 메소드 지원
    setenv("REQUEST_METHOD", method, 1); 

    Dup2(fd, STDOUT_FILENO);   // Redirect stdout to client
    Execve(filename, emptylist, environ);  // CGI 프로그램 실행
  }
  wait(NULL);  // Parent waits for and reaps child
}