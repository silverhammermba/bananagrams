BANANAGRAMS
===========
This is a computer version of the Bananagrams boardgame.

The goal is to use up all 144 tiles by constructing a crossword grid. You start
with 21 tiles. Once you have used all of your letters, you can "peel" (see the
controls below). If your grid is valid (connected and uses real words), you get
one more letter. If you have a letter that you don't think you can use, you can
"dump" it to get three letters in exchange.

The game supports both singleplayer and multiplayer (using multiple computers to
connect to a dedicated server).

Starting
--------
If you run the game from a shorcut, make sure that "Start in" in the shortcut
properties is set to the folder of the executable.

Controls
--------

The game is controlled using the keyboard and mouse. The keyboard controls can
be changed in-game or by editing config.yaml.

MOUSE CONTROLS
--------------

 * Left Click: move the cursor
 * Left Click + Drag: select tiles
 * Right Click (+ Drag): remove tiles
 * Mouse Wheel: zoom

CONTROLS
--------

* Left, Right, Up, down: move cursor
* Left Fast, Right Fast, etc.: move cursor two tiles at a time
* Zoom In, Zoom Out: zoom the view of your grid
* Zoom In Fast, Zoom Out Fast: pretty self-explanatory
* Quick Place: if you have one letter left, hold this button and left click to
  place a tile
* Peel: when you have used up your tiles, press this to check your grid and get
  a new tile
* Center: move the view back to the center of your grid
* Show: looking for a letter? Type it after this to highlight all occurences of
  it in your grid
* Dump: can't use a tile in your hand? Type its letter after this to exchange it
  for three more tiles
* Cut: if tiles are selected, lets you move them to a new location. Press it
  twice to return the cut tiles to your hand
* Paste: place cut tiles back on the grid
* Flip: when you have cut tiles, switch horizontal words to vertical and vice
  versa
* Scramble Tiles: randomize the order of tiles in your hand (press repeatedly to
  keep scrambling)
* Sort Tiles: order the tiles in your hand alphabetically
* Count Tiles: display the count of tiles with each letter in your hand
* Stack Tiles: stack tiles in your hand by letter

Bugs
----
Sometimes when resizing the window, the graphics will become distorted. Slightly
resizing the window again should fix the problem.

Credits
-------
 * SFML
 * yaml-cpp
 * Boost
 * MinGW-w64
 * WordNet
 * Enable2k North American word list
 * DejaVu Font

License
-------
This program is licensed under the MIT License. See LICENSE.txt

TODO
----
 * more sounds
 * tutorial
 * saved games don't work in windows
 * lone modifier keys (e.g. ctrl for quick place) don't work in windows
 * window can accept mouse clicks without having keyboard focus, can be
   confusing
