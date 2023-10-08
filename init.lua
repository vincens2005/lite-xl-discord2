-- mod-version:3
local core = require "core"
local config = require "core.config"
local common = require "core.common"
local command = require "core.command"
local discord = require "plugins.discord.discord_socket"

local status_template = [[{
	"cmd": "SET_ACTIVITY",
	"args": {
	"pid": %d,
		"activity": {
			"state": "this is a test",
			"details": "testing",
			"instance": false
		}
	},
	"nonce": %d
}]]

local handshake = [[{
	"v": 1,
	"client_id": "749282810971291659"
}]]

local nonce = 0

core.log("initializing discord....!>1.!")

local status = discord.init()
if status then
	discord.send(0, handshake)
else
	core.log("OH NO IT DIND WORK!!!!!")
end

core.add_thread(function()
	while status do
		coroutine.yield(config.project_scan_rate)
		local response = discord.listen()
		if response ~= nil then
			core.log(response)
			discord.send(1, status_template:format(system.get_process_id(), nonce))
			nonce = nonce + 1
		end
	end
end)
