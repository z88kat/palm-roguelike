# iLarn Player's Manual

iLarn is Ularn ("Ultra-Larn") for Palm OS, ported by Bridget Spitznagel.
Ularn is Phil Cordier's extension of Noah Morgan's 1986 game Larn. This
guide covers version 0.40, an update of the last release, 0.30 (August 2001),
built from this repository.

Your daughter has caught **dianthroritis**, and the only cure is the rare
**potion of cure dianthroritis**. It lies at the very bottom of the volcano
below the town of Larn. You must find it and bring it **Home** before her
time runs out. Death is permanent.

On the device, this guide is the **iLarn Manual** e-book
(`iLarn-manual.pdb`). The game's own **iLarn ▸ Instructions** is a one-screen
summary of it.

---

## Installing

`make` puts everything in `iLarn/build/`. Install the first two files to the
device or emulator:

| File | Purpose |
| --- | --- |
| `iLarn.prc` | The game. On first run it creates `iLarnSaveDB`, which holds your saved game, your clone and the high scores. |
| `iLarnDB.pdb` | **Required.** The game data (the fixed maps). Without it iLarn shows "iLarn needs database" and stops. |
| `iLarn-manual.pdb` | *Optional.* This manual, as an e-book for a Palm DOC reader (AportisDoc, Palm Reader, CSpotRun, ...). |

Colour needs Palm OS 3.5 or later. For building and running iLarn in the
emulator, see [README.md](README.md).

---

## Making a character

The first time you play, and after every death, you design a character.

1. Tap a **class**. Each starts with different statistics, items and spells:

   | Class | Str | Int | Wis | Con | Dex | Cha | HP | Starts with | Spells |
   | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
   | Ogre | 18 | 4 | 6 | 16 | 6 | 4 | 16 | 2 potions | magic missile |
   | Wizard | 8 | 16 | 16 | 6 | 6 | 8 | 8 | potion of treasure finding, 2 scrolls | magic missile, charm monster |
   | Klingon | 14 | 12 | 4 | 12 | 8 | 3 | 14 | studded leather armour (worn), potion | sonic spear |
   | Elf | 8 | 14 | 12 | 8 | 8 | 14 | 8 | leather armour (worn), scroll | protection |
   | Rogue | 8 | 12 | 8 | 10 | 14 | 6 | 12 | dagger, leather armour, scroll of stealth | magic missile |
   | Geek | 12 | 12 | 12 | 12 | 12 | 12 | 10 | leather armour, dagger | protection, magic missile |
   | Dwarf | 16 | 6 | 8 | 16 | 4 | 4 | 12 | spear (wielded) | protection |
   | Rambo | 3 | 3 | 3 | 3 | 3 | 3 | 1 | lance of death (wielded) | none |

2. Tap **male** or **female**.
3. Tap **So, what are ya?** to start.

With the 5-way: up / down step through the classes (down each column of
the grid), left / right pick male / female, and centre starts.

