# regex

A regular expression matcher implemented in C23.

## Note

***This is a practice project. It is very incomplete, very inefficient, and may have fatal flaws. Do not use it in any serious setting.***

Supported regular expression operators:

```
\ . ? * + | ( )
```

Supported charset: **ASCII** only.

## Build

This project uses **`meson`** as the build tool. If you haven't installed it yet, please install it first, along with `ninja`.

Since the code uses **C23** features, please ensure your compiler version is recent enough.

Run following commands to build the project.

```sh
meson setup builddir
meson compile -C builddir
```

The path to the executable file is `builddir/regex`.
