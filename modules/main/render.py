import zengl


def make_pipeline(ctx, framebuffer, uniform_buffer, vertex_buffer, bones, bone, first_vertex, vertex_count):
    pipeline = ctx.pipeline(
        vertex_shader='''
            #version 300 es
            precision highp float;

            vec3 qtransform(vec4 q, vec3 v) {
                return v + 2.0 * cross(cross(v, q.xyz) - q.w * v, q.xyz);
            }

            layout (std140) uniform Common {
                mat4 mvp;
            };

            uniform vec3 position;
            uniform vec4 rotation;

            layout (location = 0) in vec3 in_vertex;
            layout (location = 1) in vec3 in_normal;
            layout (location = 2) in vec3 in_color;

            out vec3 v_vertex;
            out vec3 v_normal;
            out vec3 v_color;

            void main() {
                v_vertex = position + qtransform(rotation, in_vertex);
                v_normal = normalize(qtransform(rotation, in_normal));
                gl_Position = mvp * vec4(v_vertex, 1.0);
                v_color = in_color;
            }
        ''',
        fragment_shader='''
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
        ''',
        layout=[
            {
                'name': 'Common',
                'binding': 0,
            },
        ],
        resources=[
            {
                'type': 'uniform_buffer',
                'binding': 0,
                'buffer': uniform_buffer,
            },
        ],
        uniforms={
            'position': [0.0, 0.0, 0.0],
            'rotation': [0.0, 0.0, 0.0, 1.0],
        },
        uniform_data=bones[bone * 28 : bone * 28 + 28],
        framebuffer=framebuffer,
        topology='triangles',
        cull_face='back',
        vertex_buffers=zengl.bind(vertex_buffer, '3f 3f 4nu1', 0, 1, 2),
        first_vertex=first_vertex,
        vertex_count=vertex_count,
    )
    return pipeline


class Renderer:
    def __init__(self, ctx: zengl.Context, size):
        self.ctx = ctx
        self.aspect = size[0] / size[1]
        self.image = ctx.image(size, 'rgba8unorm', samples=16)
        self.depth = ctx.image(size, 'depth24plus', samples=16)
        self.image.clear_value = (0.0, 0.0, 0.0, 1.0)
        framebuffer = [self.image, self.depth]

        self.uniform_buffer = ctx.buffer(size=80)
        self.vertex_buffer = ctx.buffer(open('dist/otto.mesh.bin', 'rb').read())
        self.bones = memoryview(bytearray(168))
        self.shapes = [
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 0, 0, 774),
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 1, 774, 180),
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 2, 954, 336),
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 3, 1290, 180),
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 4, 1470, 336),
            make_pipeline(self.ctx, framebuffer, self.uniform_buffer, self.vertex_buffer, self.bones, 5, 1806, 1248),
        ]

    def init(self, eye, target):
        self.ctx.new_frame()
        camera = zengl.camera(eye, target, aspect=self.aspect, fov=45.0)
        self.uniform_buffer.write(camera)
        self.image.clear()
        self.depth.clear()

    def render(self, env):
        self.bones[:140] = env.bones()
        for shape in self.shapes:
            shape.render()

    def flush(self):
        self.image.blit()
        self.ctx.end_frame()
