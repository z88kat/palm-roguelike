# kMoria Player's Manual

kMoria is Moria (Umoria 5.5.2) for Palm OS, ported by Bridget Spitznagel.
This guide covers version 0.16, an update of the original 0.15 built from
this repository.

Your goal is to go down through the Mines of Moria and kill the Balrog, who
lives on the deepest levels. You start in town, buy supplies, and then take
the stairs down. Death is permanent.

This guide covers the Palm controls. For the rules of the game (stats,
races, classes, combat, armour class, objects and the town), read the
original Moria manual, which is included at
[`src/MoriaManual/moria.txt`](src/MoriaManual/moria.txt). There is also a
Palm DOC copy, `src/MoriaManual/moria-manual.pdb`, that you can read in a
document reader on the device.

> **Note:** The command tables in `moria.txt` are for the Unix game. kMoria
> uses its own set of keys, listed below.

---

## Installing

Install **both** files to the device or emulator:

| File | Purpose |
| --- | --- |
| `kMoria.prc` | The game |
| `kMoriaDB.pdb` | Game data: monsters, items, stores and so on. Without it the game won't start. |
| `kMoriaColorDB.pdb` | *Optional.* Colour scheme. With it installed, turn on **Set ▸ Preferences ▸ Display ▸ Color**. |

The game needs Palm OS 3.5 or later for colour. The Makefile can also build
OS 2 and OS 3.1 versions. For building and running it in the emulator, see
[README.md](README.md).

---

## Creating a character

1. **Sex:** Male or Female.
2. **Race:** Human, Half-Elf, Elf, Halfling, Gnome, Dwarf, Half-Orc or
   Half-Troll.
3. **Class:** Warrior, Mage, Priest, Rogue, Ranger or Paladin. Only the
   classes your race can take are shown.
4. The game rolls your stats. Tap **Yes, I'll take it!** to keep them or
   **No, reroll!** to roll again.
5. Check the character summary, then tap **Done** to enter the town.

Mages cast spells using Intelligence. Priests (and Paladins) pray using
Wisdom. Rangers use mage spells. Section 2 of `moria.txt` explains what
each race and class is good at.

---

## The screen

The dungeon map takes up most of the screen. Your character is `@`. The
bottom lines show messages and your status (HP, mana, level and so on).
When the game shows **-more-**, tap the screen or write any stroke to
continue.

The dungeon is bigger than the screen, so you see one **sector** of it at a
time. The view scrolls when you walk off the edge. You can also move it
yourself with **Next sector** (`-`) or open the whole-level **Map** (`M`).

### Map symbols (most common)

| Symbol | Meaning | Symbol | Meaning |
| --- | --- | --- | --- |
| `@` | You | `.` | Floor (or a hidden trap) |
| `#` | Wall | `%` | Mineral vein |
| `+` | Closed door | `'` | Open door |
| `<` | Stairs up | `>` | Stairs down |
| `^` | Trap | `:` | Rubble |
| `1`–`6` | Town shops | `$` | Gold (can be inside a wall) |
| `!` | Potion | `?` | Scroll |
| `,` | Food | `=` / `"` | Ring / amulet |
| `-` / `_` | Wand / staff | `[ ( ] )` | Armour / shield |
| `\| \ /` | Weapons | `} {` | Bows / ammo |

Letters are monsters. For example, `B` is the Balrog. Section 4 of
`moria.txt` has the full list. To check what something is in the game, use
**What's that?** (`/`) and then tap the symbol.

The town shops are: **1** General Store, **2** Armoury, **3** Weaponsmith,
**4** Temple, **5** Alchemist and **6** Magic shop.

---

## Moving

### Tapping

Tap the screen in the direction you want to go. The game uses the
direction from the **centre of the screen** to your tap, so there are eight
directions.

- **Tap close to the centre** (inside the "hole"): nothing happens and no
  time passes.
- **Tap a medium distance from the centre:** walk one step. If a monster is
  in the way, you attack it.
  If a closed door is in the way, you open it. A locked door gets an attempt
  to pick the lock. A stuck door has to be forced (`f`). Next to a monster
  or a door, a far tap also counts as a single step.
