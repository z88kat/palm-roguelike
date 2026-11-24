#!/usr/bin/env python3
"""Rebuild the old roguelike-palm.sourceforge.net PHP site as static HTML.

Usage: build-website.py [OUTPUT_DIR]      (default: docs/ at the repo root)

Source of truth is the trunk head of cvs-repo/htdocs (checked out with
`co -ko -p`, via tools/cvs-resolve-tag.py so dead revisions and Attic/ are
handled).  The PHP on those pages is only server-side includes, so each page
is rendered by:

  * expanding <?php include("...") ?> -- the sidebar fragments (navAlpha.txt,
    navAlphaTop.txt, navBeta.txt, per-game navAlpha.txt) were never committed
    to CVS, so they live in tools/website/ (recovered from the still-running
    SourceForge copy of the site, 2026-11-24; the sidebar's "Contact me" and
    "Hosted by" SourceForge sections have since been dropped).  {ROOT} in a
    fragment becomes the relative path back to the site root.  Every non-.txt
    file there (3col.css, kMoria/kMoriaColorDB.pdb, also never in CVS) is
    copied as-is;
  * using tools/website/<page>.php instead of the CVS page when it exists
    (updated copies of each game's download, source and what's-new pages,
    which had gone out of date), and adding tools/website/news.txt (news
    since the revival) to the top of the home page's News box, so cvs-repo/
    itself stays untouched;
  * REPLACEMENTS below: site-wide fixes for SourceForge-era links (bug
    reports now go to GitHub issues);
  * replacing the random-quote include (quotes.php + quotes.dat) with a tiny
    script that picks a line from the same quotes.dat in the browser;
  * renaming *.php -> *.html and rewriting internal links to match, and
    turning absolute roguelike-palm.sourceforge.net / .sf.net URLs into
    relative ones so the site works under any base URL (e.g. GitHub Pages).

Hit counters, the include fragments themselves, the CGI-backed survey and the
obsolete table-layout iLarn/color.php (superseded by /color.php) are skipped.

The output is wiped and rebuilt on every run, so make content changes in the
CVS sources or tools/website/, not in the generated files.
"""
import os
import re
import shutil
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HTDOCS = os.path.join(REPO, "cvs-repo", "htdocs")
FRAGMENTS = os.path.join(REPO, "tools", "website")
RESOLVER = os.path.join(REPO, "tools", "cvs-resolve-tag.py")
MARKER = ".nojekyll"  # also marks an output dir as ours to wipe

SKIP = {
    "counter.php", "read_counter.php", "quotes.php",  # PHP-only helpers
    "iLarn/color.php",                                 # obsolete, unlinked
    "activesurveys.txt", "footer.txt", "footer-row.txt",
    "hostedby.txt", "roguelikegames.txt",              # old-layout fragments
}
# Links that were already dead on the original site (the wiki, a CGI survey,
# pages removed before the CVS history ends); reported but not fatal.
DEAD_ON_ORIGINAL = ("baby.html", "wiki/", "survey/", "../survey/",
                    "iLarn/alpha.html")
DEAD_ON_ORIGINAL_PAGES = {("kMoria/play.html", "town.html")}
GITHUB = "https://github.com/z88kat/palm-roguelike"
# Site-wide (regex, replacement) edits for links that went stale when the
# project left SourceForge; applied to every page after include expansion.
REPLACEMENTS = [
    (re.compile(r'<a href="mailto:i0lanthe@users\.sourceforge\.net">'
                r"(report any bugs\s+you find)</a>"),
     rf'<a href="{GITHUB}/issues">\1</a> on GitHub'),
    (re.compile(r"http://nosleepsoftware\.sourceforge\.net/index\.php"
                r"\?page=download"),
     "https://nosleepsoftware.sourceforge.net/"),
]
SITE_URL = re.compile(r"https?://roguelike-palm\.(?:sourceforge|sf)\.net/")
INCLUDE = re.compile(r'<\?php\s+include\s*\(\s*"([^"]+)"\s*\)\s*;?\s*\?>')
ATTR = re.compile(r'\b(href|src)\s*=\s*"([^"]*)"', re.I)

QUOTE_JS = """<span class="quote">{fallback}</span><script>
(function (el) {{
  fetch("quotes.dat").then(function (r) {{ return r.text(); }}).then(function (t) {{
    var q = t.split("\\n").filter(function (l) {{ return l.trim(); }});
    if (q.length) el.textContent = q[Math.floor(Math.random() * q.length)];
  }}).catch(function () {{}});
}})(document.currentScript.previousElementSibling);
</script>"""


def checkout(path):
    """Trunk-head contents of htdocs/<path,v> as bytes."""
    return subprocess.run(["co", "-q", "-ko", "-p", path + ",v"], cwd=HTDOCS,
                          capture_output=True, check=True).stdout


def head_files():
    out = subprocess.run([sys.executable, RESOLVER, HTDOCS], check=True,
                         capture_output=True, text=True).stdout
    return [line.split("\t")[2] for line in out.splitlines() if line]


