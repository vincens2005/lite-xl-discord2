#ifndef DISCORD
#define DISCORD
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_FRAME_SIZE 64 * 1024

typedef struct {
	struct sockaddr_un pipeaddr;
	int sock;
	bool open;
} BaseConnection;

typedef struct {
	void (*ready)();
	void (*disconnected)(int errorCode, const char* message);
	void (*errored)(int errorCode, const char* message);
} DiscordEventHandlers;

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

static const char* get_temp_path() {
	const char* temp = getenv("XDG_RUNTIME_DIR");
	temp = temp ? temp : getenv("TMP");
	temp = temp ? temp : getenv("TEMP");
	temp = temp ? temp : "/tmp";
	return temp;
}

bool make_connection(BaseConnection* connection) {
	connection->pipeaddr.sun_family = AF_UNIX;
	connection->sock = socket(AF_UNIX, SOCK_STREAM, 0);
	const char* temp_path = get_temp_path();

	if (connection->sock == 0)
		return false;

	fcntl(connection->sock, F_SETFL, O_NONBLOCK);

	for (int i = 0; i < 10; i++) {
		snprintf(connection->pipeaddr.sun_path, sizeof(connection->pipeaddr.sun_path), "%s/discord-ipc-%d", temp_path, i);
		int err = connect(connection->sock, (struct sockaddr*)&(connection->pipeaddr), sizeof(connection->pipeaddr));
		if (err == 0) {
			connection->open = true;
			return true;
		}
	}

	// TODO mark closed
	return false;
}

bool connection_write(BaseConnection* connection, const void* data, size_t len) {
	if (connection->sock == -1)
		return false;

	ssize_t sent_bytes = send(connection->sock, data, len, 0);

	if (sent_bytes < 0) {
		exit(1);
		// TODO freak out!!11! (mark closed)
	}

	return sent_bytes == (ssize_t)len;
}

bool connection_read(BaseConnection* connection, void* data, size_t len) {
	int res = (int)recv(connection->sock, data, len, 0);

	if (res <= 0) {
		return false;
	}

	return res == (int)len;
}


#endif
