
desc = """
SDL_BlitSurface is nowhere allowed outside GfxPrimitives.cpp. This is because it is insafe in general, esp. when dealing with different pixel formats, with alpha, with colorkeys or any mixture of those.

There are the safe DrawImageAdv (and co) functions which might use SDL_BlitSurface or other optimized code path for specific cases. And they work always.
"""

for f in code_files():
	if os.path.basename(f) == "GfxPrimitives.cpp": continue
	
	if "SDL_BlitSurface" in f.identifiers():
		fail()
