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

static int f_listen(lua_State* L) {
	MessageFrame read_frame;
	bool success = listen_to_discord(&read_frame);
	if (!success) {
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, read_frame.message);
	return 1;
}

static int f_clear(lua_State* L) {
	connection = (BaseConnection) {0};
	return 0;
}

static const struct luaL_Reg lib[] = {
	{"init",     f_init},
	{"send",   f_send},
	{"listen",     f_listen},
	{"clear",     f_clear},
	{NULL, NULL}
};

int luaopen_lite_xl_discord_socket(lua_State* L, void* XL) {
	lite_xl_plugin_init(XL);
	luaL_newlib(L, lib);
	return 1;
}
