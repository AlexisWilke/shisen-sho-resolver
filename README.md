
# Shisen-Sho

Shisen-Sho is a KDE game found here:

https://kde.org/applications/games/org.kde.kshisen

It allows you to play on your own by removing tiles from a board. You win
when all tiles were removed.

# Resolver

The resolver is a tool I wrote to read the tiles and transform them in a
buffer I can use to resolve the board automatically using a simple brute
force algorithm.

_Why did I do that?_

I play the game with the option "Create Solvable Games Only" flag turned on.
Yet, there are times when I'm 99.9999% sure that is not 100% the case. Once
in a while, I end up with a game that cannot be resolved. Yet, I'd like to
verify that I'm correct, hence this software.

# Basics

At the moment, the resolver has two modes. One is to generate the tiles.
If you do not play with the default tiles or somehow they are of a different
size on your screenshots, it is likely that you'll need to regenerate those
to match your samples. This is important since I'm not using AI to transform
screenshots to a board. (Unfortunately, there is no "Save As ..." in that
game, so you cannot save and/or reload a board and play it again later.)

The other mode is used to load any board and resolve it. It is expected
that the board is complete in your screenshot (i.e. if you already removed
a few tiles, this resolver is probably going to crap out).

The search uses a fairly simple algorithm with a tree of steps used to move
forward. It does not try to avoid repeating very similar moves (at times,
move A then B is 100% equivalent to move B then A). It really tries all
cases in a 100% brute force algorithm.

To determine whether a pair of tiles can be removed, it checks all possible
geographical lines. There are 18, two of which could very easily be
simplified. So I wrote 17 specialized functions to verify whether a pair
of tile can be connected _legally_ and if so return true. The result is
then to remove those two tiles and try the next move. Note that it is
frequent that tile A can be removed with tile B, C, or D (i.e. any of the
four tiles can all be removed). As mentioned above, the system will try
all combos. A/B, A/C, A/D, and later it is likely to test B/A, B/C, B/D,
etc.

I use board size 18x8 because I find that one the most challenging. Smaller
or large baords are generally very easy to solve (in my own personal tests).
Of course, once in a while, some 18x8 are very easy to resolve too.

# Build & Run Tool

I put a `build.sh` shell script to run the necessary commands to build the
resolver. It works for me on Ubuntu 22.04 which supports C++ version 23.

I simply run the script to build and run tests like so:

    ./build.sh -t

Try the `--help` command line option to see the options offered by that
script.

## Checking Output

To make sure that it works, I have a `save_board()` function. I use it
to save the input board and intermediate steps. This allows me to debug
the software.

The `save_board()` can save the board in text format, with each time
represented by a number (0 to 36, where 0 represents "no tile here").
I also save the result in a .png which allows for comparing ina much
easier manner than trying to read the .txt file.

The intermediate saves are not automatic. I tweak my code with
preprocessor and/or comments to get those. This is really to make sure
the algorithm does what it's supposed to do. Just keep in mind that
an entire board would generate a minimum (width X height / 2) images.
In my case, that represents 72 files minimum. This is if it resolves
the board the first time. If some moves prevent resolution, then more
files get generated. I do not know the maximum, but it's probably a quite
large number.

## Dependencies

To build, you will need `cmake` (v3.22+ at time of writing).

The code depends on ImageMagick version 6+ to manipulate the image data
(load/save). Everything else you should have in your C++ compiler (STL).

## Regenerate `tiles.cpp`

The tile recognition code is really bad. No real AI involved. It requires
the `tiles.cpp` file to closely match the screenshots you feed it. For
that reason, you may have to regenerate that file.

The `build.sh` script offers the `--generate` command line option. Because
it rewrites the `tiles.cpp` file, you may run in some problems and lose that
file. If that happens, edit the `shisen-sho-resolver.cpp` source file and
edit the #if near the top:

    #if 1            // <-- change this with a 0
    #include    "tiles.cpp"
    #else
    // original sizes come from the first sample
    constexpr int const ORIGINAL_TILE_WIDTH = 95;
    constexpr int const ORIGINAL_TILE_HEIGHT = 131;
    constexpr std::uint8_t const g_tiles[] = { 1 };
    #endif

Once regenerated, don't forget to restore that preprocessor entry.

# Issues

Feel free to post issues or propose pull requests. However, this is just
a pass time tool so I'm not very likely to work on it more than this.

**IMPORTANT:** At time of writing the search is not yet complete. Actually,
it pretty much doesn't work at all (it may remove one pair of tiles...) Bare
with me if you want to use this or go work on your own fork.

