# COM S 3270, Spring 2025
## Programming Project 1.05: User Interface with Ncurses

Last week we added some characters, and made them move around and smite one another. You may have added some code to drive your `@`. You can rip that code out, now! We’re going to add a user interface that you can use to drive your `@` manually. If you like, you can leave the auto-drive code in there and add a command to turn it on and off at runtime.

Still working in **C**, link in the `ncurses` library and use it for unbuffered I/O.

---

We’re going to make our stairs functional now, too. When the PC goes up or down a staircase, a new dungeon is generated and populated with the PC and new monsters.  
- An upward staircase is represented with `<`.  
- A downward staircase with `>`.  
The PC uses stairs by entering the appropriate stair command (see below) while standing on the staircase.  
NPCs cannot use stairs, not even the smart ones. Stairs provide an important means of escape for PCs.

It’s up to you whether levels **persist** or **disappear**.  
- In most roguelike games, every staircase leads to a new, random dungeon (e.g., if you go down then back up, you will not return to the place you just left).  
- Persistent levels (down and up returns you to your old position) are harder to implement than non-persistent, but not significantly so.

It’s also up to you whether or not you implement **connected stairs**. That is, if you go down a staircase, are you left standing on an up staircase?

---

### User Input

All commands are to be activated **immediately upon key-press**.  
There is **never** a need to hit enter.  
Any command not explicitly defined is a **no-op**.

#### Required Commands:

| Key(s)      | Action                                                                                 |
|-------------|----------------------------------------------------------------------------------------|
| 7 or y      | Attempt to move PC one cell to the upper left.                                         |
| 8 or k      | Attempt to move PC one cell up.                                                       |
| 9 or u      | Attempt to move PC one cell to the upper right.                                       |
| 6 or l      | Attempt to move PC one cell to the right.                                             |
| 3 or n      | Attempt to move PC one cell to the lower right.                                       |
| 2 or j      | Attempt to move PC one cell down.                                                     |
| 1 or b      | Attempt to move PC one cell to the lower left.                                        |
| 4 or h      | Attempt to move PC one cell to the left.                                              |
| >           | Attempt to go down stairs (works only if standing on down staircase).                 |
| <           | Attempt to go up stairs (works only if standing on up staircase).                     |
| 5 / space / . | Rest for a turn (NPCs still move).                                                   |
| m           | Display a list of monsters in the dungeon, with their symbol and position relative to the PC (e.g., “c, 2 north and 14 west”). |
| up arrow    | When displaying monster list: scroll list up (if not at the top).                     |
| down arrow  | When displaying monster list: scroll list down (if not at the bottom).                |
| escape      | Return to character control from monster list display.                                |
| Q           | Quit the game.                                                                        |

---

> If you gave your PC any special powers, like the ability to tunnel through walls, those should no longer apply.  
Your PC is a normal human, save that he or she may be abnormally foolish.

---

With these changes, we no longer need the delay that we built in last week; the game will now pause automatically for input. And `ncurses` should handle the redrawing, so we’re no longer spewing the entire dungeon to the terminal each turn. Things will look much nicer.

---

Note:  
- The keys `y`, `k`, `u`, `l`, `n`, `j`, `b`, and `h` are **vi** and **vim** movement keys.  
- They’re used in many roguelike games (including the original **Rogue** and most of its direct descendants).

---

### Dungeon Layout

- Our dungeons fill **21 out of 24 lines** in a terminal.  
- Display them on **lines 1–21** (zero-indexed).  
- The **top line** is for message display.  
  - Use it to display any messages you like (e.g., debugging info or messages like "There’s a wall in the way!").  
- The **bottom 2 lines** are reserved for **status information**, which we’ll deal with in a later assignment.

---

### Reserved Key Commands (Present and Future Assignments)

#### Movement Keys
| Key(s)        | Action                |
|---------------|-----------------------|
| KEY HOME      | Move up-left          |
| KEY UP        | Move up               |
| KEY PPAGE     | Move up-right         |
| KEY RIGHT     | Move right            |
| KEY NPAGE     | Move down-right       |
| KEY DOWN      | Move down             |
| KEY END       | Move down-left        |
| KEY LEFT      | Move left             |
| 1 / b         | Move down-left        |
| 2 / j         | Move down             |
| 3 / n         | Move down-right       |
| 4 / h         | Move left             |
| 6 / l         | Move right            |
| 7 / y         | Move up-left          |
| 8 / k         | Move up               |
| 9 / u         | Move up-right         |

#### Game Actions

| Key(s)        | Action                              |
|---------------|-------------------------------------|
| . / 5 / space | Rest                                |
| >             | Go down stairs                      |
| <             | Go up stairs                        |
| b             | Move down-left                     |
| c             | Display character information       |
| d             | Drop item                           |
| e             | Display equipment                   |
| f             | Toggle "fog of war"                 |
| g             | Teleport (goto)                     |
| i             | Display inventory                   |
| m             | Display monster list                |
| n             | Move down-right                     |
| s             | Display the default (terrain) map   |
| t             | Take off item                       |
| u             | Move up-right                       |
| w             | Wear item                           |
| x             | Expunge item                        |
| D             | Display the non-tunneling distance map |
| E             | Inspect equipped item               |
| H             | Display the hardness map            |
| I             | Inspect inventory item              |
| L             | Look at monster                     |
| Q             | Quit the game                       |
| T             | Display the tunneling distance map  |

---

You may add **any other commands** that you like, or map the required commands to additional keys, as long as you implement the **specified mappings**.

---

_All code is to be written in **C**._
