#ifndef CLIENT_STATE
#define CLIENT_STATE

enum class ClientState
{
	LOADING,
	READING_DONE,
	READY_TO_SEND,
	SENDING,
	KEEP_ALIVE,
	REMOVE_CONNECTION,
	ERROR,
};

#endif