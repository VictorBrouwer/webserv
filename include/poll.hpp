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
	void HandleActiveClient(struct pollfd curr);
	std::vector<pollfd> getPollFDs(void) const;
private:
	std::vector<struct pollfd> m_poll_fds;
}


#endif // POLL_HPP
