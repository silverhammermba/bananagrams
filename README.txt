BANANAGRAMS
===========
This is a computer version of the Bananagrams boardgame. You can only play
solitaire at the moment (see TODOs).

The goal is to use up all 144 tiles by constructing a crossword grid. You start
with 21 tiles. Once you have used all of your letters, you can "peel" (see the
controls below). If your grid is valid (connected and uses real words), you get
one more letter. If you have a letter that you don't think you can use, you can
"dump" it to get three letters in exchange. Good luck!

Starting
--------
If you run the game from a shorcut, make sure that "Start in" in the shortcut
properties is set to the folder of the executable.

Controls
--------

The game is controlled using the keyboard and mouse. The keyboard controls can
be modified by editing config.yaml

DEFAULT CONTROLS
 * arrow keys move cursor
 * hold Ctrl and press up/down to zoom
 * hold Shift to move/zoom faster
 * typing places tiles
 * Spacebar peels
 * Backspace removes tiles
 * hold Ctrl to place your last tile by clicking with the mouse
 * Ctrl-c moves the view to the center of your grid
 * Ctrl-d dumps the tile under the cursor
 * Ctrl-x cuts the selected tiles (Ctrl-x again to return tiles to your hand)
 * Ctrl-p places cut tiles on the grid (displaced tiles return to your hand)
 * Ctrl-f flips the cut tiles (words are switched between horizontal/vertical)
 * F1-F4 switch between the four tile displays:
   1. scrambled (press F1 again to rescramble)
   2. sorted (alphabetical order)
   3. counts (number indicates how many you have)
   4. stacks (size of stack indicates how many you have)

MOUSE CONTROLS
 * left click moves the cursor
 * left click and drag selects tiles
 * right click (and drag) removes tiles
 * mouse wheel zooms

Bugs
----
Sometimes when resizing the window, the graphics will become distorted. Slightly
resizing the window again should fix the problem.

Credits
-------
 * SFML 2.0
 * yaml-cpp 0.5.0-1
 * WordNet 3.1
 * Enable2k North American word list
 * DejaVu 2.33-4

License
-------
This program is licensed under the MIT License. See LICENSE.txt

TODO
----
 * menu
 * multiplayer
 * sounds
 * tutorial