- **Tap far from the centre** (outside the "donut"): **run** in that
  direction until something interesting happens. Run is on by default, and
  the game walks instead of running when it isn't safe to run.

To change the size of the hole and donut, use **Set ▸ Preferences ▸
Moving**. With **Relative move** on, directions are measured from your `@`
instead of the screen centre. With **Show circles** on (and Relative move
off), the rings are drawn on the screen.

### Keys (Graffiti or keyboard)

kMoria uses the rogue-like direction keys:

```
 y  k  u
  \ | /
h - . - l
  / | \
 b  j  n
```

- A lowercase letter walks one step. An **uppercase** letter (`H J K L Y U
  B N`) runs.
- `.` stays in place for one turn.
- On a keyboard, the arrow keys also work. Shift + arrow runs.
- On a device with a **5-way navigator** (Tungsten C, T, E and others),
  up / down / left / right walk north / south / west / east. Hold it to keep
  walking. The centre button is left to Palm OS. On older devices the Up and
  Down buttons walk north and south.
- To repeat a command, write a **number first**, for example `5` then `s`
  to search 5 times. A repeated command stops early if something happens.

### Commands that need a direction

Some commands (open, close, dig and others) ask for a direction. A compass
appears. **Tap the direction**, or write one of `hjklyubn`. Tapping the
centre cancels, except for **Look**, where the centre means "look all
around".

---

## Command reference

Each command can be given from a menu, by writing its key in Graffiti, or
through a hardware button you assign to it. Unless the table says
otherwise, every command uses up a game turn.

### Show menu (information)

