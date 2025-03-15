# MancalaFish

MancalaFish is an AI that can play the Turkish variation of the game Mancala.

## Compiling

If you are on Windows, use Ubuntu WSL. If you want an executable file that can be run on Windows, use MinGW/MSYS32.

### Compiling the interactive player

Use the following command to compile with GCC (Or use cmake):

```bash
gcc src/player.c src/engine.c -std=c99 -O3 -o mancala_player
```

Then just run the `mancala_player.exe` or `./mancala_player` file and play with it.

You just enter the move you want to make whenever it's your turn. You're Home A on the right. Your pits are on top. 0
notates the top left. 5 notates the top right. Just type in which pit you want to move.

### Compiling and using the Engine CLI

Use the following command to compile:

```bash
gcc src/engine_cli.c src/engine.c -std=c99 -O3 -o mancala_engine
```

Run the `mancala_engine.exe` or `./mancala_engine` file.

Now you can type in a position in the following format:

A=a number between 0 and 31 notating a pit
B=a number between 0 and 64 notating a home pit
T=whose turn it is, 0 or 1.
D=depth

Format: `A/A/A/A/A/A/A/A/A/A/A/A/B/B/T/D`

First 6 pits are for player 0, next 6 for player 1, next 2 are player 0's home and player 1's home, then whose turn it
is and finally the depth.

Example given the position:

```
Home A    2   1   2   5   7   3  Home B
   (12)   3   4   5   6   7   8  (13)
```

Formatted position: `3/7/5/2/1/2/3/4/5/6/7/8/12/13/0/16`

Goes from right top to left, then bottom left to bottom right.

Turn=0 means it's Home A's turn and that's the player that can touch the top pits.

Depth=16 is fairly good and takes about a second to compute. Will think 16 moves in advance. (Depth is about O(2^n))

After typing this in, you will get a response similar to this: `m=0,e=-1,d=16`

m notates the best move which can range from 0 to 11. 0 is top right, goes to left then down, then to the bottom right
ending with an 11.

e notates the evaluation of the position. Positive means player 0 wins, negative means player 1 wins, and 0 means a
draw.

d notates the depth of the position.

### Compiling with Emscripten to run on web

First install emscripten.

Linux:

```bash
sudo apt update
sudo apt install emscripten
```

Windows (with Chocolatey):

```bash
choco install emscripten
```

Use the following command to compile:

```bash
emcc src/engine.c -o mancala.js -s EXPORTED_FUNCTIONS='["_get_best_move", "_evaluate"]' -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'
```

This gives you a WASM file and a JavaScript file that you can use in a website's code.