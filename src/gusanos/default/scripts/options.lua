function options.init(dest)
	local controlsWin = gui_load_xml("controls", dest)
	local controlsFirew = controlsWin:child("controls-fire")
	controlsFirew:set_lock(true)
	function controlsFirew:onKeyDown(k)
		if self:is_active() then
			self:set_text(key_name(k))
			self:deactivate()
			return true
		end
	end
	
	return controlsWin
end
