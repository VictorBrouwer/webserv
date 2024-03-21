#pragma once

#include <sstream>

enum FDStatus {
	FD_IDLE,    // No polling is being done right now, we are still perparing I/O
	FD_POLLING, // The buffer is ready to be read/written and is part of the poll queue
	FD_DONE,    // The buffer has been read/written and is ready to be used
	FD_ERROR,   // Something went wrong with reading/writing and the data is
	            // assumed to be malformed or incomplete
};

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

	protected:
		ReadFileDescriptor(int fd);

		int				  read_fd;
		std::stringstream read_buffer;
		FDStatus          read_status = FD_IDLE;
		std::size_t       bytes_read  = 0;
};

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

	protected:
		WriteFileDescriptor(int fd);

		int				  write_fd;
		std::stringstream write_buffer;
		FDStatus          write_status  = FD_IDLE;
		std::size_t       bytes_written = 0;
};
