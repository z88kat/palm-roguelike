<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">

<html>
<head>
<meta http-equiv="Content-type" content="text/html;charset=UTF-8">
<title>What's new in iLarn</title>

<style type="text/css" media="screen">@import "../3col.css";</style>

<link rel="SHORTCUT ICON" href="http://roguelike-palm.sourceforge.net/favicon.ico">
</head>
<body>

<div class="content">
  <h1>iLarn status</h1>
</div>


<div class="content">
      <h2>What's new in the current version?</h2>
<p> The current version of iLarn is <b>0.40</b> (November 2026), the
first new release since 2001.  Its source is on
<a href="https://github.com/z88kat/palm-roguelike">GitHub</a>, together
with the <a href="https://github.com/z88kat/palm-roguelike/blob/main/iLarn/MANUAL.md">player's
manual</a> and the
<a href="https://github.com/z88kat/palm-roguelike/blob/main/iLarn/README.md#changes-in-v040">full
list of changes</a>.  <a href="download.php">Download iLarn 0.40
here</a>.</p>
<ul>
  <li> Features
  <ul>
  <li> <b>Colour out of the box.</b>  A built-in "neon" colour scheme
      gives every kind of item, every town building and every monster
      (dragons and demons included) its own colour, readable on both the
      white and the Invert (black) background.  It is on by default on a
      colour screen.  An iLarn-RGB memo (see
      <a href="../color.php">Color</a>) still works, and now only
      overrides the symbols it lists.
  <li> <b>No stylus needed</b> on Palm OS 5 devices with a 5-way navigator
      (Tungsten C and others).  Up, down, left and right walk, holding keeps
      walking, and with caps lock on you run.  The centre button rests a
      turn (and offers again whatever you are standing on) and clears
      --more--.  The 5-way also aims spells and Fight, and works in
      character design, the action popup, inventory, the spell list, the
      stores, college and bank, the menus, the message log, the high
      scores and the map.
  <li> New About, License and History screens, rewritten Instructions,
      and a player's manual that also comes as a Palm DOC e-book
      (iLarn-manual.pdb).
  </ul>
<li> Bug fixes
    <ul>
   <li> Hangs and crashes: a sphere of annihilation near the edge of the
     map looped forever; monster pathfinding and the screen refresh could
     run off the map.
   <li> Saves: a save that failed half-way could leave the save database
     unusable.
   <li> Tapping the screen while aiming Fight left Fight armed forever;
     unknown keys used up a turn.
   <li> Rules: the orb of dragon slaying took away 5 DEX like the
     sunsword; the amulet of invisibility didn't stop see-invisible
     wearing off; the talisman of the sphere protected monsters too; the
     altar's enchantment was applied twice; dropping more than 32,767
     gold went wrong; a new game kept the Eye of Larn's effects.
   <li> Memory leaks, and a handful of smaller things such as the altar
     asking "Drop how much?".
    </ul>
</ul>

      <h2>What was new in 0.25 (alpha, 2001)?</h2>
<p> 0.25 was the last alpha version of the original iLarn. </p>
<ul>
  <li> Features
  <ul>
  <li> COLOR.  <a href="http://roguelike-palm.sourceforge.net/color.php">Here's how to configure it</a>.
  <li> The source now compiles with SDK 3.5.  This caused some exciting
      new bugs at first, which I think I have found and fixed all of,
      but if any remain they'll be the sort that causes Fatal Error,
      so, er, keep an eye out.
