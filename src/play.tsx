import React, { useEffect, useRef } from 'react';
import { loadPyodide } from 'pyodide';
import { setupCanvas } from './canvas';
import pythonCode from './samples/play.py';

export const Play = () => {
  const ref = useRef(null);

  useEffect(() => {
    let anim = null;
    let uninstallEventHandlers = null;

    const init = async () => {
      const pyodide = await loadPyodide({ indexURL: '/otto-simulator/' });

      await pyodide.loadPackage([
        '/otto-simulator/zengl-1.13.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
        '/otto-simulator/canvas-0.1.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
        '/otto-simulator/ottosim-0.1.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
      ]);

      const gl = ref.current.getContext('webgl2', {
        alpha: false, depth: false, stencil: false, antialias: false,
        premultipliedAlpha: false, preserveDrawingBuffer: false,
        powerPreference: 'high-performance',
      });

      uninstallEventHandlers = setupCanvas(pyodide, ref.current, gl);

      pyodide.runPython(pythonCode);
      const updateCallback = pyodide.globals.get('update');

      const render = () => {
        updateCallback();
        anim = requestAnimationFrame(render);
      };

      anim = requestAnimationFrame(render);
    };

    init();

    return () => {
      cancelAnimationFrame(anim);
      uninstallEventHandlers?.();
    }
  }, []);

  return (
    <div
      style={{
        width: '100vw',
        height: '100vh',
        overflow: 'hidden',
      }}
    >
      <canvas
        ref={ref}
        width={1280}
        height={720}
        style={{
          width: '100vw',
          height: '100vh',
        }}
      />
    </div>
  );
};
