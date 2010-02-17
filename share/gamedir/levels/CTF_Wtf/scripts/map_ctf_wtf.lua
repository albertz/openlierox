--Change the function name to "map_<mapname>.init()", leave rest untouched
--Change filename to map_<mapname>.lua

--for a map called 'flaggrab' this would be:
--function map_flaggrab.init()
--and
--map_flaggrab.lua

--simple, right?

--
function map_ctf_wtf.init()
	ctf.init()
	ctf.start()


--This loads the different objects. Every team needs a flag, base and trail.
--Just copy/paste the existing ones and change gravity etc. to your liking,
--but leave the script(....) stuff intact

	flag_b = load_particle("flag_b.obj")
	flag_r = load_particle("flag_r.obj")

	base_b = load_particle("base_b.obj")
	base_r = load_particle("base_r.obj")

	trail_b = load_particle("trail_b.obj")
	trail_r = load_particle("trail_r.obj")


--This loads the scoreboard sprites. Every team needs one.

	score_b = sprites_load("score_b.bmp")
	score_r = sprites_load("score_r.bmp")


--This actually creates the bases. Usage:

--	ctf.make_base(<flag object>, <base object>, <trail object>, <scoreboard sprite>, <team name>, <X position>, <Y position>, <Red>, <Blue>)

--X and Y position refer to pixels on the map, and define where the bases are
--Red, green and blue aren't used yet, but will be later. Team name has to be surrounded by "

	ctf.make_base(flag_b, base_b, trail_b, score_b, " ", 1007, 132, 0, 0, 255)
	ctf.make_base(flag_r, base_r, trail_r, score_r, " ", 182, 132, 255, 0, 0)


--The end.
end