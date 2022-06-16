#include "fwd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

MESSBASE_NAMESPACE_START

const char *mess_log_module_name{"Default"};

bool mess_set_log_module_name(const char *name) {
    mess_log_module_name = name;
    return name;
}

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool getAvaliablePort(unsigned short &port) {
  [[maybe_unused]] pid_t fpid;
  [[maybe_unused]] int status;
  int server_sockfd; //服务器端套接字
  [[maybe_unused]] int len;
  bool result = true;
  struct sockaddr_in server_addr;               //服务器网络地址结构体
  memset(&server_addr, 0, sizeof(server_addr)); //数据初始化--清零
  server_addr.sin_family = AF_INET;             //设置为IP通信
  server_addr.sin_addr.s_addr =
      htonl(INADDR_ANY); //服务器IP地址--允许连接到所有本地地址上
  // server_addr.sin_port=htons(23); //服务器telnet端口号
  server_addr.sin_port = htons(port); //服务器telnet端口号
  // init_telnetd();
  /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
  if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    //  printf("socket faile server_addr.sin_port %d\n",server_addr.sin_port);
    result = false;
  }

  /*将套接字绑定到服务器的网络地址上*/
  if (bind(server_sockfd, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr)) < 0) {
    //  printf("close faile\n");
    result = false;
  }
  if (0 != close(server_sockfd)) {
    // printf("close faile\n");
    result = false;
  }
  // port = ntohs(server_addr.sin_port);
  return result;
}

bool getAvaliableRangePort(unsigned short &port, unsigned short range_s = 8192,
                           unsigned short range_e = 65535) {
  bool ret;
  unsigned short portIndex;
  for (portIndex = range_s; portIndex <= range_e; portIndex++) {
    ret = getAvaliablePort(portIndex);
    if (ret) break;
  }
  port = ret ? portIndex : 0;
  return ret;
}

unsigned short get_available_port() {
    unsigned short port;
    getAvaliableRangePort(port);
    return port;
}

MESSBASE_NAMESPACE_END
