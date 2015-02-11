
JSON = (loadfile "json.lua")()
queue = {}
function hasMessage()
	lock_msg_queue()
	local result = (table.getn(queue) == 0)
	unlock_msg_queue()
	return result
end

function getMessage()
	lock_msg_queue()
	local result = table.remove(queue)
	unlock_msg_queue()
	return JSON.encode(result)
end

function addMessage(msg)
	table.insert(queue, msg)
end

function send(payload)
	send_message(payload)
end

function send_info(src, dest, plugin_dest, content)
	send(json.encode({["src"] = src, ["dest"] = dest, ["pluginDest"] = plugin_dest, ["content"] = content}))
end

