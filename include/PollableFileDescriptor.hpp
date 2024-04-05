#pragma once

#include <poll.h>
#include <sstream>

// Abstract classes to describe a buffered file descriptor to read or write with.
// By inheriting these in other classes, we can polymorphically add them
// to the poll queue and call back to the child class when the I/O is done.
//
// Sending a file descriptor through poll is done as follows:
// 1) When constructing your class, construct the underlying
//    ReadFileDescriptor and/or WriteFileDescriptor with the
//    appropriate fd.
// 2) Once ready to read/write, use the set*FDStatus() function
//    to set the status to FD_POLLING. This adds it to the poll
//    queue.
// 3) When poll tells us we are ready to proceed, the public member function
//    readFromFileDescriptor or writeToFileDescriptor is called to perform the
//    required operations. Anything read will end up in the read_buffer member
//    variable, anything written will be taken out of the write_buffer.
// 4) The status field will be set to FD_DONE and a virtual function will
//    be called to continue where we left off with the child class.
// 5) If you want to reuse this file descriptor/class instance, you can call
//    resetReadBuffer/resetWriteBuffer to go back to the start with an empty
//    buffer, ready to be marked for polling.
//
//
// To customize or interact with this flow, you can do the following things:
// a) Override the afterRead or afterWrite member function. This is called
//    internally with each cycle, so you can interrupt the process early,
//    like when reading from a client socket which is not sending anything
//    useful our way.
// b) Override the readingDone or writingDone member functions. These functions
//    are called once the operation is finished and will let you continue
//    working in the context of your class. For instance, a Client will
//    probably want to parse incoming data to create Requests, and a Response
//    waiting for CGI will probably want to set itself up for sending back to
//    the Client.
// c) Override the doRead/doWrite member functions. These functions are the ones
//    that call read/write on the file descriptor and can be changed if special
//    flags need to be used in your case, or if you want to use send/recv instead.


enum FDStatus {
	FD_IDLE,    // No polling is being done right now, we are still perparing I/O
	FD_POLLING, // The buffer is ready to be read/written and is part of the poll queue
	FD_DONE,    // The buffer has been read/written and is ready to be used, the file descriptor is still open.
	FD_HUNG_UP, // The buffer has been read/written and is ready to be used, but the file descriptor has been hung up and should not be used anymore.
	FD_ERROR    // Something went wrong with reading/writing and the data is assumed to be malformed or incomplete
};


class ReadFileDescriptor {
	public:
		virtual ~ReadFileDescriptor() { };

		int      getReadFileDescriptor( void ) const;
		FDStatus getReadFDStatus( void ) const;

		// Interactable functions

		// This function is called from the poll loop to do a full read cycle.
		// It just calls the virtual functions to make sure they are all
		// part of the cycle as expected.
		void readFromFileDescriptor(pollfd pollfd);

		// Set the status of the file descriptor (from the poll loop or the child class)
		void setReadFDStatus(FDStatus status);

	protected:
		ReadFileDescriptor(int fd);
		ReadFileDescriptor(const ReadFileDescriptor& src);

		// This function clears the buffer and resets the FDStatus.
		void resetReadBuffer( void );

		// Setter to change the file descriptor from the child class.
		void setReadFileDescriptor(int fd);

		// The buffer and amount of bytes read are accessible from
		// the child class.

		std::stringstream read_buffer;
		std::size_t       bytes_read = 0;

	private:
		// Override this function to customize the process of reading itself.
		// Return value should be the amount of bytes read, or -1 on error.
		virtual ssize_t doRead( void );

		// Override this function for a callback after each read cycle.
		virtual void afterRead( void ) { };

		// Override this function to do something in the child class after
		// reading is done.
		virtual void readingDone( void ) { };

		int				  read_fd;
		FDStatus          read_status = FD_IDLE;
};


class WriteFileDescriptor {
	public:
		virtual ~WriteFileDescriptor() { };

		int      getWriteFileDescriptor( void ) const;
		FDStatus getWriteFDStatus( void ) const;

		// Interactable functions

		// This function is called from the poll loop to do a full write cycle.
		// Feel free to look at its implementation and see how the virtual
		// functions are part of it.
		void writeToFileDescriptor();

		// Setter to change the status from the poll loop or the child class.
		void setWriteFDStatus(FDStatus status);

		void callWritingDone( void );

	protected:
		WriteFileDescriptor(int fd);

		// This function clears the buffer and resets the FDStatus.
		void resetWriteBuffer( void );

		// Setter to change the file descriptor from the child class.
		void setWriteFileDescriptor(int fd);

		// The buffer and amount of bytes written are accessible from
		// the child class.

		std::stringstream write_buffer;
		std::size_t       bytes_written = 0;

	private:
		// Override this function to customize the process of writing itself.
		// Return value should be the amount of bytes written.
		virtual ssize_t doWrite( void );

		// Override this function for a callback after each write cycle.
		virtual void afterWrite( void ) { };

		// Override this function to do something in the child class after
		// writing is done.
		virtual void writingDone( void ) { };

		int				  write_fd;
		FDStatus          write_status = FD_IDLE;
};
