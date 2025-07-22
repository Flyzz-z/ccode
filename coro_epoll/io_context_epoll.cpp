#include "io_context.h"
#include "socket.h"
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

void IoContext::Attach(Socket *socket) {
	
}

