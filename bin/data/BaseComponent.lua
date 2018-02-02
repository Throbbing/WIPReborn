local BaseComponent = 
{
	
}

function BaseComponent:new(o)

	print('Base')
	local o = o or {}
	--对于表，只是传递地址拷贝
	setmetatable(o,self)
	self.__index = self
	o.host_obj = nil
	return o
end

--called when added to GameObject by GameObject
function BaseComponent:bind_host(host_obj)
	self.host_obj = host_obj
end

function BaseComponent:get_host()
	return self.host_obj
end

function BaseComponent:init()
	print('[BaseComponent]:System init...')
end

function BaseComponent:on_begin_contact(sprite)
	print('[BaseComponent]:on_begin_contact...')
end

function BaseComponent:on_contact(sprite)
	print('[BaseComponent]:on_contact...')
end

function BaseComponent:on_end_contact(sprite)
	print('[BaseComponent]:on_end_contact...')
end

function BaseComponent:update(dt)
	print('[BaseComponent]:update...')
end

function BaseComponent:fix_update(dt)
   print('[BaseComponent]:fix_update')
end

function BaseComponent:destroy()
	print('[BaseComponent]:destroy...')
end

function BaseComponent:start()
	print('[BaseComponent]:start...')
end

function BaseComponent:end()
	print('[BaseComponent]:end...')
end

function BaseComponent:Base()
	print('BaseComponent...')
end

return BaseComponent