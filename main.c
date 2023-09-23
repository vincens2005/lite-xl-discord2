#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "discord.h"

#define MAX_FRAME_SIZE 64 * 1024

char status_template[] = "{"
	"\"cmd\": \"SET_ACTIVITY\","
	"\"args\": {"
	"\"pid\": %d,"
		"\"activity\": {"
			"\"state\": \"this is a test\","
			"\"details\": \"testing\","
			"\"instance\": false"
		"}"
	"},"
	"\"nonce\": %d"
"}";

char handshake[] = "{"
	"\"v\": 1,"
	"\"client_id\": \"749282810971291659\""
"}";

int nonce = 0;

typedef enum {
	Handshake = 0,
	Frame = 1,
	Close = 2,
	Ping = 3,
	Pong = 4
} Opcode;

typedef struct {
	Opcode opcode;
	uint32_t length;
} MessageFrameHeader;

typedef struct {
	MessageFrameHeader header;
	char message[MAX_FRAME_SIZE - sizeof(MessageFrameHeader)];
} MessageFrame;

void listen_to_discord(BaseConnection* connection) {
	MessageFrame read_frame;

	bool read = connection_read(connection, &read_frame, sizeof(MessageFrameHeader));
	if (read_frame.header.length > 0) {
		read = connection_read(connection, &read_frame.message, read_frame.header.length);
	}
	read_frame.message[read_frame.header.length] = '\0';
	printf("discord says:\n%s\n", read_frame.message);
}



int main() {
	// sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
	BaseConnection connection;
	make_connection(&connection);
	printf("discord path: %s\r\n", connection.pipeaddr.sun_path);

	// send the handshake
	MessageFrame frame = {
		{0, sizeof(handshake) - 1},
	};

	memcpy(frame.message, handshake, sizeof(handshake));
	printf("we're saying:\n%s\n", frame.message);

	connection_write(&connection, &frame, sizeof(MessageFrameHeader) + sizeof(handshake) - 1);
	printf("we sent the handshake\n");
	sleep(1);

	listen_to_discord(&connection);

	// send the status
	char* status = malloc(sizeof(status_template) + 41);
	sprintf(status, status_template, getpid(), nonce++);

	unsigned long status_len = strlen(status);

	frame.header = (MessageFrameHeader){1, status_len};
	memcpy(frame.message, status, status_len);
	connection_write(&connection, &frame, sizeof(MessageFrameHeader) + status_len);
	printf("we just sent the status\n %s\n to discord!!1!\n", status);

	sleep(1);
	listen_to_discord(&connection);

	sleep(200);

	return 0;
}
