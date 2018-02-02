local BaseComponent = require "BaseComponent"
local TestComponent = BaseComponent:new()

function TestComponent:new()
	local o = {}
	setmetatable(o,self)
	self.__index = self

	--add data member here
	
	return o
end

function TestComponent:init()
	print('[TestComponent]:System init...')
end

function TestComponent:on_begin_contact(sprite)
	print('[TestComponent]:on_begin_contact...')
end

function TestComponent:on_contact(sprite)
	print('[TestComponent]:on_contact...')
end

function TestComponent:on_end_contact(sprite)
	print('[TestComponent]:on_end_contact...')
end

function TestComponent:update(dt)
	print('[TestComponent]:update...')
end

function TestComponent:fix_update(dt)
   print('[TestComponent]:fix_update')
end

function TestComponent:destroy()
	print('[TestComponent]:destroy...')
end

function TestComponent:start()
	print('[TestComponent]:start...')
end

function TestComponent:end()
	print('[TestComponent]:end...')
end


return TestComponent