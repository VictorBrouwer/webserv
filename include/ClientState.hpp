#pragma once

enum class ClientState
{
	LOADING,
	READING_DONE,
	READY_TO_SEND,
	SENDING_DONE,
	ERROR,
};
