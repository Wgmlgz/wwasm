# WWASM liblary

`WWASM` (Wgmlgz wasm) - is a `c++` & `reactjs` liblary for connecting `c++` backend and `reactjs` frontend.

## Features:

- **IO**:
  Using `io[Get/Set][double/int]` methods read/write numbers at both sides

- **WCanvas**:
  create image stream at `c++` side using `wasm::Canvas` class and then render it to react `<WCanvas/>` component.
  (for now there can be only 1 instance of this canvas per app)

  With this canvas you can also use mouse to drag and zoom it

![](./screenshot.png)

## TODO:

- Storing entities
- Animations
