import React, { useEffect, useRef, useState } from 'react';
import ReactDOM from 'react-dom/client';
import CodeEditor from '@uiw/react-textarea-code-editor';
import { loadPyodide } from 'pyodide';
import { setupCanvas } from './canvas';
import pythonCode from './samples/editor.py';
import { Spinner } from './spinner';

export const Editor = () => {
  const [code, setCode] = useState(pythonCode);
  const canvasRef = useRef(null);
  const pyodideRef = useRef(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    let anim = null;
    let uninstallEventHandlers = null;

    const controller = new AbortController();

    const init = async () => {
      const pyodide = await loadPyodide({ indexURL: '/otto-simulator/' });

      if (controller.signal.aborted) {
        return;
      }

      await pyodide.loadPackage([
        '/otto-simulator/zengl-1.13.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
        '/otto-simulator/canvas-0.1.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
        '/otto-simulator/ottosim-0.1.0-cp311-cp311-emscripten_3_1_32_wasm32.whl',
      ]);

      if (controller.signal.aborted) {
        return;
      }

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

      if (!controller.signal.aborted) {
        setLoading(false);
      }
    };

    init();

    return () => {
      cancelAnimationFrame(anim);
      uninstallEventHandlers?.();
      controller.abort();
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
        <CodeEditor
          language="py"
          value={code}
          onChange={(evt) => setCode(evt.target.value)}
          padding={15}
          style={{
            width: '50%',
            height: '100%',
            resize: 'none',
            border: 'none',
            fontFamily: 'Inconsolata,monospace',
            fontSize: 12,
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
      {loading && (
        <div
          style={{
            width: '100vw',
            height: '100vh',
            backgroundColor: '#222',
            position: 'fixed',
            top: '0',
            left: '0',
          }}
        >
          <Spinner />
        </div>
      )}
    </div>
  );
};

const root = ReactDOM.createRoot(document.getElementById('root'));

root.render((
  <React.StrictMode>
    <Editor />
  </React.StrictMode>
));
