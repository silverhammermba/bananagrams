BANANGRAMS
==========
This is a computer version of the Bananagrams boardgame.

Controls
--------
There are two control schemes: simple and Vi.

For simple controls, letter keys place tiles, backspace removes tiles, and
arrow keys move the cursor. When holding shift, the cursor moves twice as
fast. When holding CTRL, the up and down arrows zoom in and out.

Vi controls are inspired by the text editor. As with the simple mode, letter
keys place tiles, backspace removes tiles, and arrow keys move the cursor. When
holding shift, many letter keys instead perform actions:
 * HJKL move the cursor
 * X behaves like backspace
!! the following are not yet implemented
 * UIOP move the cursor twice as fast
 * D dumps the selected tile
 * V toggles "visual mode" for selecting groups of tiles
 * Y in visual mode yanks the currently selected tiles
 * P places the yanked tiles on the grid

Both control schemes also allow use of the mouse. Clicking on the grid moves
the cursor to that position. Clicking and dragging selects a group of tiles to
move. Right clicking on a tile removes it from the grid. If you have only one
unplaced tile, you can place it by holding ALT and clicking on the grid.

Credits
-------
 * SFML 2.0
 * Enable2k North American word list
 * WordNet 3.1

TODO
----
 * menu
 * mouse controls
 * cut-and-paste
 * multiplayer
 * animate grid errors
 * sounds
