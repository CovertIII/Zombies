Zombies!
========

This is a fun one player game where you try to rescue the people from the blood
thirsty Zombies.

Installation 
-----------

Eventually the plan is to port this from GLUT to SDL so that it will be much
easier to make this a mac app.  Right now, I made mac app out of this, but it is
of a much earlier version.  So for the time being, you have to compile this.

There's the required libs:
 
 * libpng
 * libfreetype2
 * libsqlite3
 * libvorbis
 * libogg
 * libvorbisfile
 * OpenGL
 * GLUT
 * OpenAL

On a mac OpenGL, OpenAL, GLUT, and sqlite3 are preinstalled.  You only have to
get libpng, and the sound libraries.  Also, if you are not on a mac you will
have to change the make file suitable to your system (sorry no ./configure!)

Game play
--------

Upon start up, either select you name from the list with the arrow keys, or push
the up arrow key until you see the '|' character, type in your name and hit
enter.  After you play a few times, you can use the left and right arrow to view
the high scores for the game and level.
 
You play as the circle hero with red pants.  You job is to rescue the yellow
people from the green zombies.  To do this you must bring a certain amount of
them into the safe zone for each level.  The upper left corner of the screen has
empty circles that will fill up as you rescue more people.  You bring the people
into the safe zone by bumping into them, which will attach you to them with
rope, then pulling them until they are entirely in the safe zone, at which point
you will automatically let go of them.  You can manually sever the connection by
pushing the space bar.

The lower left corner of the screen indicates how many lives you have left.  You
start with three lives.  If you get infected by a Zombie or if too many yellow
people are infected, then you loose a life.  You can gain a life by saving extra
people.  If you save more than the required quota for the level then the extra
people appear at on the lower right corner of the screen.  Once that number is
three then you get an extra life and the extra people disappear.

Winning
-------

Once you beat nine levels than you win... I'll add more as I get time.

