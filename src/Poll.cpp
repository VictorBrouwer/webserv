#include"Poll.hpp"
#include<algorithm>

void Poll::AddPollFd(int fd, short events)
{
	struct pollfd NewPollFd;
	NewPollFd.fd = fd;
	NewPollFd.events = events;
	m_poll_fds.push_back(NewPollFd);
	// m_poll_fds.push_back(pollfd{fd, events, 0});
}

// checks if fd is in the vector, then add it to the end. 
void Poll::RemovePollFd(int fd)
{
	m_poll_fds.erase(std::remove_if(m_poll_fds.begin(), m_poll_fds.end(), 
									[fd](const pollfd &poll_fd){ return (poll_fd.fd == fd); }),
									 m_poll_fds.end());
}

std::vector<pollfd>& Poll::getPollFDs(void)
{
	return (m_poll_fds);
}

void Poll::setEvents(int fd, short events)
{
	auto it = std::find_if(m_poll_fds.begin(), m_poll_fds.end(),
						   [fd](const pollfd &poll_fd)
						   { return (poll_fd.fd == fd); });
	if (it != m_poll_fds.end())
		it->events = events;
}