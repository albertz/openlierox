function hudtools.greenRedInterpol( factor )
	local r
	local g
      local b
	if factor > 0 then
		r = factor*49
		g = factor*224 + 30
		b = factor*224 + 30
	end
	return color(r, g, b)
end

function hudtools.drawHBar(bitmap, filled, x, y, width, height)
	gfx_set_alpha(100)
	bitmap:draw_box(x, y, x+width, y+height, 0)
	gfx_reset_blending()
	if filled > 0 then
		bitmap:draw_box(x+1, y+1, x+1+(width-2)*filled, y+height-1, hudtools.greenRedInterpol(filled))
	end
end