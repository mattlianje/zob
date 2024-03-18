# zob
「Zen · Org · Binder」My OrgMode-like tool for UNIX systems. 
<p align="center">
  <img src="pix/gandalf-study.jpeg" width="500" alt="Gandalf in Gondorian library">
</p>

Inspired by the [suckless philosophy](https://suckless.org/philosophy/) all programs
in the ZOB ecosystem are:
- Adhere to the UNIX philosophy and do one thing
- Are written in <1000 lines of frugal C99

The ZOB binary aims above all:
- To have a small memory footprint
- Link against the fewest possible libraries (libcurl is being used for now as raw sockets
do not support tls)

The ZOB binary binary does not have a runtime config, and is rebuilt as the
`ZOBMASTER` sees fit. It uses an in process [SQLite](https://www.sqlite.org/index.html) database which persists
to `<ZOB_DB_NAME>.db` at the root of the `ZOB_DIRECTORY` chosen by the `ZOBMASTER`.
The `ZOBMASTER` pseudonym is used as the <author> for zob typsetting programs.

<p align="center">
  <img src="pix/zob.svg" width="500" alt="The zobosystem">
</p>

# programs
```
zob             — zen task manager
zob rss         — simple RSS feed reader
zob fmt         — wrapper for all your code linters
zob tex         — zob-markdown -> LaTeX generator
zob html        — zob-markdown -> HTML generator
zob tengwar     — ???
```
