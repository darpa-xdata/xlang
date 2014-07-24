XLANG WebGraph Example
======================

This is a simple example project for x-language interop.  The basic
driver is a small julia script that controls download, ETL and
coordination of analytics and visualization services.  Graph analysis
is done by:

- XXX in R
- YYY in Python
- ZZZ in Java/scala

Resulting reduced graphs are then visualized via:

- WWW in Python
- UUU in ???

Prerequisites
-------------
- [julia v0.3 or later](https://github.com/JuliaLang/julia)
- [julia GZip package](https://github.com/kmsquire/GZip.jl)
- [julia Stage package](https://github.com/saltpork/Stage.jl)
- [thunderdome/xlang](https://github.com/darpa-xdata/xlang)
- curl or wget
- git
- ...

Running webgraph.jl
-------------------
To run this project (assuming julia is in your path):

```
$ julia webgraph.jl
```

Note: this script will attempt to download the PLD webgraph from
[Web data commons](http://webdatacommons.org/hyperlinkgraph/). If
you're behind a proxy, make sure that you set `http_proxy` and
`https_proxy` environment variables accordingly

