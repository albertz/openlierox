Hirudo Lua Test
---------------
Created by Jonanin

Oh wait... Hirudo is OLX!?
-=----=---=--=-===-=--=---=---=-

This a little project to test out some lua stuff for hirduo (e.g. performance of lots of projectiles [eventually...]).
However right now it is just basic lua support...

Currently have implemented a basic Lua interface as well as LuaArgument...


NOTE:
    Getting errors when running this from the command line?
    move test.lua to the executable directory
    (is made to run from Code::Blocks IDE (fixme)


NOTE:
    You may be interested in looking
    at the lua manual, which gives all C API functions (and lua library functions)
    and describes pretty much everything
    http://www.lua.org/manual/5.1/manual.html

-=----=---=--=-===-=--=---=---=-

FILES


test.lua         Testing file used by C++
LuaTest.cbp      Project file for Code::Blocks IDE. Good cross-platform opensource editor. Try it :)
readme.txt       Don't open this, it has viruses
src/*            source files of course!
bin/             final binary
obj              Compiled source
lua/include      Lua library include files
lua/lib/linux    Linux lua library
lua/lib/win      Windows lua library
lua/src          Lua source code.

-=----=---=--=-===-=--=---=---=-

[ ] TODO: add a compile.sh / makefile, currently only good way to compile is with code::blocks...
      must have support for compiling under x64 and x86 (there are different lua libraries to use!)

[ ] TODO: DOCUMENTATION !! D:

-=----=---=--=-===-=--=---=---=-
Random discussion/stuff

I think that this lua code should be kept here until:
    1. We are ready to switch to hirudo and drop compatibility with old lierox (when is that? are we ready even soon?)
    2. We have a clear idea of how lua modding should work

Might consider removing integer from LuaArgument (and everwhere in the lua project)... since lua only uses double,
it may be better.



IMPORTANT: when using lua, it is VERY important that when you modify the lua stack in your code,
you be sure to >>leave it how it was<< after you are done (i.e. pop unused items left on stack). Otherwise, you may mess stuff up bad!
(but to be safe, always use negative index values.)

