Zombies!
========

This is a fun one player game where you try to rescue the people from the blood
thirsty Zombies.

Installation 
-----------

There's the required libs:
 
 * libpng
 * libfreetype2
 * libsqlite3
 * libvorbis
 * libogg
 * libvorbisfile
 * sqlite3
 * OpenGL
 * GLUT
 * SDL
 * OpenAL

On a mac OpenGL, OpenAL and GLUT are preinstalled.  You only
have to get libpng, SDL, and the sound libraries.  Also, if you
are not on a mac you will have to change the make file suitable
to your system (sorry no ./configure!)

Also, there is an xcode project that you can compile.  You'll
have to get SDL though.  I have the dynamic libraries included
so you don't have to compile them.  Also their install name is
corrected so the game finds them in the app.  See this:
http://www.cocoadev.com/index.pl?ApplicationLinking

Game play
--------

Upon start up, type in you name or use the arrow keys to select
it.  Or you can push the left and right arrow keys to see the
high scores.
 
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

Once you beat 12 levels than you win... I'll add more as I get time.

Credits
-------

*Progamming*
 - Bill Covert

*Music and SFX*
 - Matthew Parker

*Graphics*
 - Bill Covert (Yeah I know they suck, I'll get someone who can
   draw to make better zombies and people and such)

*Fonts*

 - MedievalSharp
   * By wmk69 on openfontlibrary.org
   * License: OFL (SIL Open Font License)

- FreeUniversal
  * By: narrowhouse on openfontlibrary.org
  * License: OFL (SIL Open Font License)
