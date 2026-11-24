# iRogue Player's Manual

iRogue is Rogue for Palm OS, ported by Bridget Spitznagel from Tim Stoehr's
Rogue Clone (a copy of Rogue 5.3), with extra monsters and weapons from
urogue. This guide covers version 1.2, an update of the original 1.1.1 built
from this repository.

Your goal is to go down through the Dungeons of Doom, find the **Amulet of
Yendor** (on level 27 or deeper), and bring it back up to the surface. You
start on level 1 with some food, armour and weapons. Death is permanent.

On the device, this guide is the **iRogue Manual** e-book
(`iRogue-manual.pdb`). The game's own **?? ▸ Instructions** is a one-screen
summary of it.

---

## Installing

Install `iRogue.prc` to the device or emulator (`make` puts it in
`iRogue/build/`). The other files are optional:

| File | Purpose |
| --- | --- |
| `iRogue.prc` | The game. On first run it creates `iRogueDB`, which holds your saved game and the high scores. |
| `iRogue-manual.pdb` | *Optional.* This manual, as an e-book for a Palm DOC reader (AportisDoc, Palm Reader, CSpotRun, ...). |
| `iRogueTileDB.prc` | *Optional.* Colour pictures for the map, monsters and items, used when **Tiles** is on (see [Preferences](#preferences)). |

Colour needs Palm OS 3.5 or later. The Makefile can also build OS 2 and
OS 3.1 versions. For building and running it in the emulator, see
[README.md](README.md).

---

## The screen

The screen has three parts:

- **Top two lines: messages**, such as "you hit the bat". When there are
  more messages than fit, **--more--** appears; tap the screen or press any
  key to see the next one. **Options ▸ Message Log** shows the recent
  messages again.
- **Middle: the dungeon.** You see the part of the level around you. You
  are `@`. The room you're in is lit, so you can see what's in it; other
  rooms are dark until you go in.
- **Bottom two lines: your status.**

| Status | Meaning |
| --- | --- |
| `L` | The dungeon level you're on. You start on 1. |
| `HP` | Hit points now (and maximum). At 0 you die. |
| `Str` | Strength now (and maximum). Strength makes you hit harder. |
| `hungry`, `weak`, `faint` | Time to eat. If you go on starving, you faint, and then die. |
| `Arm` | Armour class. Higher is better. |
| `$` | Gold. It only counts for your score. |
| `Exp` | Experience level / experience points. |

### Map symbols

| Symbol | Meaning | Symbol | Meaning |
| --- | --- | --- | --- |
| `@` | You | `.` | Floor |
| `-` `\|` | Room walls | `+` | Door |
| `#` | Passage | `%` | Stairs |
| `^` | Trap | `*` | Gold |
| `!` | Potion | `?` | Scroll |
| `°` (small circle) | Food | `/` | Wand or staff |
| `=` | Ring | `]` | Armour |
| `)` | Weapon | `,` | The Amulet of Yendor |

Letters are monsters: `A`–`Z` are the classic Rogue monsters (the bat,
the dragon, the ice monster...) and `a`–`z` are extra ones (the floating
eye, the giant spider, the vermicious knid...). Some monsters sleep until
you wake them; most attack on sight.

To check what something is, use **Options ▸ whatsit?** and tap the symbol.
Tapping a monster tells you its name.

### Colour

On a colour device iRogue uses a built-in **neon** colour scheme: each kind
of item and each monster has its own bright colour, which reads on both the
normal white background and the **Invert** (black) one. Walls and you (`@`)
are drawn in the plain text colour. Colour is on by default for a new
install; turn it on or off with **Color!** in Preferences.

To change the colours, see [Edit Colors](#edit-colors).

---

## Moving

### Tapping

Imagine that the screen is a compass divided like a pie into eight slices:
N, NE, E, SE, S, SW, W and NW. **Tap inside a slice to move that way**. The
direction is measured from the **centre of the screen**, not from your `@`.
How far from the centre you tap decides what happens:

- **Tap near the centre** (inside the "hole"): **search** the squares next
  to you for hidden doors and traps. This uses a turn.
- **Tap a medium distance away** (inside the "donut"): **walk** one step.
  If a monster is in the way, you attack it.
- **Tap near the edge** (outside the donut): **run** that way until
  something interesting happens, such as reaching a door, a monster, an
  object or the stairs.

To change the size of the hole and donut, use **Options ▸ Preferences**.
With **Rogue-relative** on, directions are measured from your `@` instead
of the screen centre. With **Crosshairs** on, the slices and rings are drawn
around the centre after each tap.

### Keys (Graffiti or keyboard)

iRogue uses the classic direction keys:

```
 y  k  u
  \ | /
h - @ - l
  / | \
 b  j  n
```

- A lowercase letter walks one step. An **uppercase** letter
  (`H J K L Y U B N`) runs.
- `s` searches and `.` rests for a turn. Write a **number first** to repeat
  them, for example `5` then `s` searches 5 times. Resting stops if a
  monster turns up.

### 5-way navigator (Tungsten C and other Palm OS 5 devices)

- **Up / down / left / right** walk north / south / west / east. Hold it to
  keep walking. With caps lock (or shift) on, they run instead.
- **Centre** takes the stairs if you are standing on them (up if you carry
  the Amulet, otherwise down), and otherwise searches once.
- On the compass (below), up / down / left / right pick north / south /
  west / east, and the map closes when you press any direction.

The Tungsten C's keyboard types commands directly, so you can play without
the stylus.

### Stairs

Stairs are `%`. Stand on them and use **Commands ▸ Down** (`>`), the centre
of the 5-way, or the **Down** button of older Palms. You can only go **up**
(`<`, or the **Up** button) while you carry the Amulet of Yendor; going up
from level 1 with it wins the game.

### Commands that need a direction

Some commands (move, fight, throw, zap, identify trap, scroll view) ask for
a direction. A compass appears. Tap a direction, or write one of
`hjklyubn`. **Cancel** (or `.`) cancels. For **Throw** and **Zap** you also
choose what to throw or zap from the list at the bottom.

---

## Command reference

Each command can be given from a menu or, where a key is shown, by writing
that key in Graffiti or typing it. Menu items with a letter after them, like
**Map** (`M`), are menu shortcuts: write the command stroke (`/`) and then
the letter.

### Options menu

| Shortcut | Command | Notes |
| --- | --- | --- |
| `M` | Map | The whole level, shrunk to fit: the rooms and passages you've seen, and the stairs if you've found them. Tap, or press any key, to close it. |
| `L` | Message Log | The recent messages. |
| `W` | whatsit? | Tap a symbol to find out what it is. |
| `V` | Scroll View | Move your view of the level in a direction without moving. Write a number first to move it further. |
| `C` | Re-Center | Redraw the screen centred on you. |
| `R` | Preferences | See [Preferences](#preferences). |
| | Bindings | Give the hardware buttons commands. See [Hardware buttons](#hardware-buttons). |
| `F` | Change Font | Switch between the normal and the small map font. The small font shows more of the level. |
| | (Q) QUIT | **Ends your character for good**, after asking. To stop playing and come back later, just leave the app (see [Saving](#saving-and-restoring)). |

### Commands menu

| Key | Command | Notes |
| --- | --- | --- |
| `i` | Inventory | See [Inventory](#inventory). |
| `.` | Rest | Do nothing for a turn. Takes a count. |
| `m` | Move | Step onto a square **without picking up** what's there. Asks for a direction. |
| `,` | Pick up | Pick up what you're standing on, e.g. after `m`, or when your pack was full. |
| `s` | Search | Look for hidden doors and traps next to you. Takes a count. |
| `f` | Fight | Fight the monster in a direction until it dies or moves away, or you're badly hurt. |
| `F` | til death | Like Fight, but doesn't stop when you're badly hurt. |
| `<` | Up | Climb the stairs. You need the Amulet. |
| `>` | Down | Go down the stairs. |
| `^` | ID trap | Find out what kind of trap is next to you. |
| `t` | Throw | Throw a weapon (darts, arrows, daggers...) in a direction. |
| `z` | Zap | Zap a wand or staff in a direction. |

You pick up anything you walk over (gold too), as long as your pack has
room.

### Wiz menu

| Shortcut | Command | Notes |
| --- | --- | --- |
| `I` | Wizard | Turns wizard (debug) mode on or off. A character that has ever used it doesn't go on the high-score list. |
| | Wiz Create ... | Make any object (wizard mode only). |
| `S` | Autosave | Save now, to the autosave slot. |
| | Save ... | Save a copy of your game to a slot. |
| | Restore ... | Go back to a saved game. See [Saving](#saving-and-restoring). |
| | Top Scores | The high-score list. |

### ?? menu

| Shortcut | Command | Notes |
| --- | --- | --- |
| | Instructions | A short summary of this manual. |
| | About iRogue | Version, licence and history. |
| `G` | Graffiti Help | The Palm's Graffiti reference. |
| | Edit Colors | See [Edit Colors](#edit-colors). |
| `T` | Show time | Shows the clock and the battery voltage. |

---

## Inventory

**Commands ▸ Inventory** (`i`) lists your pack. Each item has a letter.
Tap an item, then an action on the right, then **Ok**:

| Button | Does |
| --- | --- |
| first button | The usual action for that item: **eat** food, **quaff** a potion, **read** a scroll, **zap** a wand, **put on** or **remove** a ring, **wear** or **unwear** armour, **wield** a weapon |
| **drop** | Drop it. Hidden if something is already on your square. |
| **throw** | Throw it; you then pick a direction. |
| **wield** / **unwield** | Wield it as your weapon, or stop wielding it |
| **call** | Give a potion, scroll, wand or ring a name of your own, to remember what you think it does |

Rings go on **fingers**: you have four on each hand (L1–L4, R1–R4). When
you put a ring on, pick a free finger; when you take one off, pick the
finger it's on. Putting on armour takes an extra turn if you have to take
off what you're wearing first. Cursed things can't be taken off until the
curse is lifted.

Unknown potions, scrolls, wands and rings are shown by their look ("a
fizzy green potion") until you identify them, by using them or with a
scroll of identify. When you read an identify scroll, the list appears again
for you to choose what to identify.

Up and Down (or the 5-way) scroll a long list. **Cancel** closes the
inventory.

---

## Hardware buttons

By default, the app buttons do what they normally do on the Palm: they
start other programs, which leaves iRogue (your game is saved). The **Up**
and **Down** buttons of older Palms take the stairs; on a Palm with a 5-way
navigator it moves you (see [Moving](#moving)).

Use **Options ▸ Bindings** to change this. Tick **Use these settings**, then
give each button (Date Book, Address, To Do List, Memo Pad, Up, Down,
Calculator, Find) a command:

> --- · north · east · south · west · up · down · search x 10 ·
> rest x 10 · map · scroll x 5 · throw · zap · inv

**---** leaves the button doing what it normally does. *Up* and *down* take
the stairs; *scroll x 5* moves the view 5 squares. On the map screen, any
button that has a command closes the map.

A common setup puts the four directions on the four app buttons, which lets
you play with one hand.

---

## Preferences

**Options ▸ Preferences** has these settings:

- **Name.** Your name on the high-score list. It starts as the Palm's user
  name. The list keeps one entry per name.
- **Autosave.** Save your game each time you take the stairs, so you can
  recover from that point if the game is interrupted.
- **Sound.**
- **Sort pack.** Keep your pack sorted by kind of item.
- **Small font.** Start with the small map font.
- **Centered.** Scroll the view as you move, so you stay in the middle.
- **Rogue-relative.** Measure taps from your `@` instead of the screen
  centre. (Set the hole to 10 and leave Scroll off if you use this.)
- **Crosshairs.** Draw the tap slices and rings around the centre.
- **Invert.** White on a black background.
- **Color!** Colour (Palm OS 3.5 or later).
- **Tiles.** Show the map with small colour pictures instead of letters:
  bricks for walls, cobbles for passages, a picture for each kind of
  item and for every monster. Needs `iRogueTileDB.prc` installed and
  Palm OS 3.5 or later. Tiles are bigger than letters, so you see less of
  the level at once (16 squares across); the **Map** shows the rest.
- **Search (hole)** and **Walk (donut).** The size of the rings that decide
  whether a tap searches, walks or runs. **Draw** shows them on the screen
  and **Clear** removes the drawing.
- **Outside donut:**
  - **Run...** A tap outside the donut runs. When this is off, it walks one
    step.
  - **follow turns.** When running down a passage, follow its corners
    instead of stopping at each one.
  - **Scroll.** A tap at the very edge of the screen scrolls the view
    instead of moving you.

Tap **Ok** to keep the changes, or **Cancel**.

### Edit Colors

**?? ▸ Edit Colors** shows every symbol in its current colour. Tap one to
pick a new colour for it. **Ok** saves the colours; **Cancel** forgets the
changes.

iRogue saves your colours as a Memo Pad memo titled `iRogue-RGB`. You can
also write or edit that memo yourself. After the title, each line is a
symbol and a colour as 6 hexadecimal digits (red, green, blue), for
example:

```
iRogue-RGB
@ 00ff00
D ff0000
```

makes you green and dragons red. Use `-` for walls and `:` for food.
Symbols the memo leaves out keep their built-in colour, so deleting the memo
brings back the built-in scheme.

---

## Saving and restoring

- The game **saves automatically** whenever you leave iRogue: pressing an
  app button, tapping Home or turning off the device. When you start iRogue
  again, you carry on where you were.
- **Wiz ▸ Save ...** writes a copy of your game to a slot: *autosave*, one
  of your earlier slots, or **New** (up to 15 in all). **Delete** removes
  a slot.
- **Wiz ▸ Restore ...** goes back to a slot. Restoring is "an uncanny
  procedure": come back **Alive** and something may go wrong (you might
  lose gold, an experience level or some strength), or come back
  **Undead**, with no such penalty but very hungry, and with any score
  you get kept off the high-score list.
- **Options ▸ (Q) QUIT ends your character.**

## Dying

When you die, the high-score list appears. Tap **Play Again** to start a new
character, or **Revivify** to restore a saved game (see above).

---

## Tips for new players

- Eat when you see **hungry**. Don't wait until you're **weak**.
- Search (tap the centre) at dead ends and in empty-looking rooms. Many
  levels hide their stairs behind a secret door.
- Fight in passages and doorways, so monsters can only reach you one at a
  time.
- Don't walk into a **floating eye** (`e`): it paralyses you. Throw things
  at it instead, or leave it alone.
- The **aquator** (`A`) and the **rust monster** (`r`) rust your armour. Take it
  off first if you want to keep it, or fight with something else.
- **Leprechauns** (`L`) steal gold and **nymphs** (`N`) steal items, and
  then vanish.
- Read unknown scrolls when no monsters are around. Some are bad news
  (aggravate monster, teleportation), most are useful.
- Use **whatsit?** on any monster you don't recognise before you attack it.