| Key | Command | Notes |
| --- | --- | --- |
| `M` | Map | The whole level, shrunk to fit. See [The map screen](#the-map-screen). |
| `L` | Message Log | Scroll back through old messages. |
| `Y` | Your Stats | Character sheet. |
| `/` | What's that? | Tap or write a symbol to find out what it is. Tapping a monster shows what you've learned about it. |
| `?` | Look at | Pick a direction (or the centre for all around) to look at monsters and objects. On the look screen, **R** shows what you know about the monster. |
| `-` | Next sector | Moves the view to the next part of the map in the direction you pick. |
| | Instructions / About / High Scores | |
| | **QUIT** | **Ends your character for good.** It counts as a death ("Quitting"). To stop playing and come back later, just leave the app instead (see [Saving](#saving)). |

### The map screen

The level is twice as wide as the screen, so the map shows one half at a
time. The `>>` or `<<` sign shows which way the other half is. Each map
character stands for a 4×3 block of the dungeon and shows the most
important thing in that block.

| Tap | Key | Does |
| --- | --- | --- |
| The other half of the map | `h` `l` `<` `>` space | Show the other half |
| **Mode** | `m` | Switch between symbols and a pixel outline of the walls |
| **@** | `@` `?` | Flash your position |
| **Exit** | `x` or Return | Go back to the game |

### Pick menu (your stuff)

| Key | Command | Notes |
| --- | --- | --- |
| `i` | Inventory | See [Inventory](#inventory-and-equipment). |
| `e` | Equipment | What you're wearing and wielding. |
| `m` | Magic spell | Mages and Rangers. Pick a book, then a spell. |
| `p` | Pray | Priests and Paladins. Pick a book, then a prayer. |
| `G` | Gain spells | Learn new spells when your level allows. |
| `t` | Throw | Pick an item, then a direction. Use this to fire missiles too. |
| `a` | Aim wand | Pick a wand, then a direction. |
| `z` | Zap staff | Pick a staff. |
| `{` | Inscribe | Write a short note on an item. No turn used. |
| `g` | Again | Repeat your last command. |

### Here menu (your own square)

| Key | Command | Notes |
| --- | --- | --- |
| `x` | Exchange | Swap your main and spare weapons. |
| `r` | Rest | Choose **10**, **20** or **Max!**. Resting stops if you're disturbed. |
| `.` | Stay in place | Wait one turn, e.g. to let a monster come to you. Like stepping onto your own square, it picks up anything there and can set off a trap you're standing on. |
| `s` | Search | Look for hidden traps and doors next to you. |
| `#` | Search mode | Turns constant searching on or off. It slows you down. `"` works too, because `#` is hard to write in Graffiti. |
| `<` | Ascend | You must be standing on `<`. |
| `>` | Descend | You must be standing on `>`. |

### Near menu (the square next to you; asks for a direction)

| Key | Command | Notes |
| --- | --- | --- |
| `c` | Close | Close a door. |
| `o` | Open | Open a door or chest. You can also just walk into a closed door to open it. |
| `f` | Force | Bash a stuck door, a chest or a monster. |
| `S` | Spike | Jam a door shut with an iron spike. |
| `d` | Dig | Tunnel through rubble or rock. A digging tool helps a lot. |
| space | Move onto | Step onto a square **without picking up** what's there. |
| `^` | Disarm | Disarm a trap or a trapped chest. |

You can set Search, Dig, Open, Disarm and Force to repeat until they work.
Use **Set ▸ Repeaters**.

### Set menu (options)

| Key | Command | Notes |
| --- | --- | --- |
| | Preferences | See [Preferences](#preferences). |
| | Buttons | Assign commands to the hardware buttons. |
| | Repeaters | Choose which commands repeat until they work. |
| | Spell Btns | Assign spells to the hardware buttons. |
| | Beam… | Beam the game, your top score, an item or gold to another Palm. |
| `U` | User… | Switch between saved characters, or make a new one. |
| `F` | Font | Switch between the big and small map font. The small font shows more of the map. |
| `!` | Stats mode | Change which stats the status line shows. |
| `T` | Show time | Shows the clock and battery voltage. |

Keys shown as `M`, `L`, `Y`, `Q`, `U`, `F` and `T` in the menus are menu
shortcuts. Write the command stroke (`/`) and then the letter.

---

## Inventory and equipment

Tap an item to select it. Buttons for what you can do with it appear along
the bottom:

| Item | Main button |
| --- | --- |
| Food | **eat** |
| Flask of oil | **refill** (your lamp) |
| Weapon | **wield** |
| Armour, rings, amulets, light sources | **wear** |
| Scrolls and spell books | **read** |
| Potions | **quaff** |
| Wands / staffs | **aim** / **zap** |

There are also **drop** and **throw** buttons. To drop part of a stack, the
game asks whether to drop **All** or **Just one**. On the equipment screen,
use **take off** or **drop**.

Tap **[E]** to go from the inventory to your equipment, and **[In]** to go
back. **Done** closes the screen. Every action uses a turn.

The **Options** menu has **Compact?**, which fits more items on the screen,
and **Weights?**, which shows each item's weight.

---

## Shops

Walk onto a shop's number to go in.

- The shop's stock is a table of **Item** and **Gold**, one row per item.
  Long names are cut short with "…". Tap a row to see the item's full name
  and weight below the table. Your gold is always shown there too.
- **Buy:** select an item in the shop's list, then tap **Buy**.
- **Sell:** tap **Sell**, then choose an item from your pack. The shop only
  offers **sell** for kinds of item it buys.
- Use **< −** and **+ >** to page through the shop's stock.

**Haggling.** You make an offer and the shopkeeper makes a counter-offer.
Write your offer and tap **Submit**. **[=]** offers exactly what the
shopkeeper is asking. After you've made two offers, a **[+n]** button
appears. It adds the same step again (n is how much your last two offers
differed). Make the shopkeeper angry and they'll throw you out. To skip
haggling and just pay the asking price, turn off **Options ▸ Haggle?**.

---

## Magic and prayer

1. Give the **m** (or **p**) command. A list of your spell books appears.
2. Choose a book and tap **Read Book**.
3. The list shows each spell's level (**Lv**), mana cost (**Mn**) and
   chance of failing (**Fail**). Choose a spell and tap **Recite**, or write
   its letter (`a`–`i`). If the spell needs a direction, the compass
   appears.

Use **G** (Gain spells) to learn new spells as you level up. Choose a spell
and tap **Learn**. The same screen can also show whether you know each
spell and whether you've tried it yet.

Shortcuts: on the book list, the four app buttons choose the 1st to 4th
book. Under **Set ▸ Spell Btns** you can assign a spell from each book to a
hardware button, so pressing that button on the spell list casts it at
once.

---

## Hardware buttons

By default, the app buttons do what they normally do on the Palm: they
start other programs, which leaves kMoria (your game is saved). In the
dungeon view, the 5-way navigator (or the Up and Down buttons) moves you; see
[Moving](#moving).

Use **Set ▸ Buttons** to change this. Tick **Rebind these buttons**, then
give each button (Date, Addr, To Do, Memo, Up, Down, Calc, Find) a command:

> north · south · east · west · map · look · inventory · magic/pray ·
> search · srch mode · rest · dig tunnel · force/bash · open door ·
> close door · spike door · font · stats · exchange · throw · aim wand ·
> zap staff · what's that · next sector · again · **RUN**

**RUN** acts as a shift key. Press it and then a direction to run that way.

A common setup is to put the four directions on the Up, Down, Calc and Find
buttons, which lets you play with one hand.

Some buttons always do the same thing on certain screens:

| Screen | Buttons |
| --- | --- |
| Rest | Date / Addr / To Do / Memo = **0** (cancel) / **10** / **20** / **Max!**. Writing `0`–`3` does the same. |
| Look | Up / Down = next thing · Date / Addr = monster info · To Do / Memo = close |
| Monster info | Any app button closes it |
| Map | The button set to *map* closes the map |

---

## Preferences

**Set ▸ Preferences** has two tabs.

**Display**

- **Big font.** Use the big map font when the game starts. Off by default,
  so the game starts with the small font, which shows more of the map.
- **Sound.**
- **Invert.** White text on a black background.
- **Color.** Only on colour devices.
- **Stats at top of screen.**
- **Highlight/notice mineral seams.** Shows soft rock as `%` and hard rock
  as `#`.
- **Animate spells and missiles.**
- **Prompt for save file** and **Select most recent user.** These control
  which character is loaded when the game starts.

**Moving**

- **Hole & Donut sliders.** How far from the centre a tap must be to walk,
  and how far to run.
- **Relative move.** Measure taps from your `@` instead of the screen
  centre.
- **Show circles.** Draw the hole and donut on the screen.
- **Run.** Far taps run. When this is off, they only walk.
  - **Cut known corners.** Run diagonally through corridors you've already
    explored, which saves turns.
  - **Examine potential corners.** Don't cut corners in corridors you
    haven't explored yet.
  - **Stop when map sector changes.**
- **Run through open doors.** Don't stop at doorways.
- **Prompt to pick up objects.** This option does nothing yet.

---

## Saving

- The game **saves automatically** whenever you leave kMoria: pressing an
  app button, tapping Home or turning off the device. When you start
  kMoria again, you carry on where you were.
- **Show ▸ QUIT ends your character.** It counts as a death. If you've won,
  it's called **retiring** instead.
- If kMoria crashed last time, it asks when it starts. Choose **Recover Me**
  to load your last save, or **Just Quit**.
- **Set ▸ User…** lets you keep several characters, each in its own save
  file. The **Hotsync?** box backs up that save when you HotSync.

## Dying

When you die, kMoria shows your tombstone. Tap **Write memo** to save a
full character record as a new Memo Pad memo (Unfiled, titled with the
date and time you died). It uses the same layout as Unix Moria. Tap **Next
page** to go on to the high scores, and **Play again** to make a new
character.

---

## Tips for new players

- In town, buy **flasks of oil**, **Cure Light Wounds** potions and
  **food**. Mages should buy **Magic Missile**. Throwing a flask of oil
  (`t`) does good fire damage early on.
- Don't fight in the open. Step back into a corridor so monsters can only
  reach you one at a time.
- Rest (`r` ▸ **Max!**) before going down the stairs.
- Turn on search mode (`#`) when you think there's a secret door. Turn it
  off afterwards, because it slows you down.
- Use `/` on any monster you don't recognise before you attack it.
- If an item is cursed you can't take it off. A **Remove Curse** scroll
  fixes this.
