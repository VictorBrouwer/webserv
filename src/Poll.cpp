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

// checks if fd is in the vector, then removes it
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

void Poll::unsetEvent(int fd, short event)
{
	auto it = std::find_if(m_poll_fds.begin(), m_poll_fds.end(),
						   [fd](const pollfd &poll_fd)
						   { return (poll_fd.fd == fd); });
	if (it != m_poll_fds.end())
	{
    	it->events = 0;
    	it->revents = 0;
	}
	if (it != m_poll_fds.end())
	{
    	it->events &= ~event;
    	it->revents &= ~event;
	}
}

void Poll::unsetEvents(int fd)
{
	auto it = std::find_if(m_poll_fds.begin(), m_poll_fds.end(),
						   [fd](const pollfd &poll_fd)
						   { return (poll_fd.fd == fd); });
	if (it != m_poll_fds.end())
	{
    	it->events = 0;
    	it->revents = 0;
	}
}

void Poll::checkErrors(short revents) const
{
	if (revents & POLLHUP)
		throw PollException("Poll wasn't closed properly");
	if (revents & POLLNVAL)
		throw PollException("Invalid file descriptor");
	if (revents & POLLERR)
		throw PollException("Error occurred on file descriptor");
	if (revents & POLLPRI)
		throw PollException("Exceptional condition on file descriptor");
}