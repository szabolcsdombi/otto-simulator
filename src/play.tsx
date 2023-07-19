import React, { useEffect, useRef } from 'react';
import { loadPyodide } from 'pyodide';
import { setupCanvas } from './canvas';
import pythonCode from './samples/play.py';

export const Play = () => {
  const ref = useRef(null);

  useEffect(() => {
    (async () => {
      const pyodide = await loadPyodide({ indexURL: '/otto-simulator/' });

      await pyodide.loadPackage([
        '/otto-simulator/zengl-1.13.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
        '/otto-simulator/canvas-0.1.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
      ]);

      const gl = ref.current.getContext('webgl2', {
        alpha: false, depth: false, stencil: false, antialias: false,
        premultipliedAlpha: false, preserveDrawingBuffer: false,
        powerPreference: 'high-performance',
      });

      setupCanvas(pyodide, ref.current, gl);

      pyodide.runPython(pythonCode);
      const updateCallback = pyodide.globals.get('update');

      const render = () => {
        // const { clientWidth, clientHeight } = ref.current;
        updateCallback();
        requestAnimationFrame(render);
      };

      requestAnimationFrame(render);
    })();
  }, []);

  return (
    <canvas
      ref={ref}
      width={1280}
      height={720}
      style={{
        width: 1280,
        height: 720,
      }}
    />
  );
};
