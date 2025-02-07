# COM S 3270, Spring 2025
## Programming Project 1.02: Dungeon Load/Save
 It’s time to save our dungeons to disk. If we’re going to save them, we also want to load them, and of course, if there’s nothing to load, we’ll still want to generate new ones. This functionality will provide us with the ability to test our implementations of a number of the future assignments against “gold” output to confirm correctness. 
 
In Linux and UNIX, we hide files by beginning their names with a dot. We call them dot files. If you list a directory with ls, you won’t usually see dot files by default (it’s possible your shell is configured to give different behavior, however). Passing the -a switch to ls will force it to list all, which includes hidden files. Try it. 
 
Be very careful of the rookie sysadmin mistake of trying to clean up all dot files with `rm -rf .*`. This probably doesn’t do what you expect. To understand why, consider that `.` is another name (an alias) for the current directory and `..` is the parent of the current directory. To get some idea of just how much damage the command above can do, try issuing the command `ls -aR`. 
 
We’ll store all of our game data for our Roguelike game in a hidden directory one level below our home directories. We’ll call it, creatively enough, `.rlg327`. You can create this directly manually in the shell with the mkdir command, or alternatively, you may use the `mkdir(2)` system call (remember that the `2` here means “section 2 of the manual”, which is where system calls are documented) to do it in program control. Since this is a system call, it’s more something for an operating systems class than for this one, so we won’t require you to use it (but it’s not complicated). Since the game data directory is under your home, you need to know how to find that. Use getenv(3) with the argument "HOME". Concatenate `.rlg327/` on to that. Then our dungeon will be saved there in a file named dungeon. Putting that all together, my home directory is `/home/sheaffer`, so when I call `getenv("HOME")` it returns the string `/home/sheaffer`. After concatenating, I get a dungeon save file in `/home/sheaffer/.rlg327/dungeon`. To be clear, I do not hard code this path! I always call `getenv()`, because I want this to work for anybody. If I hard coded the path to my home directory, it would only work for me! 
 
For now, our default will always be to generate a new dungeon, display it, and exit. We’ll add two switches, `--save` and `--load`. The save switch will cause the game to save the dungeon to disk before terminating. The load switch will load the dungeon from disk, rather than generate a new one, then display it and exit. The game can take both switches at the same time, in which case it reads the dungeon from disk, displays it, rewrites it, and exits. You should be able to load, display, and save the same dungeon over and over. If things change from run to run, you have a bug. This is a very good test of both your save and load routines! It doesn’t not show that they are correct (that is, strictly compliant with the spec), but it does show that they are inverses of one another. 
 
Planning ahead a bit, we’re going to include a position for our PC (player character). The only rule is that the PC must be someplace on the floor (i.e., not embedded in the rock). For maximum simplicity you may use, say, the upper left corner of room zero. You’re welcome to display the PC if you want (not required, yet). If you do, use an `@`. 
 
Our dungeon file format follows. All data is written in network order (big-endian). You’ll need to ensure that you are doing the endianness conversions on both ends (reading and writing). All code in C.

| Offset | Num Bytes | Values |
|---|----|----|
| 0 | 12 | A semantic file-type marker with the value `RLG327-S2025`. |
| 12 | 4 | An unsigned 32-bit integer file version marker with the value `0`. |
| 16 | 4 | An unsigned 32-bit integer size of the file. |
| 20 | 2 | A pair of unsigned 8-bit integers giving the x and y position of the PC, respectively. |
| 22 | 1680 | The row-major dungeon matrix from top to bottom, with one byte containing cell hardness per cell. Hardness ranges from `0` to `255`. |
| 1702 | 2 | An unsigned 16-bit integer `r` giving the number of rooms in the dungeon. |
| 1704 | r × 4 | Positions of all rooms in the dungeon, with 4 unsigned 8-bit integers each (x position, y position, width, and height). |
| 1704 + r × 4 | 2 | An unsigned 16-bit integer giving the number of upward staircases in the dungeon. |
| 1706 + r × 4 | u × 2 | `u` pairs of unsigned 8-bit integers, each giving the x and y position of a unique upward staircase. |
| 1706 + r × 4 + u × 2 | 2 | An unsigned 16-bit integer giving the number of downward staircases in the dungeon. |
| 1708 + r × 4 + u × 2 | d × 2 | `d` pairs of unsigned 8-bit integers, each giving the x and y position of a unique downward staircase. |



Everybody’s program, if correct, should load anybody else’s file! If your dungeon doesn’t have a “hard-ness” concept, add it now. It will be needed for a later stage of development. If you’ve implemented non-rectangular rooms, you have two choices:
1. Change your code to make them rectangular. 
2. Decompose them into contiguous, rectangular “subrooms”. The decomposition could even be to contiguous 1 × 1 rooms, which would be an incredibly inelegant implementation, but which would get the job done.


Even if your dungeon generation routines limit your dungeon to a fixed maximum number of rooms, your load routines should have no limit, otherwise, some of us will write dungeons that others cannot read.