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

function hudtools.blueRedInterpol( factor )
	local r
	local b
	if factor < 0.5 then
		b = 255
		r = 228*factor
	else
		r = 72 - 512*(factor-0.5)
		b = 228
	end
	return color(r, 68, b)
end

function hudtools.drawHBar(bitmap, filled, x, y, width, height)
	gfx_set_alpha(160)
	bitmap:draw_box(x, y, x+width, y+height, 0)
	gfx_reset_blending()
	if filled > 0 then
		bitmap:draw_box(x+1, y+1, x+1+(width-2)*filled, y+height-1, hudtools.greenRedInterpol(1-filled))
	end
end

function hudtools.drawLieroHBar(bitmap, filled, x, y, width, height)
	gfx_set_alpha(160)
		bitmap:draw_box(x, y, x+width, y+height, 0)
	gfx_reset_blending()

	bitmap:draw_box(x+1, y+1, x+1+(width-2)*filled, y+height-1, hudtools.greenRedInterpol(1-filled))
end

function hudtools.drawLieroABar(bitmap, filled, x, y, width, height)
	gfx_set_alpha(160)
		bitmap:draw_box(x, y, x+width, y+height, 0)
	gfx_reset_blending()

	bitmap:draw_box(x+1, y+1, x+1+(width-2)*filled, y+height-1, hudtools.blueRedInterpol(1-filled))
end

function hudtools.drawSelected(bitmap, x, y, width, height)
	bitmap:draw_box(x,y-1, x+width, y+5,0)
	bitmap:draw_box(x+1,y-2,x+width-1,y+6,0)
end

function hudtools.cogDrawAmmo(bitmap, filled, x, y )
	if filled > 0 then
		bitmap:draw_box(x, y-30*filled, x+1, y, color(10,10,255))
	end
end

function hudtools.simpleBox(bitmap, x, y, width, height)
	bitmap:draw_box(x, y, x + width, y + height)
end

function hudtools.cogDrawHealth(bitmap, health, x, y )
	local filled = health / 100
	local r,g,b = 0,255,0
	
	local limit = filled*15*2+1
	if filled <= 0 then limit = 0 r,g,b = 255,0,0 end
	for i = 0, 30, 2 do
		if i < limit then
			gfx_set_alpha(255)
		else
			if filled <= 0 then
				gfx_set_alpha(255)
			else
				gfx_set_alpha(100)
			end
		end
		bitmap:draw_box(x, y-i, x+10, y-i, color(r, g, b))
	end
	gfx_reset_blending()

	fonts.minifont:render(bitmap, floor(health), x-1, y+2)
end
