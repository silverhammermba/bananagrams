BANANAGRAMS
===========
This is a computer version of the Bananagrams boardgame. You can only play
solitaire at the moment (see TODOs).

The goal is to use up all 144 tiles by constructing a crossword grid. You start
with 21 tiles. Once you have used all of your letters, you can "peel" (see the
controls below). If your grid is valid (connected and has valid words), you get
one more letter. If you have a letter that you don't think you can use, you can
"dump" it to get three letters in exchange. Good luck!

Starting
--------
If you run the game from a shorcut, make sure that "Start in" in the shortcut
properties is set to the folder of the executable.

Controls
--------

There are two control schemes: simple (the default) and Vim.
You can switch between them with F5.

Both schemes share some controls:
COMMON
 * typing places tiles
 * Spacebar peels
 * Backspace removes tiles
 * F1-F4 change the tile display between scrambled, sorted, tile counts, and
   tile stacks
COMMON (Mouse)
 * left click moves the cursor
 * right click (and drag) removes tiles
 * mouse wheel zooms
 * Ctrl-left click to place a tile (only if you have one letter left)

SIMPLE
 * arrow keys move cursor
 * hold Shift to move cursor quickly
 * hold Ctrl and press up/down to zoom
 * Ctrl-D dumps the tile under the cursor

VIM
 * all actions (other than placing tiles) require Shift to be held
 * HJKL and the arrow keys move the cursor
 * YUIO move the cursor quickly
 * D dumps the tile under the cursor
 * X removes tiles

Credits
-------
 * SFML 2.0
 * Enable2k North American word list
 * WordNet 3.1

License
-------
This program is licensed under the MIT License. See LICENSE.txt

TODO
----
 * menu
 * definitions
 * cut-and-paste
 * multiplayer
 * animate grid errors
 * sounds
