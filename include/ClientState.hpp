#ifndef CLIENT_STATE
#define CLIENT_STATE

enum class ClientState
{
	LOADING,
	READING_DONE,
	READY_TO_SEND,
	SENDING_DONE,
	ERROR,
};

#endif