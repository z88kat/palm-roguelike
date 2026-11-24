<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">

<html>
<head>
<meta http-equiv="Content-type" content="text/html;charset=UTF-8">
<title>Source for iLarn</title>

<style type="text/css" media="screen">@import "../3col.css";</style>
<link rel="SHORTCUT ICON" href="http://roguelike-palm.sourceforge.net/favicon.ico">
</head>
<body>

<div class="content">
  <h1>iLarn source code</h1>
</div>

<div class="content">

      <h2>Where's the source?</h2>

      <p> On GitHub, in the <a href="https://github.com/z88kat/palm-roguelike/tree/main/iLarn">iLarn
      folder</a> of the <a href="https://github.com/z88kat/palm-roguelike">palm-roguelike</a> repository.  The
      original CVS repository is there too, with its full history
      (<a href="https://github.com/z88kat/palm-roguelike/tree/main/cvs-repo/iLarn">cvs-repo/iLarn</a>).</p>

      <h2>What do I need to compile it?</h2>

      <p> Today the games build on macOS with the
      <a href="https://github.com/savaughn/palmdev-macos">palmdev-macos</a>
      toolchain (prc-tools with m68k-palmos-gcc 2.95) and the Palm OS
      SDKs.  The <a href="https://github.com/z88kat/palm-roguelike/blob/main/iLarn/README.md#building">README</a>
      explains how, and how to test the game in the <a href="https://cloudpilot-emu.github.io/">CloudpilotEmu</a> emulator.</p>

      <h2>The original build notes (2002)</h2>
      <p><i>(Kept for history: the prc-tools and emulator links below no
      longer lead to the tools.)</i></p>
      <p> I've used prc-tools 2.0.  I'm running Linux.</p>

      <p> You can get <a href="http://www.palmos.com/dev/tools/gcc/">
      prc-tools</a> from <a href="http://www.palmsource.com/developers/">
      palmsource.com</a>.  Follow their
      directions for installation.  You will need
      <a href="http://www.ardiri.com/index.php?redir=palm&amp;cat=pilrc">
      PilRC</a> as well (version 2.7b or greater).</p>

      <p> <b>SDK</b> - The current version of iLarn uses SDK 3.5 or
      higher.  (Or, in the unlikely event that you want to compile for
      PalmOS 2.0, SDK 2; to compile for OS 2.0 you will also need to
      change which definition of CFLAGS is commented-out in the
      Makefile.)  Prior to iLarn 0.25 I used SDK 3.1 (the upgrade
      involved some header file changes.)</p>

      <p> <b>multigen</b> - Since iLarn is large, it has "multiple
      code resources" and uses multigen.  You might want to read about
      that in the prc-tools documentation.</p>

      <p> <b>emulator</b> - The safest place to test software is in a <a
      href="http://www.palmos.com/dev/tools/emulator/">Palm emulator</a>.  You
      will need a ROM image to use it.</p>

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

<!-- this document tentatively valid HTML Tue Aug 8 2006 -->

</body>
</html>
