#!/usr/bin/env python3
"""Resolve what `cvs checkout -r TAG` would produce from raw RCS ,v files.

Usage: cvs-resolve-tag.py MODULE_DIR [TAG]

Prints one tab-separated line per file that exists at TAG:
  <revision> <date> <working path> <,v source path>
(paths relative to MODULE_DIR; the working path drops ,v and Attic/).
The explicit source path matters on case-insensitive filesystems, where
e.g. Attic/globals.h,v and Globals.h,v can otherwise be confused.
With no TAG, resolves the trunk head (like a plain `cvs checkout`).

Handles CVS conventions that plain `co` does not:
  * branch tags (magic revision x.y.0.z -> newest revision on branch x.y.z,
    or the branch point x.y if nothing was committed on the branch yet)
  * files in Attic/ (deleted on trunk but possibly alive on a branch)
  * revisions with state "dead" (file removed at that point)
  * vendor-branch default (`branch: 1.1.1`) for untouched imports
"""
import os
import re
import subprocess
import sys


def rlog(path):
    return subprocess.run(["rlog", path], capture_output=True, text=True,
                          check=True).stdout


def parse(text):
    head = re.search(r"^head: (\S*)", text, re.M).group(1)
    m = re.search(r"^branch:\s*(\S*)", text, re.M)
    default_branch = m.group(1) if m else ""
    syms = {}
    block = re.search(r"^symbolic names:\n((?:\t.*\n)*)", text, re.M)
    if block:
        for line in block.group(1).splitlines():
            name, rev = line.strip().split(": ")
            syms[name] = rev
    revs = {}
    for m in re.finditer(
            r"^revision (\S+)(?:\s+locked by: \S+;)?\ndate: (\S+ \S+);.*?state: (\w+);",
            text, re.M):
        revs[m.group(1)] = (m.group(2), m.group(3))
    return head, default_branch, syms, revs


def newest_on_branch(branch, revs):
    """Highest revision directly on `branch` (e.g. 1.20.2 -> 1.20.2.17)."""
    depth = branch.count(".") + 2
    on = [r for r in revs if r.startswith(branch + ".") and r.count(".") == depth - 1]
    if not on:
        return None
    return max(on, key=lambda r: int(r.rsplit(".", 1)[1]))


def resolve(head, default_branch, syms, revs, tag):
    if tag is None:
        if default_branch:          # untouched vendor import
            return newest_on_branch(default_branch, revs) or head
        return head
    rev = syms.get(tag)
    if rev is None:
        return None
    parts = rev.split(".")
    if len(parts) >= 4 and parts[-2] == "0":    # magic branch number
        branch = ".".join(parts[:-2] + [parts[-1]])
        return newest_on_branch(branch, revs) or ".".join(parts[:-2])
    return rev


def main():
    root = sys.argv[1]
    tag = sys.argv[2] if len(sys.argv) > 2 else None
    for dirpath, _, files in os.walk(root):
        for f in sorted(files):
            if not f.endswith(",v"):
                continue
            path = os.path.join(dirpath, f)
            head, dbranch, syms, revs = parse(rlog(path))
            rev = resolve(head, dbranch, syms, revs, tag)
            if rev is None or rev not in revs:
                continue
            date, state = revs[rev]
            in_attic = os.path.basename(dirpath) == "Attic"
            if state == "dead" or (tag is None and in_attic):
                continue
            rel = os.path.relpath(path, root)[:-2].replace("Attic" + os.sep, "")
            print(rev, date, rel, os.path.relpath(path, root), sep="\t")


if __name__ == "__main__":
    main()
