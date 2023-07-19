import React, { useEffect, useRef, useState } from 'react';
import { loadPyodide } from 'pyodide';
import { setupCanvas } from './canvas';
import pythonCode from './samples/editor.py';

export const Editor = () => {
  const [code, setCode] = useState(pythonCode);
  const canvasRef = useRef(null);
  const pyodideRef = useRef(null);

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

      const gl = canvasRef.current.getContext('webgl2', {
        alpha: false, depth: false, stencil: false, antialias: false,
        premultipliedAlpha: false, preserveDrawingBuffer: false,
        powerPreference: 'high-performance',
      });

      uninstallEventHandlers = setupCanvas(pyodide, canvasRef.current, gl);

      pyodide.runPython(pythonCode);
      pyodideRef.current = pyodide;

      const render = () => {
        const updateCallback = pyodide.globals.get('update');
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

  const executeCode = () => {
    if (pyodideRef.current) {
      pyodideRef.current.runPython(code);
    }
  };

  return (
    <div
      style={{
        width: '100vw',
        height: '100vh',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <div
        style={{
          width: '100vw',
          height: 'calc(100vh - 60px)',
          overflow: 'hidden',
          display: 'flex',
        }}
      >
        <textarea
          value={code}
          onChange={(evt) => setCode(evt.target.value)}
          style={{
            width: '50%',
            height: '100%',
            resize: 'none',
            border: 'none',
          }}
        />
        <canvas
          ref={canvasRef}
          width={1024}
          height={1024}
          style={{
            width: '50%',
            height: '100%',
          }}
        />
      </div>
      <div
        style={{
          height: '60px',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
        }}
      >
      <button
        onClick={executeCode}
      >
        Execute
      </button>
      </div>
    </div>
  );
};
