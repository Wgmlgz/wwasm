# WWASM liblary

`WWASM` (Wgmlgz wasm) - is a `c++` & `reactjs` liblary for connecting `c++` backend and `reactjs` frontend.

## Features:

- **IO**:
  Using `io[Get/Set][double/int]` methods read/write numbers at both sides.

- **WCanvas**:
  create image stream at `c++` side using `wasm::Canvas` class and then render it to react `<WCanvas/>` component.

  Canvas features:

  - Store entities
  - Ddraw lines
  - Draw n-gons
  - Draw preloaded `.bmp` images
  - Zoom & drag with mouse
  - Auto resizing to match window size
  - Multiple instances

# Converting images

To use custom images in `wwasm` you need to convert them to bytes array.
The easiest way to do this is to use this command (linux or wsl).

`$ xxd -i imagename.bmp > imagename.hpp`

# Demo:

![](./screenshot.png)

## TODO:

- Animations
