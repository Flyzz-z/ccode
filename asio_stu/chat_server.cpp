#include <asio/ip/tcp.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/redirect_error.hpp>
#include <asio/use_awaitable.hpp>
#include <memory>
#include <set>
#include <deque>

using asio::ip::tcp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const std::string& msg) = 0;
};

using chat_participant_ptr = std::shared_ptr<chat_participant> ;

class chat_room
{
public:
	void join(chat_participant_ptr participant)
	{
		participants_.insert(participant);
		for (auto msg: recent_msgs_)
			participant->deliver(msg);
	}

	void leave(chat_participant_ptr participant)
	{
		participants_.erase(participant);
	}

	void deliver(const std::string& msg)
	{
		recent_msgs_.push_back(msg);
		while (recent_msgs_.size() > max_recent_msgs)
			recent_msgs_.pop_front();

		for (auto participant: participants_)
			participant->deliver(msg);
	}
private:
	std::set<chat_participant_ptr> participants_;
	enum {max_recent_msgs = 100};
	std::deque<std::string> recent_msgs_;
 };