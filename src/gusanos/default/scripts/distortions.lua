function distortions.pinch(x, y)
	local slen = (x * x + y * y)
	slen = (slen * slen) * 0.0005
	return (x / slen), (y / slen)
end

function distortions.lens(x, y, width)
	local delta = vector_distance( 0, 0, x, y )
	local radius = width/2
	local newLength = 0
	if delta < radius then
		newLength = radius - math.sqrt( radius * radius - delta * delta ) - delta
		newLength = newLength / delta
	end
	return x*newLength, y*newLength
end

function distortions.spin( degrees )
	return function( x, y, w, h )
		local delta = vector_distance( 0, 0, x, y )
		local radius = w/2
		local angle = vector_direction( 0, 0, x, y )
		if ( delta < radius ) then
			local factor = 1 - delta/radius
			local newX, newY= angle_vector( angle + degrees * factor, delta )
			return vector_diff( x, y, newX, newY )
		end
		return 0,0
	end
end

distortions.swirl = distortions.spin(90)

function distortions.ripple(waves, distance )
	return function( x, y, w, h )
		local delta = vector_distance( 0, 0, x, y )
		local radius = w/2
		local angle = vector_direction( 0, 0, x, y )
		if ( delta < radius ) then
			local factor = delta/radius
			return angle_vector( angle, -distance * math.sin(math.pi * waves * factor) )
		end
		return 0, 0
	end
end