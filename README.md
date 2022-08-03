# StreamCommandParser
An Arduino library to tokenize and parse commands received over a Stream (see https://www.arduino.cc/reference/en/language/functions/communication/stream/) type interface.

Based on :
https://github.com/ppedro74/Arduino-SerialCommands

Inspired by (original version):
https://github.com/scogswell/ArduinoSerialCommand

This library is for handling text commands over a Stream type interface.

Goals:
* Small footprint
* No dynamic memory allocation
* Stream class's interfaces
* Multiple interfaces callback methods
* Custom Command termination default is `CR & LF`
* Custom Arguments Delimeter default is `SPACE`
* Optional _OneKey_ commands that are single characters without the need for
    additional command termination

## StreamCommand object

StreamCommand creates a binding between a const string token with a callback function.

Note:
StreamCommand is used in a Linked List. Do not share StreamCommand objects between StreamCommandParser objects.

```c++
#include <StreamCommand.h>

void cmd_hello(Stream& sender) {
	sender.println("HELLO from arduino!");
}

StreamCommand cmd_hello_("hello", cmd_hello);
```

### OneKey commands

When sending commands from a keyboard, it is often useful to be able to use
single keypresses to perform a specific action, like adjusting a motor speed up
or down, for example.

To make any command a _OneKey_ command, instantiate the command with a **single
character** command string, and add `true` as a third argument:

```c++
StreamCommand cmd_faster_("+", cmd_faster, true);
StreamCommand cmd_slower_("-", cmd_slower, true);
```

The `cmd_faster` and `cmd_slower` command functions has the same signature as
for multiple character commands, except that arguments are not allowed for
_OneKey_ commands (they don't make sense), so calling `sender->Next()` is not
supported.

_OneKey_ commands are only active when they come in as the _first_ character,
either on startup, or after processing a previous command. In other words, if
another command string contains a `+` like `set+go` for example, the command
callback for the one key `+` command will not be called when `set+go` is
entered, and the `set+go` command callback will be called as expected.

Any multi character command that _starts_ with the same character as a one key
command, will however never be activated as the one key command will always
consume the single character command first, i.e:

Multi char command: `up`  
OneKey command: `u`

The `up` command will never be executed since the `u` will be consumed by the
one key command first, which leaves the `p` as the remainder for the command
which will not match (unless of course there *is* in fact a `p` one key or multi char
command).

## StreamCommandParser object

StreamCommandParser is the main object creates a binding between a Stream type interface and a list of StreamCommand

```c++
#include <StreamCommand.h>

//Create a 32 bytes static buffer to be used exclusive by StreamCommandParser object.
//The size should accomodate command token, arguments, termination sequence and string delimeter \0 char.
char stream_command_buffer_[32];

//Creates StreamCommandParser object attached to Serial
//working buffer = stream_commands_buffer_
//command delimeter: Cr & Lf
//argument delimeter: SPACE
StreamCommandParser stream_command_parser_(&Serial, stream_commands_buffer_, sizeof(stream_commands_buffer_), "\r\n", " ");
```

### Arduino setup

```c++
void setup() {
	Serial.begin(115200);
	stream_command_parser_.AddCommand(&cmd_hello);
}
```
### Default handler

```c++
void cmd_unrecognized(Stream& sender, const char* cmd) {
	sender.print("ERROR: Unrecognized command [");
	sender.print(cmd);
	sender.println("]");
}
void setup() {
	stream_command_parser_.SetDefaultHandler(&cmd_unrecognized);
}
```
### Arduino loop

```c++
void loop() {
	stream_command_parser_.ReadSerial();
}
```

### Multiple Serial Ports
```c++
#include <StreamCommandParser.h>

char stream_commands_0_buffer_[32];
StreamCommandParser stream_command_parser_0_(&Serial, stream_commands_0_buffer_, sizeof(stream_commands_0_buffer_));

char stream_commands_1_buffer_[32];
StreamCommandParser stream_command_parser_1_(&Serial1, stream_commands_1_buffer_, sizeof(stream_commands_1_buffer_));

void cmd_hello(Stream& sender) {
	sender.println("HELLO from arduino!");
}

void cmd_unrecognized(Stream& sender, const char* cmd) {
	sender.print("ERROR: Unrecognized command [");
	sender.print(cmd);
	sender.println("]");
}

SerialCommand cmd_hello_0_("hello", cmd_hello);
SerialCommand cmd_hello_1_("hello", cmd_hello);

void setup() {
    Serial.begin(115200);
	stream_command_parser_0_.AddCommand(&cmd_hello_0_);
	stream_command_parser_0_.SetDefaultHandler(&cmd_unrecognized);
	
    Serial1.begin(115200);
	stream_command_parser_1_.AddCommand(&cmd_hello_1_);
	stream_command_parser_1_.SetDefaultHandler(&cmd_unrecognized);
}

void loop() {
	stream_command_parser_0_.ReadSerial();
	stream_command_parser_1_.ReadSerial();
}
```
