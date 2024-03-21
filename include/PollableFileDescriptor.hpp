#pragma once

#include <sstream>

enum FDStatus {
	FD_IDLE,    // No polling is being done right now, we are still perparing I/O
	FD_POLLING, // The buffer is ready to be read/written and is part of the poll queue
	FD_DONE,    // The buffer has been read/written and is ready to be used
	FD_ERROR,   // Something went wrong with reading/writing and the data is
	            // assumed to be malformed or incomplete
};

// Abstract class to describe a buffered file descriptor to read from. Use
// this instead of reading directly to let poll handle it and report back
// when the process is done.
//
// When normal, "dumb" read calls suffice, this class can be used as-is.
// If you need different behavior, like using recv or cutting off the stream
// under certain conditions, override the readFromFileDescriptor() method in
// the child class.
//
// After the reading operation finishes, the poll loop will call the
// purely abstract function readingDone(), which you will need to implement
// in your child class to process the buffered data and continue. Afterwards,
// if you want to keep this file descriptor open, you can use resetReadBuffer()
// and restart the process.
class ReadFileDescriptor {
	public:
		virtual ~ReadFileDescriptor() { };

		FDStatus getReadFDStatus( void ) const;

		void setReadFDStatus(FDStatus status);

		// Virtual function so it can be overwritten with specific behavior
		// by the child class. The basic version just reads until the limit
		// with no regard for anything else.
		virtual void readFromFileDescriptor();

		void resetReadBuffer( void );
		virtual void readingDone( void ) = 0;

	protected:
		ReadFileDescriptor(int fd);

		int				  read_fd;
		std::stringstream read_buffer;
		FDStatus          read_status = FD_IDLE;
		std::size_t       bytes_read  = 0;
};

// Abstract class to describe a buffered file descriptor to write to. Use
// this instead of writing directly to let poll handle it and report back
// when the process is done.
//
// When normal, "dumb" write calls suffice, this class can be used as-is.
// If you need different behavior, like using send or cutting off the stream
// under certain conditions, override the writeToFileDescriptor() method in
// the child class.
//
// After the writing operation finishes, the poll loop will call the
// function writingDone(), which you will need to implement in your child
// class if there is more to do there. If so, use resetWriteBuffer() and
// go on. If not, the default writingDone() implementation will just do
// nothing.
class WriteFileDescriptor {
	public:
		virtual ~WriteFileDescriptor() { };

		FDStatus getWriteFDStatus( void ) const;

		void setWriteFDStatus(FDStatus status);

		// Virtual function so it can be overwritten with specific behavior
		// by the child class. The basic version just writes until the limit
		// with no regard for anything else.
		virtual void writeToFileDescriptor();

		void resetWriteBuffer( void );
		virtual void writingDone( void ) { };

	protected:
		WriteFileDescriptor(int fd);

		int				  write_fd;
		std::stringstream write_buffer;
		FDStatus          write_status  = FD_IDLE;
		std::size_t       bytes_written = 0;
};
