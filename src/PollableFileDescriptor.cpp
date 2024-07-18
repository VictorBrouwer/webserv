#include <memory>
#include <unistd.h>
#include <poll.h>

#include "HelperFuncs.hpp"
#include "PollableFileDescriptor.hpp"
#include "Request.hpp"

// ReadFileDescriptor

ReadFileDescriptor::ReadFileDescriptor(const ReadFileDescriptor& src) {
	this->read_fd = src.read_fd;
}

ReadFileDescriptor::ReadFileDescriptor( void ) {
	this->read_fd = -1;
}

ReadFileDescriptor::ReadFileDescriptor(int fd) {
	this->read_fd = fd;
}

int ReadFileDescriptor::getReadFileDescriptor( void ) const {
	return this->read_fd;
}

FDStatus ReadFileDescriptor::getReadFDStatus( void ) const {
	return this->read_status;
}

void ReadFileDescriptor::readFromFileDescriptor(pollfd pollfd) {
	ssize_t bytes_read = this->doRead();

	if (bytes_read > 0)
		this->bytes_read += bytes_read;

	if (this->read_status != FD_ERROR)
		this->afterRead();
	if (this->read_status == FD_DONE) {
		if (pollfd.revents & POLLHUP)
			this->read_status = FD_HUNG_UP;
		this->readingDone();
	}

	// log(this->read_buffer.str(), L_Warning);
}

void ReadFileDescriptor::resetReadBuffer( void ) {
	this->read_buffer.str("");
	this->bytes_read  = 0;
	this->read_status = FD_IDLE;
}

void ReadFileDescriptor::setReadFDStatus(FDStatus status) {
	this->read_status = status;
}

void ReadFileDescriptor::callReadingDone( void ) {
	this->readingDone();
}

void ReadFileDescriptor::setReadFileDescriptor(int fd) {
	this->read_fd = fd;
}

ssize_t ReadFileDescriptor::doRead( void ) {
	std::unique_ptr<char[]> temp_buffer(new char[BUFFER_SIZE]);

	ssize_t bytes_read = read(this->read_fd, temp_buffer.get(), BUFFER_SIZE);
	if (bytes_read < 0)
		// Something went wrong and we error out here
		this->read_status = FD_ERROR;
	else if (bytes_read == 0)
		// We have reached end of file and our buffer is done
		this->read_status = FD_DONE;
	else {
		// We have new data to add to our buffer
		std::string read_content = std::move(this->read_buffer).str();
		read_content.append(temp_buffer.get(), bytes_read);
		this->read_buffer.str(std::move(read_content));
	}

	return bytes_read;
}


// WriteFileDescriptor

WriteFileDescriptor::WriteFileDescriptor( void ) {
	this->write_fd = -1;
}

WriteFileDescriptor::WriteFileDescriptor(int fd) {
	this->write_fd = fd;
}

int WriteFileDescriptor::getWriteFileDescriptor( void ) const {
	return this->write_fd;
}

FDStatus WriteFileDescriptor::getWriteFDStatus( void ) const {
	return this->write_status;
}

void WriteFileDescriptor::writeToFileDescriptor( void ) {
	ssize_t bytes_written = this->doWrite();

	if (bytes_written > 0)
		this->bytes_written += bytes_written;

	if (this->write_status != FD_ERROR)
		this->afterWrite();
	if (this->write_status == FD_DONE)
		this->writingDone();
}

void WriteFileDescriptor::resetWriteBuffer( void ) {
	this->write_buffer.str("");
	this->bytes_written = 0;
	this->write_status  = FD_IDLE;
}

void WriteFileDescriptor::setWriteFDStatus(FDStatus status) {
	this->write_status = status;
}

void WriteFileDescriptor::setWriteFileDescriptor(int fd) {
	this->write_fd = fd;
}

void WriteFileDescriptor::callWritingDone( void ) {
	this->writingDone();
}

ssize_t WriteFileDescriptor::doWrite( void ) {
	std::unique_ptr<char[]> temp_buffer(new char[BUFFER_SIZE]);

	this->write_buffer.read(temp_buffer.get(), BUFFER_SIZE);
	ssize_t bytes_read = this->write_buffer.gcount();

	ssize_t result = write(this->write_fd, temp_buffer.get(), bytes_read);

	if (result < 0)
		// Write errored and we stop here
		this->write_status = FD_ERROR;
	else if (result < bytes_read) {
		// We did not write everything we wanted to write and need to
		// rewind the buffer for next time
		this->write_buffer.seekp(-(bytes_read - result), std::stringstream::cur);
	} else if (this->write_buffer.eof())
		// We have reached the end of our buffer and we are done writing.
		this->write_status = FD_DONE;

	return result;
}
