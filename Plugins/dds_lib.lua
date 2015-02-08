
JSON = (loadfile "JSON.lua")()
Message = {}
Message.__index = Message

function Message.create(src, dest, plugin_dest, content, dt)
	local obj = {}
	setmetatable(obj, Message)
	obj.src = src
	obj.dest = dest
	obj.plugin_dest = plugin_dest
	obj.content = content
	obj.dt = dt
	return obj
end


function Message.tostring()
	return 
