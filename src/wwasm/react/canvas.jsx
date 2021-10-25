import React from "react";
import "./canvas.css";
import { ioGetDouble, ioSetDouble, drawCanvas } from "./wwasm.js";

export default function Canvas() {
  let w = 1000;
  let h = 400;

  let canvas_cpp_id = 0;
  let canvas_id = "main_canvas";

  let zoom = 1;

  let offset_x = 0;
  let offset_y = 0;

  let down_x = 0;
  let down_y = 0;

  let is_down = false;

  function wheel(event) {
    zoom *= 1 + event.deltaY * -0.001 * 1.5;
    zoom = Math.min(Math.max(0.1, zoom), 100);
    ioSetDouble(3, zoom);
  }

  function btnDown(e) {
    is_down = true;
    down_x = -e.screenX;
    down_y = e.screenY;
  }

  function btnUp(e) {
    is_down = false;
    offset_x = ioGetDouble(1);
    offset_y = ioGetDouble(2);
  }

  function dragCanvas(e) {
    let x = -e.screenX;
    let y = e.screenY;

    if (is_down) {
      x = (x - down_x) * (1 / zoom) + offset_x;
      y = (y - down_y) * (1 / zoom) + offset_y;
      ioSetDouble(1, x);
      ioSetDouble(2, y);
    }
  }

  const interval = setInterval(() => {
    w = window.innerWidth;
    h = window.innerHeight;
    document.getElementById(canvas_id).width = w;
    document.getElementById(canvas_id).height = h;
    drawCanvas(canvas_id, canvas_cpp_id, w, h);
  }, 16);
  return (
    <canvas
      id={canvas_id}
      onMouseDown={btnDown}
      onMouseUp={btnUp}
      onMouseMove={dragCanvas}
      onWheel={wheel}
    ></canvas>
  );
}
