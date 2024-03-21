#include <memory>
#include <unistd.h>

#include "PollableFileDescriptor.hpp"
#include "Request.hpp"

// ReadFileDescriptor

ReadFileDescriptor::ReadFileDescriptor(int fd) {
	this->read_fd = fd;
}

FDStatus ReadFileDescriptor::getReadFDStatus( void ) const {
	return this->read_status;
}

void ReadFileDescriptor::setReadFDStatus(FDStatus status) {
	this->read_status = status;
}

void ReadFileDescriptor::readFromFileDescriptor( void ) {
	std::unique_ptr<char[]> temp_buffer(new char[BUFFER_SIZE]);

	std::size_t bytes_read = read(this->read_fd, &temp_buffer, BUFFER_SIZE);
	if (bytes_read < 0)
		this->read_status = FD_ERROR;
	else if (bytes_read == 0)
		this->read_status = FD_DONE;
	else {
		this->bytes_read += bytes_read;
		this->read_buffer.write(temp_buffer.get(), bytes_read);
	}
}

void ReadFileDescriptor::resetReadBuffer( void ) {
	this->read_buffer.str("");
	this->bytes_read  = 0;
	this->read_status = FD_IDLE;
}

// WriteFileDescriptor

WriteFileDescriptor::WriteFileDescriptor(int fd) {
	this->write_fd = fd;
}

FDStatus WriteFileDescriptor::getWriteFDStatus( void ) const {
	return this->write_status;
}

void WriteFileDescriptor::setWriteFDStatus(FDStatus status) {
	this->write_status = status;
}

void WriteFileDescriptor::writeToFileDescriptor( void ) {
	std::unique_ptr<char[]> temp_buffer(new char[BUFFER_SIZE]);

	this->write_buffer.read(temp_buffer.get(), BUFFER_SIZE);
	std::size_t bytes_read = this->write_buffer.gcount();

	std::size_t result = write(this->write_fd, temp_buffer.get(), bytes_read);

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
}

void WriteFileDescriptor::resetWriteBuffer( void ) {
	this->write_buffer.str("");
	this->bytes_written = 0;
	this->write_status  = FD_IDLE;
}
