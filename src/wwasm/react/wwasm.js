import React, { Component, useEffect, useState } from "react";
import WWasmModule from "./wwasm.mjs";

export let ioGetInt = () => null;
export let ioSetInt = () => null;
export let ioGetDouble = () => null;
export let ioSetDouble = () => null;

export let getCanvasData = () => 0;
export let readUint8Array = () => null;

WWasmModule().then((Module) => {
  ioSetInt = Module.cwrap("ioSetInt", "number", ["number", "number"]);
  ioGetInt = Module.cwrap("ioGetInt", "number", ["number"]);

  ioSetDouble = Module.cwrap("ioSetDouble", "number", ["number", "number"]);
  ioGetDouble = Module.cwrap("ioGetDouble", "number", ["number"]);

  getCanvasData = Module.cwrap("getCanvasData", "number", [
    "number",
    "number",
    "number",
  ]);
  readUint8Array = (ptr, size) => {
    return new Uint8Array(Module.HEAPU8.buffer, ptr, size);
  };
});

function writeImageDataToCanvas(canvas, data, width, height) {
  canvas.width = width;
  canvas.height = height;
  var context = canvas.getContext("2d");
  var imageData = context.createImageData(width, height);
  if (imageData === undefined || imageData === null) return;
  if (data === undefined || data === null) return;
  imageData.data.set(data);
  context.putImageData(imageData, 0, 0);
}

export function drawCanvas(canvas_id, id, w, h) {
  let ptr = getCanvasData(id, w, h);
  if (!ptr) return;
  writeImageDataToCanvas(
    document.getElementById(canvas_id),
    readUint8Array(ptr, w * h * 4),
    w,
    h
  );
}
