/*--------------------------------------------------------------------
Author		: Jan Henrik Sawatzki
License		: BSD
Repository	: https://github.com/JHSawatzki/Arduino-StreamCommandParser
--------------------------------------------------------------------*/

#include "StreamCommandParser.h"

StreamCommand::StreamCommand(const char* cmd, void(*func)(arduino::Stream&, StreamCommandParser*)) {
	command = cmd;
	hash = MurmurHash3_x86_32(STREAM_COMMAND_PARSER_HASH_SEED, &command, sizeof(command));
	function = func;
	next = NULL;
}


//Do nothing function
void DefaultErrorHandler(arduino::Stream& sender, StreamCommandParserErrorCode, const char* command) {
}

StreamCommandParser::StreamCommandParser(char* buffer, int16_t buffer_len, const char* term, const char* message_delim, const char* param_delim) {
	buffer_ = buffer;
	buffer_len_ = buffer != NULL && buffer_len > 0 ? buffer_len - 1 : 0; //string termination char '\0'
	term_ = term;
	message_delim_ = message_delim;
	param_delim_ = param_delim;
	error_handler_ = &DefaultErrorHandler;
	buffer_pos_ = 0;
	last_message_token_ = NULL;
	last_param_token_ = NULL;
	term_pos_ = 0;
	commands_head_ = NULL;
	commands_tail_ = NULL;
	commands_count_ = 0;
}

void StreamCommandParser::AddCommand(StreamCommand* command) {
	StreamCommand** cmd_head =& commands_head_;
	StreamCommand** cmd_tail =& commands_tail_;
	uint8_t *cmd_count = &commands_count_;

	command->next = NULL;
	if (*cmd_head == NULL) {
		*cmd_head = *cmd_tail = command;
	} else {
		(*cmd_tail)->next = command;
		*cmd_tail = command;
	}
	(*cmd_count)++;
}

void StreamCommandParser::Execute(Stream& sender, char* message) {
	char* message_token = strtok_r(message, message_delim_, &last_message_token_);
	while (message_token != NULL) {
		char* command = strtok_r(message_token, param_delim_, &last_param_token_);
		if (command != NULL) {
			uint32_t hash = MurmurHash3_x86_32(STREAM_COMMAND_PARSER_HASH_SEED, &command, sizeof(command));
			boolean matched = false;
			StreamCommand* cmd;
			for (cmd = commands_head_; cmd != NULL; cmd = cmd->next) {
				//if (strncmp(command, cmd->command, strlen(cmd->command) + 1) == 0) {
				if (hash == cmd->hash) {
					cmd->function(sender, this);
					matched = true;
					break;
				}
			}
			if (!matched) {
				(*error_handler_)(sender, StreamCommandParserErrorCode::UnknownCommand, command);
			}
		}
		message_token = strtok_r(NULL, message_delim_, &last_message_token_);
	}
}

void StreamCommandParser::ProcessInput(Stream& sender, uint32_t timeout) {
	uint32_t time_checker = 0;
	while (sender.available()) {
   		time_checker = millis();
		buffer_[buffer_pos_] = sender.read();
    	++buffer_pos_;
		if (buffer_pos_ >= buffer_len_){
			//Call ErrorHandler due to BufferOverflow
			(*error_handler_)(sender, StreamCommandParserErrorCode::BufferOverflow, buffer_);
			ClearBuffer();
			return;
		}

		//Test for termination chars (end of the message)
		buffer_[buffer_pos_] = '\0';
		if (strstr(buffer_, term_) != NULL) {
			//Return the received message
			buffer_[buffer_pos_ - strlen(term_)] = '\0';
			break;
		}
	}
	//No more chars aviable yet

	//Check for communication timeout
	if ((millis() - time_checker) > timeout) {
		//Call ErrorHandler due Timeout
		(*error_handler_)(sender, StreamCommandParserErrorCode::Timeout, buffer_);
		ClearBuffer();
		return;
	}

	this->Execute(sender, buffer_);

	ClearBuffer();
}

void StreamCommandParser::SetErrorHandler(void(*function)(arduino::Stream&, StreamCommandParserErrorCode, const char*)) {
	error_handler_ = function;
}

void StreamCommandParser::ClearBuffer() {
	buffer_[0] = '\0';
	buffer_pos_ = 0;
	term_pos_ = 0;
}

char* StreamCommandParser::Next() {
	return strtok_r(NULL, param_delim_, &last_param_token_);
}

const char* StreamCommandParser::Terminator() {
	return term_;
}

void StreamCommandParser::PrintDebugInfo(arduino::Stream& interface) {
	interface.println("*** StreamCommandParser DEBUG INFO ***");
	interface.print("Commands stored:\t");
	interface.println(commands_count_);
	StreamCommand* cmd;
	for (cmd = commands_head_; cmd != NULL; cmd = cmd->next) {
		interface.print("\t");
		interface.print(cmd->hash);
		interface.print("\t");
		interface.println(cmd->command);
	}
	interface.println();
	interface.println("*******************");
	interface.println();
}