# Trackballs

### Introduction.

Trackballs is a simple game similar to the classical game Marble Madness
on the Amiga in the 80's. By steering a marble ball through a labyrinth
filled with vicious hammers, pools of acid and other obstacles the player
collects points. When the ball reaches the destination it continues
at the next, more difficult level - unless the time runs out. 

It should be noted that this game is not intended to be a replica of 
marble madness but rather inspired by it. For instance the game uses 
advanced 3D graphics even though the original game had no real use for 
it. Also we aim at making the game highly configurable by a scripting 
extension (Guile) and provide a simple editor by which new levels easily 
can be created.

You start /Trackballs/ by giving the command `trackballs` which
instructs the game to load the first level. If you wish to cheat or simply
are testing out a level you are currently designing you can give the command
`trackballs -l foo` which jumps to level "foo". For more descriptions on how
to play trackballs and documentation of the level editor see the homepage at:

[https://trackballs.github.io](https://trackballs.github.io)

Have fun playing *Trackballs*!

![Screenshot](https://trackballs.github.io/screenshots/s16.png "Screenshot")

### About Security

Per default trackballs is not installed setuid or setgid and as such no 
serious security issues should arise as long as trackballs is played only with 
the default levels. However, extra levels can be designed by users and 
upload/download from the internet.  Since the scripting capabilities within 
maps are very powerful it also means a security vulnerability if you are 
running custom maps created by others. 

*Never run a track downloaded from internet unless you trust the creator 
or if you have carefully read the corresponding .scm file of the track.*

This also means that trackballs must never be run with the setuid flag set and
preferably not with any setgui flags either unless you trust all your users 
completely (which still is a bad security practice!)

However, running trackballs without the setuid/setgid flags means that it 
might not be possible for users to modify the highscores file (default 
installed at `/usr/local/share/trackballs/highScores`) unless you have setup 
the privileges of all users to allow this. Rather than setting trackballs 
setgid games it is therefore better to use local highscore files on a per 
user basis. 

### Contributions

The simplest way to contribute levels, code, or updated translations is to 
make a pull request or issue at the current hosting location, namely

[https://github.com/trackballs/trackballs](https://github.com/trackballs/trackballs)

If possible, keep code contribution commits to focus on a single change
(i.e, "Fixed string typos"), and but try to avoid trivial updates: for 
instance, when fixing string typos, fix all of them. Code should be
formatted with `clang-format` per the style control file .clang-format.
Avoid introducing new dependencies or code requiring more than C++03.

As levelset and translation improvements are easier to change than code,
they may be passed in via the issue system.

### Other

If you have a question that needs answering, check the [FAQ](FAQ.md) first.

The change log for Trackballs is [NEWS.md](NEWS.md).

To read about translating Trackballs, see [ABOUT-NLS.md](ABOUT-NLS.md).

Find out who helped realize Trackballs at [AUTHORS.md](AUTHORS.md).

Think the game couldn't be better? Read [TODO.md](TODO.md) and think again!
