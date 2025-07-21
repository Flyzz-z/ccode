#pragma once

#include "socket.h"

class Socket;
class Send;
class Recv;
class Accept;

class IoContext {
public:
	IoContext();

	void run();

private:
	constexpr static std::size_t max_events = 1024;
	const int fd_;
	friend class Socket;
	friend class Send;
	friend class Recv;
	friend class Accept;
	void Attach(Socket *socket);
	void WatchRead(Socket *socket);
	void UnwatchRead(Socket *socket);
	void WatchWrite(Socket *socket);
	void UnwatchWrite(Socket *socket);
	void Detach(Socket *socket);
};