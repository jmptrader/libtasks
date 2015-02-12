/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_SOCKET_H_
#define _TASKS_SOCKET_H_

#include <string>
#include <exception>
#include <memory>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>

#include <tasks/io_base.h>
#include <tasks/tasks_exception.h>

#ifdef _OS_LINUX_
#define SEND_RECV_FLAGS MSG_NOSIGNAL
#else
#define SEND_RECV_FLAGS 0
#endif

struct sockaddr_in;

namespace tasks {
namespace net {

enum class socket_type { TCP, UDP };

class socket : public io_base {
  public:
    socket(int fd) : io_base(fd) {}
    socket(socket_type = socket_type::TCP);

    inline bool udp() const { return m_type == socket_type::UDP; }

    inline bool tcp() const { return m_type == socket_type::TCP; }

    inline socket_type type() const { return m_type; }

    inline std::shared_ptr<struct sockaddr_in> addr() { return m_addr; }

    inline void set_blocking() { m_blocking = true; }

    // bind for udp sockets. This method can be used to bind udp sockets. For tcp servers
    // socket::listen has to be called.
    void bind(int port, std::string ip = "");

    // listen for unix domain sockets
    void listen(std::string path, int queue_size = 128);

    // listen for tcp sockets
    void listen(int port, std::string ip = "", int queue_size = 128);

    socket accept();

    // connect a domain socket
    void connect(std::string path);

    // connect via tcp
    void connect(std::string host, int port);

    void shutdown();

    std::streamsize write(const char* data, std::size_t len, int port = -1, std::string ip = "");
    std::streamsize read(char* data, std::size_t len);

  private:
    socket_type m_type = socket_type::TCP;
    bool m_blocking = false;
    std::shared_ptr<struct sockaddr_in> m_addr;

    void bind(int port, std::string ip, bool udp);
    void init_sockaddr(int port, std::string ip = "");
};

}  // net
}  // tasks

#endif  // _TASKS_SOCKET_H_
