#include "PollableFileDescriptor.hpp"
#include "Request.hpp"
#include <unistd.h>

// ReadFileDescriptor

ReadFileDescriptor::ReadFileDescriptor(int fd, int limit = 0) {
	this->read_fd    = fd;
	this->read_limit = limit;
}

FDStatus ReadFileDescriptor::getReadFDStatus( void ) const {
	return this->read_status;
}

void ReadFileDescriptor::setReadFDStatus(FDStatus status) {
	this->read_status = status;
}

void ReadFileDescriptor::readFromFileDescriptor( void ) {
	char *temp_buffer = new char[BUFFER_SIZE];
	if (!temp_buffer)
		throw std::bad_alloc();

	std::size_t bytes_read = read(this->read_fd, &temp_buffer, BUFFER_SIZE);
	if (bytes_read < 0)
		this->read_status = FD_ERROR;
	else if (bytes_read = 0)
		this->read_status = FD_DONE;
	else
		this->bytes_read += bytes_read;



	delete temp_buffer;
}

void ReadFileDescriptor::resetReadBuffer( void ) {

}

// WriteFileDescriptor

WriteFileDescriptor::WriteFileDescriptor(int fd, int limit = 0) {
	this->write_fd    = fd;
	this->write_limit = limit;
}

FDStatus WriteFileDescriptor::getWriteFDStatus( void ) const {
	return this->write_status;
}

void WriteFileDescriptor::setWriteFDStatus(FDStatus status) {
	this->write_status = status;
}

void WriteFileDescriptor::writeToFileDescriptor( void ) {

}

void WriteFileDescriptor::resetWriteBuffer( void ) {

}
