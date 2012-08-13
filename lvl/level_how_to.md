# How to make levels for Zombies!

Levels are now made using svg edit (the alpha build).  This is
an in browser svg-editor.

## Elements

*Height and width:*  The height and width of the svg canvas is
going to be the dimensions of the game.  A small level is 50x50,
a large level is 200x200.  This can be changed in the svg source
or the document properties in the menu.

*Save Count:* This is a text object in the level with a single
number in it.  Don't use any other text objects other than this
one.

*Safe Zone:*  This is defined by a circle (not an ellipse) and
MUST be the fill color e5e5e5 in lower case letters.  It cannot
have velocity.

*Hero:* There is only one hero per level.  If you put in more than
one the last one will be loaded.  He is a circle defined by the
fill color of ff0000 in lowercase letters.

*People:*  These are the people you have to save.  They are
defined by the fill color ffff00 in lowercase letters.

*Zombies:*  These are the enemy to be avoided.  Again a circle
with the fill color of 00bf5f in lowercase letters.

*Walls:*  These are single ungrouped line segments.  You may find
it useful to turn on grid snapping to get them to touch each
other.

## Getting people to move:

If you want some people to start out moving draw a line on them
which indicates the direction they will go.  Then group the line
and circle together.  The parser will know this is a velocity
vector and it will not show up as a wall. 

## Misc.

If the zombie circle radius is bigger than 2 they will follow
the hero and try to bit him if he gets too close.  It makes the
game interesting.

Also, when a person is bit by a zombie, they will start
following that zombie around.  You can make this interesting by
intentionally having a zombie run into someone (or multiple
people) to generate some weird behaviour.

See the levels inside the Zombies.app/Contents/lvl/ for
examples.

Happy level making!
