#if !defined(POLL_HPP)
#define POLL_HPP

#include <poll.h>
#include <vector>
#include <stdexcept>
#include <exception>

class Poll
{
public:
	void AddPollFd(int fd, short events);
	void PollFds(void);
	void RemovePollFd(int fd);
	void setEvents(int fd, short events);
	void unsetEvent(int fd, short event);
	void unsetEvents(int fd);
	std::vector<pollfd>& getPollFDs(void);
	void checkErrors(short revents) const;
	
	class PollException : public std::runtime_error
	{
	  public:
		PollException(const std::string &message) : std::runtime_error(message)
		{
		}
	};
private:
	std::vector<struct pollfd> m_poll_fds;
};


#endif // POLL_HPP
