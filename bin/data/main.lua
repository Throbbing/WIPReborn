--require "init"
local app = require "lib"

function genertate_component_by_name(component_name)
	s = "local _ = require \""..component_name.."\";return _:new()"
	return loadstring(s)
end

function init()
end

function main_logic()
	--app.init()
end

function debug_draw()

end


function main ()

	
end

--this function is not in use in c++ or lua
--so never called
function finalization()

end