</ul>
<li> Bug fixes
    <ul>
         <li> Changing the font does not take time now.
   <li> A four-button action, even cancel, takes time AFTER you answer the popup.
   This fixes: you getting pushed onto a monster, when you fail to open door;
   monster on the other side of door attacks you, when you fail to open door.
   <li> The Chest actions (open, take, ignore) have been reordered.
  (There was also a survey on another reordering question.)
   <li> Structures that are destroyed by fireball don't turn into "," now.
   <li> Stats are updated promptly when you pick up items that affect Ac or Wc.
   <li> Demonlords are now actually invisible without Eye of Larn (like they're
     really supposed to be, but haven't been since, hmmm, August).
     I play-tested it.  It's not so bad.  Demonlords start on level 11 and
     a fixed number of new ones are created <b>each</b> time you enter the level,
     so I'd suggest that you don't backtrack until you get the Eye.
       </ul>
<li> Bugs that I don't quite remember whether they're fixed yet or not:
  <br>"monster 'letter' is left behind after Teleport Away or after stealing"
<li> "Known" bugs that I haven't been able to fix yet:
    <br> Rarely there's a bad volcano level, where, if you climb
up/down, it crashes!   (I will fix this as soon as I can reproduce it
    enough to figure out what the heck the problem is.  This may take
    quite a while.)
<li>  New known bugs
<br>  The "Invert" reverse-video option has occasional white splotches at bottom.
</ul>


<p> Features and bugfixes in beta version 0.22: </p>
<ul>
  <li> Only one score listed per clone.  [I haven't tested this for
      winners yet.]
  <li> Difficulty is displayed (read-only) in Preferences.
  <li> Your name can be edited in Preferences (but is only displayed
      for winners).
  <li> Expensive purchase (e.g. sunsword) overflow bug fixed.
  <li> Pulverization scroll's timing bug fixed.
  <li> One of the "exploitable features" of the game has been made only
      finitely exploitable.
  <li> footnote: I examined behavior of the monster detection potion,
      which had been reported as odd.  Ularn behaves the same way, it
      turns out, so I am demoting this from "possible bug" to "oddity of
      game reality".
</ul>
These features were added in beta 0.21:
<ul>
  <li> "Clone Me": You can save a clone of your character (minus plot tokens).
  <li> Fixed a tax-payment-inheritance bug.
  <li> High score list can be viewed any time.
</ul>

      <h2>What was new in the old stable version?</h2>
<p> The "stable" (== found on PalmGear) version of the original iLarn
was: 0.20. </p>
<p> 0.20 included these bug fixes: </p>
<ul>
<li> Decreased frequency of bottomless trap doors, for difficulty = 0.
<li> Fixed a tax-payment bug (encountered after winning a game).
</ul>
</div>

<div class="content">
      <h2>How does this game differ from iRogue?</h2>
      <p> If you have never played Ularn, here are some ways that it
      is different from Rogue. </p>

      <p> You never get hungry, but instead your quest has a time
      limit (number of moves). </p>

      <p> You can't throw things, but you can learn and cast spells. </p>

      <p> In addition to "strength" you have the other familiar
      "intelligence, wisdom, dexterity, constitution, charisma"
      attributes.  There are several character classes to choose from,
      which will determine the initial values of your attributes. </p>

      <p> There is a town with shops and a bank and a college.  The
      shops have fixed prices.  Your bank account is interest-bearing.
      Taking a college course can raise one of your attributes. </p>

      <p> The dungeon levels are less like a collection of rooms and
      more like a maze. </p>

      <p> Ularn/iLarn 'remembers' the dungeon levels you have been to.
      If you enter the dungeon, explore some of level 1, then return
      to the town, when you re-enter the dungeon you will find level 1
      just as you have left it.. (except that a few more monsters
      might have moved in while you were gone).  This means that when
      you have a game in progress, iLarn will tend to take up more space.
</p>
</div>

<div class="content">
      <h2>Bugs</h2>
      <p> Please <a href="mailto:i0lanthe@users.sourceforge.net">report any bugs
you find</a> and they will be fixed as time permits.</p>


      <p> iLarn comes with absolutely no warranty.  The worst case
for a buggy application is a hard reset which means losing everything
since your last backup.  Caveat emptor.</p>
</div>

<div id="navAlpha">
      <?php include ("navAlpha.txt") ?>
      <?php include ("../navAlpha.txt") ?>
</div>

<div id="navBeta">
      <?php include ("../navBeta.txt") ?>
</div>

<!-- hhmts start -->
<!-- hhmts end -->

</body>
</html>
