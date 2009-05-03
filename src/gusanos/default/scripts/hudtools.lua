function hudtools.greenRedInterpol( factor )
	local r
	local g
	if factor < 0.5 then
		r = 512*factor
		g = 255
	else
		r = 255
		g = 255 - 512*(factor-0.5)
	end
	return color(r, g, 0)
end

function hudtools.drawHBar(bitmap, filled, x, y, width, height)
	gfx_set_alpha(160)
	bitmap:draw_box(x, y, x+width, y+height, 0)
	gfx_reset_blending()
	if filled > 0 then
		bitmap:draw_box(x+1, y+1, x+1+(width-2)*filled, y+height-1, hudtools.greenRedInterpol(1-filled))
	end
end