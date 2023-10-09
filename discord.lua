-- mod-version:3
local socket = require "plugins.discord-presence.discord_socket"
local json = require "libraries.json"


local discord = {}
local handlers = {}

local Opcode = {
	Handshake = 0,
	Frame = 1,
	Close = 2,
	Ping = 3,
	Pong = 4
}

local nonce = 0
local pid = system.get_process_id()

local function handle_response(response)
if response ~= nil then
		response = json.decode(response)
		if response.code ~= nil then
			if handlers.disconnect ~= nil then
				handlers.disconnect(response.code, response.message)
			end
			return
		end

		if response.evt ~= nil then
			local event = response.evt:lower()
			if handlers[event] then
				(handlers[event])(response)
			end
		end

	end
end

function discord.init(client_id)
	local success = socket.init()
	if not success then
		return false
	end

	local handshake = ([[{
		"v": 1,
		"client_id": "%s"
	}]]):format(client_id)

	success = socket.send(Opcode.Handshake, handshake)
	return success
end

function discord.poll()
	local response = socket.listen()
	handle_response(response)
end

function discord.on_event(event, callback)
	handlers[event] = callback
end

function discord.shutdown()
	socket.send(Opcode.Close, "{}")
	handlers = {}
	socket.clear()
end

function discord.update(new_status)
	local message = {
		cmd = "SET_ACTIVITY",
		args = {
			pid = pid,
			activity = {
				state = new_status.state,
				details = new_status.details,
				timestamps = {
					start = new_status.start_time,
				},
				assets = {
					large_image = new_status.large_image
				},
				instance = false
			}
		},
		nonce = nonce
	}
	if new_status.small_image ~= nil then
		message.args.activity.assets.small_image = new_status.small_image
	end

	local message_string = json.encode(message)
	socket.send(Opcode.Frame, message_string)
	nonce = nonce + 1
end

setmetatable(discord, {__gc = function() discord.shutdown() end})

return discord
