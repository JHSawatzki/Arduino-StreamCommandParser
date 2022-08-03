/*--------------------------------------------------------------------
Author		: Jan Henrik Sawatzki
License		: BSD
Repository	: https://github.com/JHSawatzki/Arduino-StreamCommandParser
--------------------------------------------------------------------*/

//#define STREAM_COMMAND_PARSER_DEBUG

#ifndef STREAM_COMMAND_PARSER_H
#define STREAM_COMMAND_PARSER_H

#include <Arduino.h>

//Do nothing function
void DefaultErrorHandler(Stream& sender, ErrorCode, const char* command) {}

class StreamCommandParser;

typedef class StreamCommand StreamCommand;
class StreamCommand {
public:
	StreamCommand(const char* cmd, void(*func)(Stream&, StreamCommandParser*)) : command(cmd), function(func), next(NULL)) {
	}

	const char* command;
	void(*function)(Stream&, StreamCommandParser*);
	StreamCommand* next;
};

class StreamCommandParser {
public:
	StreamCommandParser(char* buffer, int16_t buffer_len, const char* term = "\r\n", const char* delim = " ") :
		buffer_(buffer),
		buffer_len_(buffer!=NULL && buffer_len > 0 ? buffer_len - 1 : 0), //string termination char '\0'
		term_(term),
		delim_(delim),
		error_handler_(DefaultErrorHandler),
		buffer_pos_(0),
		last_token_(NULL), 
		term_pos_(0),
		commands_head_(NULL),
		commands_tail_(NULL),
		commands_count_(0)
	{
	}

	/**
	 * \brief Adds a command handler (Uses a linked list)
	 * \param command 
	 */
	void AddCommand(StreamCommand* command);

	/**
	 * \brief Checks the Serial port, reads the input buffer and calls a matching command handler.
	 * \return SERIAL_COMMANDS_SUCCESS when successful or SERIAL_COMMANDS_ERROR_XXXX on error.
	 */
	void ProcessInput(Stream& sender, unsinged long timeout);

	/**
	 * \brief Sets a default handler can be used for a catch all or unrecognized commands
	 * \param function 
	 */
	void SetErrorHandler(void(*function)(Stream&, ErrorCode, const char*));
	
	///Error codes.
	enum class ErrorCode{
		///No error
		NoError,
		///Unknown command received.
		UnknownCommand,
		///Timeout before receiving the termination chars.
		Timeout,
		///Message buffer overflow.
		BufferOverflow,
	};

	/**
	 * \brief Clears the buffer, and resets the indexes.
	 */
	void ClearBuffer();
	
	/**
	 * \brief Gets the next argument
	 * \return returns NULL if no argument is available
	 */
	char* Next();

private:
	char* buffer_;
	int16_t buffer_len_;
	const char* term_;
	const char* delim_;
	void(*error_handler_)(Stream&, ErrorCode, const char*);
	int16_t buffer_pos_;
	char* last_token_;
	int8_t term_pos_;
	StreamCommand* commands_head_;
	StreamCommand* commands_tail_;
	uint8_t commands_count_;
};

#endif