def skipped(path):
    return (path in SKIP or path.startswith("survey/")
            or path.endswith(".hits")
            or os.path.basename(path) == "playnavigation.txt")  # inlined


def fix_url(url, root):
    url = SITE_URL.sub(root, url)
    if re.match(r"[a-z][a-z0-9+.-]*:|#", url, re.I):
        return url  # external, mailto:, or same-page anchor
    path, sep, frag = url.partition("#")
    if path.endswith(".php"):
        path = path[:-4] + ".html"
    elif path.endswith("/") or path in ("..", "."):
        path = path.rstrip("/") + "/index.html"
    return path + sep + frag


def add_news(text):
    """Put tools/website/news.txt at the top of the home page's News box."""
    with open(os.path.join(FRAGMENTS, "news.txt"), encoding="utf-8") as f:
        news = f.read().rstrip("\n")
    heading = "<h2>~ News ~</h2>"
    if heading not in text:
        sys.exit(f"index.php: no {heading!r} to add news.txt under")
    return text.replace(heading, heading + "\n" + news, 1)


def render(page, files):
    pdir = os.path.dirname(page)
    root = "../" * page.count("/")
    override = os.path.join(FRAGMENTS, page)
    if os.path.exists(override):  # an updated copy of the CVS page
        with open(override, encoding="utf-8") as f:
            text = f.read()
    else:
        text = checkout(page).decode("utf-8")

    def include(m):
        target = os.path.normpath(os.path.join(pdir, m.group(1)))
        if target == "quotes.php":
            dat = os.path.join(pdir, "quotes.dat")
            # Deterministic no-JS fallback; the browser picks a random line.
            lines = [l for l in checkout(dat).decode("utf-8").splitlines()
                     if l.strip()]
            return QUOTE_JS.format(fallback=lines[len(lines) // 2])
        frag = os.path.join(FRAGMENTS, target)
        if os.path.exists(frag):
            with open(frag, encoding="utf-8") as f:
                return f.read().replace("{ROOT}", root).rstrip("\n")
        if target in files:
            return checkout(target).decode("utf-8").rstrip("\n")
        sys.exit(f"{page}: cannot resolve include {m.group(1)!r}")

    text = INCLUDE.sub(include, text)
    if page == "index.php":
        text = add_news(text)
    if "<?" in text:
        sys.exit(f"{page}: unhandled PHP left after include expansion")
    for pattern, new in REPLACEMENTS:
        text = pattern.sub(new, text)
    return ATTR.sub(lambda m: f'{m.group(1)}="{fix_url(m.group(2), root)}"',
                    text)


def check_links(out):
    """Report internal href/src targets that don't exist in the output.

    Returns the number of broken links that are not known historic ones."""
    bad = known = 0
    for dirpath, _, names in os.walk(out):
        for name in names:
            if not name.endswith(".html"):
                continue
            page = os.path.join(dirpath, name)
            rel = os.path.relpath(page, out)
            with open(page, encoding="utf-8") as f:
                html = re.sub(r"<!--.*?-->", "", f.read(), flags=re.S)
            for _, url in ATTR.findall(html):
                if re.match(r"[a-z][a-z0-9+.-]*:|#", url, re.I):
                    continue
                path = url.partition("#")[0].partition("?")[0]
                if os.path.exists(os.path.join(dirpath, path)):
                    continue
                if (url.startswith(DEAD_ON_ORIGINAL)
                        or (rel, url) in DEAD_ON_ORIGINAL_PAGES):
                    known += 1
                else:
                    print(f"broken link: {rel} -> {url}")
                    bad += 1
    if known:
        print(f"({known} links were already dead on the original site)")
    return bad


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1
                          else os.path.join(REPO, "docs"))
    if os.path.exists(out):
        if not os.path.exists(os.path.join(out, MARKER)):
            sys.exit(f"{out} exists and was not made by this script; "
                     "refusing to overwrite it")
        shutil.rmtree(out)

    files = set(head_files())
    pages = 0
    for path in sorted(files):
        if skipped(path):
            continue
        dest = os.path.join(out, path)
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        if path.endswith(".php"):
            with open(dest[:-4] + ".html", "w", encoding="utf-8") as f:
                f.write(render(path, files))
            pages += 1
        else:
            with open(dest, "wb") as f:
                f.write(checkout(path))
    for dirpath, _, names in os.walk(FRAGMENTS):
        for name in names:
            if name.endswith((".txt", ".php")) or name.startswith("."):
                continue
            rel = os.path.relpath(os.path.join(dirpath, name), FRAGMENTS)
            os.makedirs(os.path.dirname(os.path.join(out, rel)), exist_ok=True)
            shutil.copy(os.path.join(dirpath, name), os.path.join(out, rel))
    open(os.path.join(out, MARKER), "w").close()

    print(f"wrote {pages} pages to {os.path.relpath(out, REPO)}/")
    if check_links(out):
        sys.exit(1)


if __name__ == "__main__":
    main()
