
# Web-Server, Proxy-Server 구현

## 테스트

- (mac) 시스템 설정 > 네트워크 > 네트워크 서비스 > 세부사항 > 프록시 > 자동 프록시 인식 ON
- sudo apt-get install net-tools (ec2 ubuntu 환경 net-tools 설치)
- `make all`후 `./driver.sh` 실행하면 자세한 채점(테스트) 결과가 나옵니다. 


## 의도(Motivation)

- HTTP, Socket, TCP/IP, 프록시 서버 등 네트워크 지식에 대한 이해도 상승


## Main 파일 구성 요소

#### proxy.c   
#### csapp.h    
#### csapp.c  
    
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

#### Makefile
    
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

#### driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

#### tiny
    Tiny Web server from the CS:APP text

#### port-for-user.pl

    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

#### free-port.sh

    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

#### nop-server.py

     helper for the autograder.         
