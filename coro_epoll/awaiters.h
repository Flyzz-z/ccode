#pragma once

#include "socket.h"
#include "task.h"
#include <iostream>
#include <sys/socket.h>

template <typename Syscall, typename ReturnValue> class AsyncSyscall {
public:
  AsyncSyscall() : suspended_(false) {}

protected:
  bool suspended_;
  // 当前awaiter所在协程的handle，需要设置给socket的coro_recv_或是coro_send_来读写数据
  // handle_不是在构造函数中设置的，所以在子类的构造函数中也无法获取，必须在await_suspend以后才能设置
  std::coroutine_handle<> handle_;
  ReturnValue value_;
};

class Socket;

class Accept : public AsyncSyscall<Accept, int> {
public:
  Accept(Socket *socket) : AsyncSyscall{}, socket_(socket) {
    socket_->io_context_.WatchRead(socket_);
    std::cout << "Accept::Accept" << std::endl;
  }

  ~Accept() {
    socket_->io_context_.UnwatchRead(socket_);
    std::cout << "Accept::~Accept" << std::endl;
  }

  int Syscall() { 
		struct sockaddr_storage addr; 
		socklen_t addr_size = sizeof(addr);
		std::cout<<"accept "<<socket_->fd_<<std::endl;
		return ::accept(socket_->fd_, (struct sockaddr *)&addr, &addr_size);
	}

	void SetcoroHandle() {
		socket_->coro_recv_ = handle_;
	}

private:
  Socket *socket_;
  void *buffer_;
  std::size_t len_;
};

class Send {

private:
	Socket *socket_;
	void *buffer_;
	std::size_t len_;
};