import React, { useEffect } from 'react';
import { useRef } from 'react';

const vec = (x, y, z) => ({ x, y, z });
const sub = (a, b) => ({ x: a.x - b.x, y: a.y - b.y, z: a.z - b.z });
const cross = (a, b) => ({ x: a.y * b.z - a.z * b.y, y: a.z * b.x - a.x * b.z, z: a.x * b.y - a.y * b.x });
const dot = (a, b) => a.x * b.x + a.y * b.y + a.z * b.z;
const normalize = (a) => {
  const l = Math.sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  return { x: a.x / l, y: a.y / l, z: a.z / l };
};

const camera = (eye, target, aspect) => {
  const up = { x: 0.0, y: 0.0, z: 1.0 };
  const fov = 60.0;
  const znear = 0.1;
  const zfar = 1000.0;

  const f = normalize(sub(target, eye));
  const s = normalize(cross(f, up));
  const u = cross(s, f);
  const t = { x: -dot(s, eye), y: -dot(u, eye), z: -dot(f, eye) };

  const r0 = Math.tan(fov * 0.008726646259971647884618453842);
  const r1 = r0 * Math.max(1.0 / aspect, 1.0);
  const r2 = r0 * Math.max(aspect, 1.0);
  const r3 = (zfar + znear) / (zfar - znear);
  const r4 = (2.0 * zfar * znear) / (zfar - znear);

  return new Float32Array([
    s.x / r2, u.x / r1, r3 * f.x, f.x,
    s.y / r2, u.y / r1, r3 * f.y, f.y,
    s.z / r2, u.z / r1, r3 * f.z, f.z,
    t.x / r2, t.y / r1, r3 * t.z - r4, t.z,
  ]);
};

const vertexShaderCode = `
  #version 300 es
  precision highp float;

  vec3 qtransform(vec4 q, vec3 v) {
    return v + 2.0 * cross(cross(v, q.xyz) - q.w * v, q.xyz);
  }

  uniform mat4 mvp;

  uniform vec3 position;
  uniform vec4 rotation;

  uniform int flip;

  layout (location = 0) in vec3 in_vertex;
  layout (location = 1) in vec3 in_normal;
  layout (location = 2) in vec3 in_color;

  out vec3 v_vertex;
  out vec3 v_normal;
  out vec3 v_color;

  void main() {
    v_vertex = position + qtransform(rotation, in_vertex);
    v_normal = normalize(qtransform(rotation, in_normal));
    if (flip == 1) {
      v_vertex.z *= -1.0;
      v_normal.z *= -1.0;
    }
    gl_Position = mvp * vec4(v_vertex, 1.0);
    v_color = in_color;
  }
`;

const fragmentShaderCode = `
  #version 300 es
  precision highp float;

  in vec3 v_vertex;
  in vec3 v_normal;
  in vec3 v_color;

  layout (location = 0) out vec4 out_color;

  void main() {
    vec3 light = vec3(4.0, -3.0, 10.0);
    float lum = dot(normalize(light), normalize(v_normal)) * 0.3 + 0.7;
    out_color = vec4(v_color * lum, 1.0);
  }
`;

export const Demo = () => {
  const resolution = 1200;
  const ref = useRef(null);

  useEffect(() => {
    let anim = null;

    const download = async (url) => {
      const res = await fetch(url);
      const data = await res.arrayBuffer();
      return data;
    };

    const downloadResources = async () => {
      const [mesh, frames] = await Promise.all([
        download('otto.mesh.bin'),
        download('otto.frames.bin'),
      ]);
      return { mesh, frames: new Float32Array(frames) };
    };

    const init = async () => {
      const resources = await downloadResources();

      const gl = ref.current.getContext('webgl2') as WebGL2RenderingContext;

      const vbo = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
      gl.bufferData(gl.ARRAY_BUFFER, resources.mesh, gl.STATIC_DRAW);

      const vao = gl.createVertexArray();
      gl.bindVertexArray(vao);
      gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 28, 0);
      gl.vertexAttribPointer(1, 3, gl.FLOAT, false, 28, 12);
      gl.vertexAttribPointer(2, 4, gl.UNSIGNED_BYTE, true, 28, 24);
      gl.enableVertexAttribArray(0);
      gl.enableVertexAttribArray(1);
      gl.enableVertexAttribArray(2);

      const vs = gl.createShader(gl.VERTEX_SHADER);
      const fs = gl.createShader(gl.FRAGMENT_SHADER);
      gl.shaderSource(vs, vertexShaderCode.trim());
      gl.shaderSource(fs, fragmentShaderCode.trim());
      gl.compileShader(vs);
      gl.compileShader(fs);

      const prog = gl.createProgram();
      gl.attachShader(prog, vs);
      gl.attachShader(prog, fs);
      gl.linkProgram(prog);

      const mvpUniform = gl.getUniformLocation(prog, 'mvp');
      const positionUniform = gl.getUniformLocation(prog, 'position');
      const rotationUniform = gl.getUniformLocation(prog, 'rotation');
      const flipUniform = gl.getUniformLocation(prog, 'flip');

      const renderPart = (offset, vertices, bone) => {
        gl.uniform3fv(positionUniform, bone.subarray(0, 3));
        gl.uniform4fv(rotationUniform, bone.subarray(3, 7));
        gl.drawArrays(gl.TRIANGLES, offset, vertices);
      };

      gl.clearColor(0.0, 0.0, 0.0, 0.0);
      gl.blendColor(0.0, 0.0, 0.0, 0.8);
      gl.blendFunc(gl.CONSTANT_ALPHA, gl.ONE_MINUS_CONSTANT_ALPHA);

      let boneIndex = 0;

      const render = () => {
        const { clientWidth, clientHeight } = ref.current;
        const aspect = clientWidth / clientHeight;

        const bones = resources.frames.subarray(boneIndex * 35, boneIndex * 35 + 35);
        boneIndex = (boneIndex + 1) % Math.floor(resources.frames.length / 35);

        gl.viewport(0, 0, resolution, resolution);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        gl.useProgram(prog);
        gl.bindVertexArray(vao);
        gl.uniformMatrix4fv(mvpUniform, false, camera(vec(0.0, -0.3, 0.15), vec(0.0, 0.0, 0.07), aspect));

        gl.uniform1i(flipUniform, 1);

        gl.enable(gl.DEPTH_TEST);
        renderPart(0, 774, bones.subarray(0, 7));
        renderPart(774, 180, bones.subarray(7, 14));
        renderPart(954, 336, bones.subarray(14, 21));
        renderPart(1290, 180, bones.subarray(21, 28));
        renderPart(1470, 336, bones.subarray(28, 35));
        gl.disable(gl.DEPTH_TEST);

        gl.uniform1i(flipUniform, 0);

        gl.enable(gl.BLEND);
        renderPart(1806, 1248, new Float32Array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0]));
        gl.disable(gl.BLEND);

        gl.enable(gl.DEPTH_TEST);
        renderPart(0, 774, bones.subarray(0, 7));
        renderPart(774, 180, bones.subarray(7, 14));
        renderPart(954, 336, bones.subarray(14, 21));
        renderPart(1290, 180, bones.subarray(21, 28));
        renderPart(1470, 336, bones.subarray(28, 35));
        gl.enable(gl.DEPTH_TEST);

        anim = requestAnimationFrame(render);
      };

      anim = requestAnimationFrame(render);
    };

    init();

    return () => {
      cancelAnimationFrame(anim);
    };
  }, []);

  return (
    <canvas
      ref={ref}
      width={resolution}
      height={resolution}
      style={{
        width: '100vw',
        height: '100vh',
      }}
    />
  )
};
