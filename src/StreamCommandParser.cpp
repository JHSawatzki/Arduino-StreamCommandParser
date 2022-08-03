/*--------------------------------------------------------------------
Author		: Jan Henrik Sawatzki
License		: BSD
Repository	: https://github.com/JHSawatzki/Arduino-StreamCommandParser
--------------------------------------------------------------------*/

#include "StreamCommandParser.h""

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

void StreamCommandParser::ProcessInput(Stream& sender, unsinged long timeout) {
	unsigned long time_checker = 0;
	while (sender.available()) {
   		time_checker = millis();
		buffer_[buffer_pos_] = interface.read();
    	++buffer_pos_;
		if (buffer_pos_ >= buffer_len_){
			//Call ErrorHandler due to BufferOverflow
			(*error_handler_)(&sender, ErrorCode::BufferOverflow, buffer_);
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
		(*error_handler_)(&sender, ErrorCode::Timeout, buffer_);
		ClearBuffer();
		return;
	}
	
	char* command = strtok_r(buffer_, delim_, &last_token_);
	if (command != NULL) {
		boolean matched = false;
		int cx;
		StreamCommand* cmd;
		for (cmd = commands_head_, cx = 0; cmd != NULL; cmd = cmd->next, cx++) {
			if (strncmp(command, cmd->command, strlen(cmd->command) + 1) == 0) {
				cmd->function(&sender, this);
				matched = true;
				break;
			}
		}
		if (!matched) {
			(*error_handler_)(&sender, ErrorCode::UnknownCommand, command);
		}
	}

	ClearBuffer();
}

void StreamCommandParser::SetDefaultHandler(void(*function)(StreamCommandParser*, const char*)) {
	default_handler_ = function;
}

void StreamCommandParser::ClearBuffer() {
	buffer_[0] = '\0';
	buffer_pos_ = 0;
	term_pos_ = 0;
}

char* StreamCommandParser::Next() {
	return strtok_r(NULL, delim_, &last_token_);
}

