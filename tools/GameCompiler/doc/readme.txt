Liero Xtreme Game Compiler v0.31
By Auxiliary Software 2002-2003

http://galileo.spaceports.com/~lierox/

15th July 2003
-------------------------------------



Contents
1) Description
2) How to use it
3) How to play a mod
4) Troubleshooting
5) Contact



1) Description
The game compiler for Liero Xtreme compiles the game scripts into a single file for use in Liero Xtreme.
Script files are simple text files with an ini type of format.

The first file is main.txt. This file must exist and use this name. This main file has info about the ninja rope. Eventually, this file will contain more info like worm details (speed, jumping, carving, etc) and other stuff.
This main file points to other script files that contain weapons.

Weapon files hold basic information about the weapon such as name, class, firing details, and the projectile(s) the weapon shoots out.
The projectile section in this weapon file contains information about the projectiles to shoot as well as the script file that contains information about the projectile.

Projectile files hold information about the behaviour of the projectile. The projectile has three main events. Hitting the world (dirt & rock), Hitting a worm and a timer event.
When the projectile triggers one of these 3 events, the details in each section tell the projectile how to behave. Details are action (injure, explode, bounce), shooting more projectiles and other misc stuff like shaking the viewport, etc.

The projectile section in the projectile script file contains information about spawning projectiles from this projectile. This is a recursive process that can go on and on depending on how you want your projectiles to act.

If you create any additional sounds or graphics for your mod, put them in the sfx or gfx folder in your mod directory that goes in your Liero Xtreme directory. That is, don't put the graphics and sounds with the scripts. The game compiler doesn't need to use them, the actual game itself does!


The above information is just an explanation of the process. Eventually, i'll create a full document that contains information on every single variable that can be modified.

Also, a caution: The game and the game compiler are both in alpha stages. I will be added/modifying some of the variables in the scripts to provide better functionality and to also support more kinds of weapons (like the laser).
Don't get too angry if you've made modifications and find out that you may need to alter the script files to be used with future versions of the game and game compiler.
I don't plan any huge changes, but there still will be changes occuring.



3) How to use it
Put all your script files into a directory.
For this example, i'll use a directory name base (which is the default mod used).
To compile the mod, run the following command:

gc base script.lgs

Liero Xtreme looks for directories containing a file called script.lgs, so you should use this filename.



4) How to play a mod
Create a directory in the Liero Xtreme directory with whatever name you want. Put the script.lgs file, that you just compiled, in this directory.

Eg:
LieroX\base\script.lgs
(Where 'LieroX' is the directory holding Liero Xtreme and 'base' is the mod directory)

Put any additional graphics in the gfx dir in your mod dir, and sounds in the sfx dir.
Eg:
LieroX\base\gfx
LieroX\base\sfx

Next, start Liero Xtreme and click on 'Local play'. Click on the arrow on the 'Mod' combo box and select your mod name. The name shown will be the text you set for the 'Name' variable in the main.txt file in the scripts.

Next, play your mod!



6) Contact
Contact me at:
jasonb___@hotmail.com
(Use 'LieroX' as the subject name to help me out)