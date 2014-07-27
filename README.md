BANANAGRAMS
===========
This is a computer version of the Bananagrams boardgame. Multiplayer is a work
in progress, but solitaire works fine.

The goal is to use up all 144 tiles by constructing a crossword grid. You start
with 21 tiles. Once you have used all of your letters, you can "peel" (see the
controls below). If your grid is valid (connected and uses real words), you get
one more letter. If you have a letter that you don't think you can use, you can
"dump" it to get three letters in exchange.

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
* Dump: exchange the tile under the cursor for three new tiles
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
 * SFML 2.0
 * yaml-cpp 0.5.0
 * Boost 1.53.0
 * WordNet 3.1
 * Enable2k North American word list
 * DejaVu 2.33

License
-------
This program is licensed under the MIT License. See LICENSE.txt

TODO
----
 * sounds
 * tutorial
 * way to cycle through letter positions on the board
