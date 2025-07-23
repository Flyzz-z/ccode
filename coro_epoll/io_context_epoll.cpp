#include "io_context.h"
#include "socket.h"
#include <cstdio>
#include <stdexcept>
#include <sys/epoll.h>  // 添加此行，引入 epoll 相关函数声明

IoContext::IoContext() : fd_(epoll_create1(0)) {
	if(fd_ == -1) {
		throw std::runtime_error("epoll_create1 failed");
	}
}

void IoContext::run() {
	struct epoll_event ev, events[max_events];
	for(;;) {
		int nfds = epoll_wait(fd_,events,max_events,-1);
		if(nfds == -1)	{
			throw std::runtime_error("epoll_wait failed");
		}
		for(int i = 0; i < nfds; i++) {
			auto socket = static_cast<Socket*>(events[i].data.ptr);
			if(events[i].events & EPOLLIN) {
				socket->ResumeRecv();
			}
			if(events[i].events & EPOLLOUT) {
				socket->ResumeSend();
			}
		}
	}
}

/**
 * @brief 将一个套接字附加到IO上下文
 *
 * 将一个套接字附加到IO上下文中，使用epoll机制监听该套接字的事件。
 *
 * @param socket 要附加的套接字指针
 *
 * @throws std::runtime_error 当epoll_ctl调用失败时抛出运行时错误
 */
void IoContext::Attach(Socket *socket) {
	struct epoll_event ev;
	auto io_state = EPOLLIN | EPOLLET;
	ev.events = io_state;
	ev.dada.ptr = socket;
	if(epoll_ctl(fd_,EPOLL_CTL_ADD,socket->fd_,&ev) == -1) {
		throw std::runtime_error("epoll_ctl failed");
	}
	socket->io_state_ = io_state;
}

#define UpdateState(new_state) \
	if(socket->io_state_ != new_state) { \
		struct epoll_event ev; \
		ev.events = new_state; \
		ev.data.ptr = socket; \
		if(epoll_ctl(fd_,EPOLL_CTL_MOD,socket->fd_,&ev) == -1) { \
			throw std::runtime_error("epoll_ctl failed"); \
		} \
		socket->io_state_ = new_state; \
	} \

void IoContext::WatchRead(Socket *socket) {
	UpdateState(socket->io_state_ | EPOLLIN);
}

void IoContext::UnwatchRead(Socket *socket) {
	UpdateState(socket->io_state_ & ~EPOLLIN);
}

void IoContext::WatchWrite(Socket *socket) {
	UpdateState(socket->io_state_ | EPOLLOUT);
}

void IoContext::UnwatchWrite(Socket *socket) {
	UpdateState(socket->io_state_ & ~EPOLLOUT);
}

void IoContext::Detach(Socket *socket) {
	if(epoll_ctl(fd_,EPOLL_CTL_DEL,socket->fd_,nullptr) == -1) {
		perror("epoll_ctl failed")
		exit(EXIT_FAILURE)
	}
	socket->io_state_ = 0;
}


