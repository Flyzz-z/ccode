#include "socket.h"
#include "awaiters.h"
#include "io_context.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Socket类的构造函数
 *
 * 初始化Socket对象，绑定指定的端口并开始监听连接请求。
 * 构造函数会初始化Socket对象，并设置非阻塞模式。
 * 加入到IO上下文中，并开始监听读取事件。
 *
 * @param port 端口号，类型为std::string_view
 * @param io_context IO上下文对象，类型为IoContext&
 *
 * @throw std::runtime_error 如果绑定失败，则抛出运行时错误异常
 */
Socket::Socket(std::string_view port, IoContext &io_context)
    : io_context_(io_context) {

  struct addrinfo hints, *res;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // need study
  getaddrinfo(NULL, port.data(), &hints, &res);
  fd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  int opt;
  // need study
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (::bind(fd_, res->ai_addr, res->ai_addrlen) == -1) {
    throw std::runtime_error("bind error");
  }
  ::listen(fd_, 10);
  fcntl(fd_, F_SETFL, O_NONBLOCK);
  io_context_.Attach(this);
  io_context_.WatchRead(this);
}

Socket::~Socket() {
  if (fd_ == -1)
    return;
  io_context_.Detach(this);
  std::cout << "close socket" << std::endl;
  ::close(fd_);
}

task<std::shared_ptr<Socket>> Socket::accept() {
    int fd = co_await Accept(this);
		if(fd == -1) {
			throw std::runtime_error("accept error");
		}
		co_return std::shared_ptr<Socket>(new Socket{fd,io_context_});
}

Recv Socket::recv(void  *buffer, std::size_t len) {
  return Recv(this, buffer, len);
}

Send Socket::send(void *buffer, std::size_t len) {
  return Send(this, buffer, len);
}

bool Socket::ResumeRecv() {
  if(!coro_recv_) {
		std::cout << "ResumeRecv: coro_recv_ is null" << std::endl;
		return false;
	}
	coro_recv_.resume();
	return true;
}

bool Socket::ResumeSend() {
  if(!coro_send_) {
		std::cout << "ResumeSend: coro_send_ is null" << std::endl;
		return false;
	}
	coro_send_.resume();
	return true;
}

Socket::Socket(int fd,IoContext &io_context) :  io_context_(io_context), fd_(fd){
	fcntl(fd_, F_SETFL, O_NONBLOCK);
	io_context_.Attach(this);
}