/*--------------------------------------------------------------------
Author		: Jan Henrik Sawatzki
License		: BSD
Repository	: https://github.com/JHSawatzki/Arduino-StreamCommandParser
--------------------------------------------------------------------*/

//#define STREAM_COMMAND_PARSER_DEBUG

#ifndef STREAM_COMMAND_PARSER_H
#define STREAM_COMMAND_PARSER_H

#include <Arduino.h>
	
///Error codes.
enum class StreamCommandParserErrorCode{
	///No error
	NoError,
	///Unknown command received.
	UnknownCommand,
	///Timeout before receiving the termination chars.
	Timeout,
	///Message buffer overflow.
	BufferOverflow,
};

class StreamCommandParser;

typedef class StreamCommand StreamCommand;
class StreamCommand {
public:
	StreamCommand(const char* cmd, void(*func)(arduino::Stream&, StreamCommandParser*));

	const char* command;
	void(*function)(arduino::Stream&, StreamCommandParser*);
	StreamCommand* next;
};

class StreamCommandParser {
public:
	StreamCommandParser(char* buffer, int16_t buffer_len, const char* term = "\r\n", const char* message_delim = ";", const char* param_delim = " ");

	/**
	 * \brief Adds a command handler (Uses a linked list)
	 * \param command 
	 */
	void AddCommand(StreamCommand* command);

	
	/**
	 * \brief Process a message and execute it if valid command are found.
	 * \param sender
	 * \param message 
	 */
	void Execute(arduino::Stream& sender, char* message);

	/**
	 * \brief Checks the Stream, reads the input buffer and calls message execution.
	 * \param sender
	 * \param timeout 
	 */
	void ProcessInput(arduino::Stream& sender, uint32_t timeout);

	/**
	 * \brief Sets a default handler can be used for a catch all or unrecognized commands
	 * \param function 
	 */
	void SetErrorHandler(void(*function)(arduino::Stream&, StreamCommandParserErrorCode, const char*));

	/**
	 * \brief Clears the buffer, and resets the indexes.
	 */
	void ClearBuffer();
	
	/**
	 * \brief Gets the next argument
	 * \return returns NULL if no argument is available
	 */
	char* Next();

	/**
	 * \brief Gets the termination characters
	 * \return returns the termination characters
	 */
	const char* Terminator();

	/**
	 * \brief Print all information of the stored commands
	 * \param interface
	 */
	void PrintDebugInfo(arduino::Stream& interface);

private:
	char* buffer_;
	int16_t buffer_len_;
	const char* term_;
	const char* message_delim_;
	const char* param_delim_;
	void(*error_handler_)(arduino::Stream&, StreamCommandParserErrorCode, const char*);
	int16_t buffer_pos_;
	char* last_message_token_;
	char* last_param_token_;
	int8_t term_pos_;
	StreamCommand* commands_head_;
	StreamCommand* commands_tail_;
	uint8_t commands_count_;
};

#endif