# COM S 3270, Spring 2025
## Programming Project 1.09: PC Equipment and Updated Combat

Now we’ve got a dungeon full of super-powered monsters, but our combat semantics haven’t changed to reflect that, so everything—monsters and PC—still dies in one round. Let’s pick up and use those items and update the combat so it goes multiple rounds.

Characters have been updated with hitpoints and damage (if you put these in `npc`, you need to move them to `character`). Give the PC a default number of hitpoints; you may use whatever value you like. The PC’s default speed will remain at 10. It will also get a default damage dice, something small, like `0+1d4`, since this is bare-handed damage and will be augmented by equipment.

The PC gets equipment slots, one each for:

- WEAPON  
- OFFHAND  
- RANGED  
- ARMOR  
- HELMET  
- CLOAK  
- GLOVES  
- BOOTS  
- AMULET  
- LIGHT  
- and **two for RING**

These slots are labeled `a–l` (lowercase 'a' through 'l').

The PC also gets 10 carry slots, numbered `0–9`. Equipped items do not impact carry. When the PC walks over an item, if it has an open carry slot, it automatically picks the item up; else the item is ignored.

Optionally, instead of automatic pickup, you may implement a **pickup command**. When the PC is standing on an object or pile of objects, the `,` (comma) key will pick up items or—better yet—display a pickup menu where the user may choose which of a stack of items to pick up.

---

### New Commands

| Command | Meaning |
|---------|---------|
| `w`     | Wear an item. Prompts the user for a carry slot. If an item of that type is already equipped, items are swapped. |
| `t`     | Take off an item. Prompts for equipment slot. Item goes to an open carry slot. |
| `d`     | Drop an item. Prompts user for carry slot. Item goes to floor. |
| `x`     | Expunge an item from the game. Prompts the user for a carry slot. Item is permanently removed from the game. |
| `i`     | List PC inventory. |
| `e`     | List PC equipment. |
| `I`     | Inspect an item. Prompts user for a carry slot. Item’s description is displayed. |
| `L`     | Look at a monster. Enters targeting mode similar to controlled teleport in 1.06. Select a visible monster with `t` or abort with escape. Display monster description. Escape returns to normal input. |

In all cases, **failures should be handled gracefully**. You may decide what that means, but it does not mean, e.g., crashing the game, leaking memory, adding an extra carry slot, or expunging an item that the user didn’t want expunged. A prompt should include a list of all appropriate slots including their slot numbers and the name of the item in that slot. The user may abort the command by entering the escape character.

---

### Equipment Effects

- **Speed bonuses** from equipment are applied *additively* to the PC’s base speed.
- **Damage bonuses** are applied *additively* to the PC’s damage.
  - All damage dice for all equipped items are rolled and added together.

---

### Combat Updates

- NPCs do **not** attack other NPCs.  
  - When an NPC attempts to move to a cell containing another NPC, the current occupant is displaced to any open cell neighboring it.  
  - If all neighboring cells are occupied, displacement degenerates to a **position swap**.

- Combat is initiated by attempts to move into the cell of a defensive character.
  - The attack consumes the attacker’s turn; no move occurs.
  - **All attacks connect.**<sup><a href="#note1">1</a></sup>

- When an attack connects:
  - Damage is calculated by rolling all applicable dice.
    - NPCs have only one set.
    - PC rolls:
      - Base damage (bare-handed), *only* if nothing is equipped in the weapon slot.
      - Plus **all equipped item dice**.

- HP is reduced by the damage. If HP falls below zero<sup><a href="#note2">2</a></sup>, the character dies and is removed from the game.
- If the **PC dies**, the game ends with a **loss**.
- The game ends with a **win** if the PC kills a monster with the **BOSS flag** (e.g., SpongeBob SquarePants).  
  - The game **no longer** ends when the dungeon level is clear of NPCs.

---

### Notes

1. <a id="note1"></a>You may do something more advanced than auto-hit if you like.
2. <a id="note2"></a>If you have HP as an unsigned type, it will never fall below zero. You will need to use a signed type.