If you have saved a clone (see [Clones](#clones)), there are two buttons
instead: **Generate** makes the new character, and **No, Use My Clone**
brings back the clone.

After you have won a game, a **Difficulty** button and list appear: you may
play at any difficulty up to one more than the highest you have won at.
Higher difficulties make monsters tougher and the world less forgiving.

---

## The screen

- **The map** fills most of the screen. It scrolls to follow you;
  **Options ▸ Re-Center** centres it on you.
- **Active spells** appear down the right edge of the map in the dungeon,
  as two-letter codes: `S` stealth, `U` undead protection, `Sp` spirit
  protection, `Ch` charm, `T` time stop, `Ho` hold monster, `G` giant
  strength, `F` fire resistance, `D` dexterity, `St` strength, `Sc` scare
  monster, `Ha` haste self, `C` cancellation, `I` invisibility, `P'`
  altered protection, `P` protection, `W` walk through walls.
- **The status lines** at the bottom:
  - `Hp` hit points / maximum, `Ac` armour class, `Wc` weapon class,
    `Floor` where you are (`H` is the town, `1`–`15` the dungeon,
    `V1`–`V5` the volcano; `?` after you have been teleported).
  - `Spells` spells left / maximum, then your strength, intelligence,
    wisdom, constitution, dexterity and charisma.
  - `Level` your experience level, `Exp` experience points, `$` gold.
- **Messages** appear over the status lines. When there are more than fit,
  **--more--** appears: tap the screen (or press any key or the 5-way) to
  see the rest. **Options ▸ Message Log** shows the last ten.

**Options ▸ Change Font** switches the dungeon between the normal font and a
tiny one that shows more of the level. (The town always uses the normal
font.) **Options ▸ Map** shows the whole level at once; tap or press any key
to close it.

### Map symbols

Monsters are plain letters. Things are drawn as a block with the letter cut
out of it (a coloured block when colour is on), so a monster `A` and an
altar `A` look different.

| Symbol | Thing | Symbol | Thing |
| --- | --- | --- | --- |
| `#` | wall | `,` | not explored yet |
| `!` | potion | `?` | scroll |
| `B` | book (learn a spell) | `C` | chest |
| `)` `(` | weapons | `[` `]` | armour, shield |
| `\|` | ring | `{` | belt |
| `/` | wand of wonder, staff of power | `.` | amulet, scarab, lamp, talisman, ... |
| `*` | gold | `<` | gem |
| `~` | the Eye of Larn | `:` | drugs |
| `c` | fortune cookie | `o` | orb |
| `s` | sphere of annihilation | `u` | golden urn |
| `%` | stairs | `^` | trap, teleporter, elevator |
| `D` `O` | closed and open door | `P` | pit |
| `A` | altar | `T` `t` | throne |
| `F` `f` | fountain, dried-up fountain | `M` | mirror |
| `&` | statue | `V` | volcano shaft |
| `E` | dungeon entrance | `H` | your home |
| `=` | DND store | `+` | College of Larn |
| `$` | bank | `S` | trading post |
| `L` | Larn Revenue Service | `@` | Dealer McDope's Pad |

Some traps are invisible until you find them.

### Monsters

| Letter | Monster | Letter | Monster |
| --- | --- | --- | --- |
| `l` | lemming, lama nobe | `G` | gnome |
| `H` | hobgoblin | `J` | jackal |
| `K` | kobold | `O` | orc |
| `S` | snake | `c` | giant centipede |
| `j` | jaculi | `t` | troglodyte |
| `A` | giant ant | `E` | floating eye |
| `L` | leprechaun | `N` | nymph |
| `Q` | quasit | `R` | rust monster |
| `Z` | zombie | `a` | assassin bug |
| `b` | bitbug | `h` | hell hound |
| `i` | ice lizard | `C` | centaur |
| `T` | troll | `Y` | yeti |
| `d` | white dragon | `e` | elf |
| `g` | gelatinous cube | `m` | metamorph |
| `v` | vortex | `z` | ziller |
| `F` | violet fungi | `W` | wraith |
| `f` | forvalaka | `o` | osequip |
| `r` | rothe | `X` | xorn |
| `V` | vampire | `I` | invisible stalker (only with see invisible) |
| `p` | poltergeist | `q` | disenchantress |
| `s` | shambling mound | `y` | yellow mold |
| `U` | umber hulk | `k` | gnome king |
| `M` | mimic | `w` | water lord |
| `D` | bronze, green, silver, platinum and red dragons | `P` | purple worm |
| `x` | xvart | `n` | spirit naga |
| `u` | green urchin | `1`–`7` | demon lords |
| `9` | demon prince | `0` | the Prince of Darkness |

Demons are invisible unless you carry the Eye of Larn. A mimic shows up as
a random letter.

### Colour

On a colour screen with Palm OS 3.5 or later, colour is on by default for a
new install: iLarn has a built-in **neon** scheme in which every kind of
thing and monster has its own vivid colour (red dragons are red, gold is
gold, stairs are green...). It works on both the white and the **Invert**
(black) background. Turn it off and on with **iLarn ▸ Preferences ▸ Color!**.

You can change any colour with a memo in the Memo Pad whose first line is
`iLarn-RGB`. Each further line is a symbol and a six-digit hexadecimal
`rrggbb` colour:

```
iLarn-RGB
@ 00ff00
A_ ffcc00
Dr ff0000
```

- A letter followed by `_` is the *thing* of that letter (`A_` is the altar,
  `A` is the giant ant).
- `Db`, `Dg`, `Ds`, `Dp` and `Dr` are the bronze, green, silver, platinum
  and red dragons; a plain `D` colours all the dragons the memo doesn't list.
- Symbols the memo leaves out keep their built-in colour.

iLarn reads the memo when it starts. On Palm OS 5.4 and later (T5, TX,
Treo 650...) the new Memos application keeps memos somewhere else, so the
memo is ignored there; the built-in colours still work.

---

## Moving

### Tapping

Think of the screen as an eight-point compass centred on the middle of the
screen, and tap in the direction you want to go:

- Tap **near the middle** (the "hole") to do nothing; it just redraws the
  status lines.
- Tap **a bit further out** (the "donut") to **walk** one step.
- Tap **near the edge** to **run** until something interesting happens.

**iLarn ▸ Preferences** sets the sizes of the hole and the donut. Untick
**Run.** to always walk one step. Tick **Relative move.** to centre the
compass on your character instead of on the screen.

Walk into a monster to attack it. **Commands ▸ Fight (F)** attacks in a
direction without moving, which is useful against things you can't see.

### Keys (Graffiti or keyboard)

Tick **iLarn ▸ Bindings ▸ Enable hjklyubn for keyboard**, and then:

| Key | Direction | Key | Direction |
| --- | --- | --- | --- |
| `k` | north | `j` | south |
| `h` | west | `l` | east |
| `y` | north-west | `u` | north-east |
| `b` | south-west | `n` | south-east |

Capital letters (`K`, `J`, ...) run. Leave this off if you use Graffiti, so
that misread strokes don't move you.

### 5-way navigator (Tungsten C and other Palm OS 5 devices)

The 5-way works without any setting:

- **Up / down / left / right** walk north / south / west / east. Hold it to
  keep walking. With caps lock on (or shift), they run.
- **Centre** rests for one turn. Resting on top of something offers its
  popup again (see [Things on the floor](#things-on-the-floor)), so
  centre is also how you go back to stairs or a potion you ignored.
- After **--more--**, any of them shows the next messages.
- When a spell or Fight is waiting for a direction, up / down / left /
  right aim it and centre cancels.
- In the **popup of things to do** (Take, Drink, Ignore...), a frame sits
  on the first button; the 5-way moves it from button to button and centre
  presses the framed one.
- In the **inventory** and the **spell** lists, up / down pick a line,
  centre does the main action (the inventory's first button, the spell
  list's Ok) and left cancels.
- In the **stores** (the DND store, the college, the bank) and at
  **Dealer McDope's**, up / down pick a line, centre buys
  (enrols, sells) and left leaves. In the DND store, going past the top or
  bottom of the list turns the page, and right shows the next page; in the
  bank, right opens the Transaction window.
- In the **Bank Transaction** and **LRS** windows, the 5-way moves a frame
  between the buttons and centre presses the framed one (or Cancel / Exit,
  if nothing is framed). Typing an amount needs the keyboard or Graffiti;
  the **[=]** buttons fill in "all of it".
- In the **menus**, the 5-way moves between menus and items, and centre
  picks one.
- In the **message log** and the **high scores**, up / down scroll and
  centre is Done (in the high scores, left / right switch between Show All
  and Winners). Centre or left also closes **About** and the screen you get
  at **home**; any press closes the **map**. The **trading post** uses the
  inventory list, as above.

The plain Up / Down buttons of older Palms (which aren't a 5-way) still do
what they always did: nothing, unless you rebind them (see
[Hardware buttons](#hardware-buttons)).

### Directions for spells and Fight

When a spell (or **Fight**) needs a direction, eight lines divide the screen
into slices. Tap a slice, or use the 5-way or the `hjklyubn` keys. Tap the
middle (or press the 5-way centre) to cancel; a cancelled spell gives its
spell point back.

---

## Things on the floor

When you step onto something, a **popup** offers what you can do with it:

| Thing | Choices |
| --- | --- |
| potion | Drink, Take, Spill |
| scroll | Read, Take, Burn |
| book | Read, Take |
| fortune cookie | Eat, Take |
| altar | Give (donate gold), Pray, Desecrate |
| throne | Pry off gems, Sit |
| fountain | Drink, Wash |
| chest | Open, Take |
| brass lamp | Rub, Take |
| drugs | Snort / Eat / Smoke, Take |
| door | Open / Close |
| stairs, volcano shaft | Climb up / Climb down |
| shop, bank, college, home, entrance | Go in |
| anything else | Take |

**Ignore** (or **Stay**) leaves it where it is. You can also write the
button's first letter, or `1`–`4`. Answering always takes a turn.

Gold is picked up at once. Pits, trap doors, elevators and teleporters act
as soon as you step on them.

---

## Command reference

You can choose commands from the menus, or write the letter in brackets in
Graffiti (or type it).

### Options menu

| Item | What it does |
| --- | --- |
| Map | The whole level, tiny. Tap or press any key to close it. |
| Message Log | The last ten messages. |
| Re-Center | Centre the map on you. |
| Change Font | Normal or tiny dungeon font. |
| High Scores | The scoreboards: everyone, or only winners. |
| Clone Me | Save a clone of your character (see [Clones](#clones)). |
| (Q) QUIT | Kill your character and start again. |

### Commands menu

| Command | What it does |
| --- | --- |
| (i) Inventory | Your pack: use, wield, wear and drop things. |
| (t) Take off armor | Take off your armour. |
| (c) Cast spell | Pick a spell you know, then (for most) a direction. |
| (d) Known items | The potions and scrolls you have identified. |
| (.) Rest | Do nothing for a turn. |
| (^) ID trap | Name the traps next to you. |
| (F) Fight | Attack in a direction. |
| (Z) Teleport | Teleport, once you are level 10 or more. On a difficulty above 0 it can kill you. |
| ($) Drop gold | Drop some of your gold. |
| (w) Pack weight | How much you are carrying. |
| (s) Tax status | What you owe the Larn Revenue Service. |

### iLarn menu

| Item | What it does |
| --- | --- |
| Instructions | A one-screen summary of this manual. |
| About iLarn | Version, credits; **License** and **History**. |
| Preferences | See [Preferences](#preferences). |
| Bindings | See [Hardware buttons](#hardware-buttons). |
| Beam iLarn! | Beam the game (`iLarn.prc` and `iLarnDB`) to another Palm by infrared. |

---

## Inventory

**Commands ▸ Inventory (i)** lists what you carry, `a` to `z`. Tap a line
(or write its letter, or use the 5-way up / down) and the buttons change to
suit it:

- The **first button** does the obvious thing: Read a scroll or book, Drink
  a potion, Wear armour, Wield a weapon, Put on a ring, Eat a cookie...
- **drop** drops it.
- **wield** wields it as a weapon (or unwields).
- **Cancel** closes the list. Nothing you only look at takes a turn.

Armour, shields and rings take effect at once. A two-handed sword can't be
used with a shield.

---

## Spells

You start knowing one or two spells (see the class table). Reading a
**book** teaches you another, and the **College of Larn** in town also
teaches. Each cast uses one spell point; points come back slowly.

**Commands ▸ Cast spell (c)** lists the spells you know. Pick one and tap
**Ok** (or write its letter). Spells that need a direction then show the
aiming slices.

The spells are: protection, magic missile, dexterity, sleep, charm monster,
sonic spear, web, strength, enlightenment, healing, cure blindness, create
monster, phantasmal forces, invisibility, fireball, cold, polymorph,
cancellation, haste self, cloud kill, vaporize rock, dehydration, lightning,
drain life, invulnerability, flood, finger of death, scare monster, hold
monster, time stop, teleport away, magic fire, make a wall, sphere of
annihilation, genocide, summon demon, walk through walls, alter reality and
permanence.

---

## The town and the dungeon

You start in the **town** (floor `H`). Its buildings:

- **DND store** (`=`): buy weapons, armour, potions, scrolls and more.
  Browse the pages with the arrows.
- **College of Larn** (`+`): courses that raise your statistics. Each takes
  time.
- **Bank** (`$`): deposit and withdraw gold (it earns interest), and sell
  gems and the Eye of Larn. There is a branch office on dungeon level 8.
- **Trading post** (`S`): sell things from your pack.
- **Larn Revenue Service** (`L`): pay your taxes. If you owe taxes, the
  stores won't serve you.
- **Home** (`H`): see how your daughter is, and how long she has left.
  Bring the potion here to win.
- **Dungeon entrance** (`E`) and **volcano shaft** (`V`).

The **dungeon** has fifteen levels below the entrance; the **Eye of Larn**
is on the bottom one, guarded by a demon prince. The **volcano** (`V1`–`V5`)
is far more dangerous. The potion of cure dianthroritis is on `V5`, guarded
by the Prince of Darkness himself.

### Time

Your daughter has about **400 mobuls** (a mobul is 100 turns). Every turn
counts, and courses at the College take a while too. Go Home to see how
long is left.

---

## Preferences

**iLarn ▸ Preferences**:

| Setting | What it does |
| --- | --- |
| Player name | The name on the scoreboard. A new character only takes your HotSync name if this is empty. |
| Sound. | Sound effects. |
| Big font. | Start in the normal (big) dungeon font rather than the tiny one. |
| Centered. | Keep the map centred on you. |
| Relative move. | Taps are relative to your character, not the screen. |
| Invert | White on black. |
| Color! | Colour (Palm OS 3.5 and later; see [Colour](#colour)). |
| Run. | Taps near the edge run; untick to always walk. |
| Discretion | Running stops as soon as you reach the edge of unexplored ground ("discretion is the better part of valour"). |
| No-Op (hole), Walk (donut) | The sizes of the tap zones (see [Tapping](#tapping)). **Draw** shows them and **Clear** hides them again. |

The current difficulty is shown at the top right.

---

## Hardware buttons

**iLarn ▸ Bindings** can give the Date Book, Address, To Do, Memo Pad,
Calculator, Find and Up / Down buttons a game action: walk south, east,
north or west, cast a spell, rest, map, change font, known items, teleport
or inventory. Tick **Rebind these buttons** to use them; `---` leaves a
button doing what it normally does.

This screen also has **Enable hjklyubn for keyboard** (see
[Keys](#keys-graffiti-or-keyboard)).

The 5-way's up and down are the Up / Down buttons as far as Bindings is
concerned: if you rebind Up or Down here, those 5-way buttons do that
instead of walking.

---

## Saving and restoring

iLarn saves when you leave it (switch to another application) and carries
on where you were when you come back. There is one game at a time. Every
level you have visited is kept, so they are the same when you come back.

### Clones

At difficulty 0 only, **Options ▸ Clone Me** saves a copy of your character
as it is now. After a death you can bring it back with **No, Use My Clone**
on the character screen; that uses the clone up. There is one clone at a
time. A clone can't carry the Eye of Larn or the potion of cure
dianthroritis, and all clones are destroyed when any character wins.

---

## Dying and winning

When you die, the scoreboard shows how you did. Your score is mostly your
gold. If you win, you go on the separate **winners** board, your taxes are
assessed, and you may play the next game at a higher difficulty.

---

## Tips for new players

- Shop first: buy armour and a weapon at the DND store before you go down.
- Don't drink from fountains or sit on thrones you can't afford to lose to.
- A potion or scroll you don't know is a gamble; **Known items (d)** lists
  the ones you have identified.
- Keep an eye on the time: go Home now and then to check.
- Pay your taxes, or the stores will turn you away.
- Spheres of annihilation destroy everything they touch, including you.
