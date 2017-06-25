This is a list of feature requests, bugs and known inconsistencies.
It is divided into the sections: known bugs, feature requests, 
code cleanup and documentation

## Known bugs

Bugs marked with a (?) after the title *may* already have been fixed. 
Verify if these bugs still are present.

1. **Up-arrow stuck**? The up-arrow is sometimes stuck when restarting a new 
level. 

1. **Broken physics** Fix cheat in ball physics - by moving to close to 
another cell you can get drawn up on it.

1. **Broken physics**? Sometimes the ball can get stuck on an edge. Clue to 
find bug: print out the position of the ball continuously, play until such a 
position is found and restart from that 
position while debugging.

1. **Ground physics** Make the friction etc. of ice and normal ground 
configurable.

1. **Pipe physics** Fix the physics when entering pipes, rather than bouncing 
on entrance.

1. **Heightmodifier broken** Doesn't seem to update the center of the squares.

## Feature requests


1. **Multiplayer** Multiplayer for multiple computers and/or on same computer.
Implement multiplayer modes in the game. Possible variations: Race, 
Cooperative, Arena

1. **Choose level** In the setup screen: Allow left/right arrow to switch 
between chosen level easier.

1. **Initial menu - speed** Increase the speed of the initial menu.

1. **First use::calibration of mouse** Go to a mouse calibration screen (like 
for the joystick) the first time the game is started. Later: screen 
accessible through the menus.

    Ask user to place mouse is at top left corner of mouse pad and hit button.
Get user to move mouse to bottom-right corner on a diagonal and then
press space.Now this only works if mouse is rolled to each corner.
If mouse is lifted, then it won't work. You then know the distance in
co-ordinates and can now adjust the amount of sensitivity needed. If the
distance is big, then reduce sensitivity. If the distance is short, increase
sensitivity.

1. **Disappearing ground** Ground that disappears when moving on it.

1. **Context sensitive help in the editor**

1. **Snapshots**  Using automatic snapshots together with the highscores file.

1. **Mr. Black** Determine what features etc. he should be afraid of.

1. **Alternative difficulty settings**  Perhaps a "Championship" option for 
the game might be an option? I mean complete all the levels from Kidz, 
through to Jumpz one by one. It could have a different High scoring - more 
like Frozen Bubble where your score is the level you got to - and the time it 
took. That way, in such a competition, you can have infinite lives. It could 
also take into account the scores/flags obtained along the way, as to not 
just be about totally about speed.

1. **api :: Generalized callbacks** Create generalized callbacks for many 
objects. Ie, a lambda expression that can be called whenever an object is 
touched, created, the game is reset etc. Use it to create flags with a 
callback function.

1. **General wind**

1. **Poison pill, Medicine pill**

1. **Alternative opponents** Add more control flags for Mr. Black and create 
alternative opponents. Pendulum, Hammer, Fireballs, Laser beams

1. **Better control of opponent behaviours** Allow some (level designer) 
control of the opponent behaviours. Ie, the path they can take or the area 
they have to stay within. What they are afraid of etc.

1. **Visual effects** Add clouds, fumes emitted from pipes etc.

1. **Lava** Hot ground and lava effects. Maybe lava is moving.

1. **Visible overview map** Make a visible overview map

1. **Hidden levelsets** locked levelsets, have to complete X other levelsets 
first.

1. **Fonts etc. on settings screens**

1. **Let editor create files in current directory**

1. **Decorations** Display "fortunes"... eg. small tips scrolling by on 
bottom of screen. display names of songs ( "Life by Design", "Genesis (In The 
Beginning)" )

1. **Editor :: visualize flags** Possibility to visualize ground flags in 
editor. Esp. the "kill" flag.

1. **Speed bumps**

1. **Automatic snapshots of interesting game events...**

1. **simple debris** (eg. for sand, frozen) using points instead of 
spheres

1. **Editor :: check file permissions**

1. **Springs** When player moves over a spring he automatically jumps.

1. **Alternative "flags"** Allow diamonds, coins etc. on map which have a 
good effect of taking such as adding extra time.

1. **Multiple layers of ground** Tricky to do under current architecture. 
Fixing this would best be begun by making water into a distinct layer; that 
way physics and representation don't have to be updated at the same time when 
the ground layering is done.

1. **Rotation of map in editor and in game**

1. **Add a way of skipping a too difficult level** May make it cost one life 
and half the points or so to skip a difficult level. 

1. **Now playing** A scrolling text saying which song just was started...

1. **Mr. Black / Baby - control area** Add possibility to control the area 
the other balls are allowed to wander into.

1. **Customized camera position** Give both position, focus and 'up' from a 
script.

1. **Improve graphics for speed improvement** Eg. make lines transparent, 
maybe somewhat like a "comet", leaving trails (glowing particles?)

1. **Sound effects** Use all the new sound effects (warren's sound and the 
new laughters)

1. **Internationalization**   (UNDER WAY)

1. **Levelset information** Make the .set files more elaboration with 
descriptive texts, info from the author etc.  Perhaps even screenshots?

## Code cleanup

1. **Desktop and icon install** Determine where to install desktop and icon
files so that desktop environments and programs can find them

1. **Level fixes** Many levels can be polished some more by fixing background 
colors etc.


## Documentation


1. **Documentation** New script commands set-song-preference, force-next-song 
and clear-song-preferences to allow choosing specific songs for levels.

1. **Keys in menus** Document how the keys are used instead of the mouse in 
the menu system.

1. **Modpills** Add information about the large- and small-modpills. Nitro 
modpill

1. **api :: sign** Added optional Z argument to sign's.

1. **api :: overall** Overall cleanup of the api. Use doxygen? New functions 
added: snow, rain Double check that all functions are documented.

1. **editor** Document all keys/menu entries in the editor. Esp: the "smooth" 
feature

1. **help screen** Extra life, birds, teleporters, cactus  => picture added 
in the help1_*.png. Just have to add description text

1. **Default values** Document the default values for all configurable values.

1. **API FAQ** Create a separate FAQ with examples and ideas of how the api 
can be used.

1. **Tutorial Map** Create a "tutorial" map which contains notes in the scm 
file how things are done and texts on the map when testing it.

