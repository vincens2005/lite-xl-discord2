#include <string.h>
#include "discord.h"

#define LITE_XL_PLUGIN_ENTRYPOINT
#include "lite_xl_plugin_api.h"

BaseConnection connection;


bool listen_to_discord(MessageFrame* read_frame) {
	bool read = connection_read(&connection, read_frame, sizeof(MessageFrameHeader));

	if (!read)
		return false;

	if (read_frame->header.length > 0) {
		read = connection_read(&connection, read_frame->message, read_frame->header.length);
	}

	read_frame->message[read_frame->header.length] = '\0';
	return read;
}


bool send_to_discord(int opcode, const char* message, size_t message_len) {
	if (!connection.open)
		return false;

	MessageFrame frame = {
		{opcode, message_len},
	};

	memcpy(frame.message, message, message_len + 1);

	return connection_write(&connection, &frame, sizeof(MessageFrameHeader) + message_len);
}

static int f_init(lua_State* L) {
	bool success = make_connection(&connection);
	lua_pushboolean(L, success);
	return 1;
}

static int f_send(lua_State* L) {
	int opcode = luaL_checkinteger(L, 1);
	size_t len;
	const char* message = luaL_checklstring(L, 2, &len);
	bool success = send_to_discord(opcode, message, len);
	lua_pushboolean(L, success);
	return 1;
}

static int f_listen(lua_State *L) {
	MessageFrame read_frame;
	bool success = listen_to_discord(&read_frame);
	if (!success) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, read_frame.message);
	return 1;
}

static const struct luaL_Reg lib[] = {
	{"init",     f_init},
	{"send",   f_send},
	{"listen",     f_listen},
	{NULL, NULL}
};

int luaopen_lite_xl_discord_socket(lua_State* L, void* XL) {
	lite_xl_plugin_init(XL);
	luaL_newlib(L, lib);
	return 1;
}

/*
int main() {
	// sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

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
*/
