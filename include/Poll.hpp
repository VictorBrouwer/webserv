#if !defined(POLL_HPP)
#define POLL_HPP

#include <poll.h>
#include <vector>

class Poll
{
public:
	void AddPollFd(int fd, short events);
	void PollFds(void);
	void RemovePollFd(int fd);
	std::vector<pollfd>& getPollFDs(void);
private:
	std::vector<struct pollfd> m_poll_fds;
};


#endif // POLL_HPP
