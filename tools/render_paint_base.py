"""판 원화용 구조 렌더 (AI 화풍 입히기의 기준 이미지).

게임 기본 카메라(Common.h Camera{} + Renderer.cpp)와 같은 시점으로 판만 렌더한다.
말·적은 게임에서 3D로 그리므로 넣지 않는다.

실행 (프로젝트 폴더에서):
  "F:/Program Files/Blender Foundation/Blender 5.2/blender.exe" -b -P tools/render_paint_base.py

결과: assets/paint/
  board_base.png   AI에 줄 구조 렌더 (텍스처 + 외곽선)
  board_flat.png   단색 + 외곽선 (칸 경계 검수용)
  board_paint.blend  같은 장면·카메라 (다음 단계: 원화 투사 후 텍스처 굽기)
"""
import bpy, math
from pathlib import Path
from mathutils import Vector

R = Path(__file__).resolve().parents[1]
O = R / 'assets' / 'paint'; O.mkdir(exist_ok=True)

# ---- 게임 카메라 값 (CardDefense/Common.h, Renderer.cpp) ----
GAME_DISTANCE = 11.0   # Camera::distance
GAME_PITCH = 50.0      # Camera::pitch (도)
GAME_FOVY = 45.0       # glm::perspective 세로 시야각 (도)
RES_X, RES_Y = 1920, 1080  # 게임 창 1280x720과 같은 16:9

bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.render.engine = 'CYCLES'; scene.cycles.samples = 32
scene.render.resolution_x = RES_X; scene.render.resolution_y = RES_Y; scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = 'PNG'; scene.render.image_settings.color_mode = 'RGB'
scene.view_settings.view_transform = 'Standard'
scene.world.color = (.55, .55, .55)

def mat(n, c, tex=None):
    m = bpy.data.materials.new(n); m.use_nodes = True
    p = m.node_tree.nodes.get('Principled BSDF')
    p.inputs['Base Color'].default_value = (*c, 1); p.inputs['Roughness'].default_value = .85
    if tex:
        nodes, links = m.node_tree.nodes, m.node_tree.links
        t = nodes.new('ShaderNodeTexImage'); t.image = bpy.data.images.load(str(R / 'assets' / 'textures' / (tex + '.png')))
        co = nodes.new('ShaderNodeTexCoord'); links.new(co.outputs['Generated'], t.inputs['Vector'])
        links.new(t.outputs['Color'], p.inputs['Base Color'])
    return m

def cube(n, loc, size, m, bevel=.06):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc); o = bpy.context.object; o.name = n
    o.dimensions = size; bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    o.data.materials.append(m)
    if bevel:
        mod = o.modifiers.new('rounded', 'BEVEL'); mod.width = bevel; mod.segments = 3
        o.modifiers.new('normals', 'WEIGHTED_NORMAL')
    return o

# ---- 판 (render_design_board.py와 같은 배치) ----
wood = mat('wood', (.50, .29, .15), 'board_wood'); edge = mat('edge', (.71, .44, .23), 'board_wood')
grass = mat('grass', (.40, .61, .24), 'grass'); stone = mat('stone', (.81, .71, .53), 'path_stone')
soil = mat('soil', (.20, .25, .12)); table = mat('table', (.36, .26, .18), 'table_wood')

cube('wood_board', (0, 0, -.25), (7.6, 7.6, .6), wood, .16)
cube('inset', (0, 0, .04), (7.1, 7.1, .15), soil)
for x in [-3.65, 3.65]: cube('wood_edge', (x, 0, .08), (.22, 7.6, .2), edge)
for y in [-3.65, 3.65]: cube('wood_edge', (0, y, .08), (7.6, .22, .2), edge)
for i in range(4):
    for j in range(4): cube('place_%d_%d' % (i, j), (i - 1.5, j - 1.5, .16), (.95, .95, .16), grass)
for i in range(-3, 4):
    for j in range(-3, 4):
        if abs(i) == 3 or abs(j) == 3: cube('path_%d_%d' % (i, j), (i, j, .14), (.95, .95, .12), stone, .08)
cube('table', (0, 0, -.6), (60, 60, .2), table, 0)

# ---- 카메라: 게임 기본 시점과 일치 ----
# 게임(Y-up): 카메라 = Rx(-pitch)·(0,0,distance) → (0, d·sinθ, d·cosθ), 원점을 바라봄
# Blender(Z-up): (x, y, z)_game → (x, -z, y)
th = math.radians(GAME_PITCH)
cam_loc = Vector((0, -GAME_DISTANCE * math.cos(th), GAME_DISTANCE * math.sin(th)))
bpy.ops.object.camera_add(location=cam_loc); cam = bpy.context.object; cam.name = 'game_camera'
cam.rotation_euler = (Vector((0, 0, 0)) - cam_loc).to_track_quat('-Z', 'Y').to_euler()
cam.data.type = 'PERSP'; cam.data.sensor_fit = 'VERTICAL'; cam.data.angle_y = math.radians(GAME_FOVY)
cam.data.clip_start = .1; cam.data.clip_end = 100
scene.camera = cam

# ---- 빛: 왼쪽 위 (원화 프롬프트와 같은 방향) ----
bpy.ops.object.light_add(type='SUN', location=(-5, -5, 10)); sun = bpy.context.object
sun.data.energy = 3.0; sun.data.angle = math.radians(8)
sun.rotation_euler = (Vector((0, 0, 0)) - sun.location).to_track_quat('-Z', 'Y').to_euler()

# ---- 외곽선 (AI가 칸 경계를 따라 그리도록) ----
scene.render.use_freestyle = True
ls = scene.view_layers[0].freestyle_settings.linesets[0].linestyle
ls.thickness = 1.6; ls.color = (.10, .08, .07)

scene.render.filepath = str(O / 'board_base.png'); bpy.ops.render.render(write_still=True)

# 단색 버전: 텍스처를 끊고 기본색만
for m in bpy.data.materials:
    if m.use_nodes:
        p = m.node_tree.nodes.get('Principled BSDF')
        for l in list(p.inputs['Base Color'].links): m.node_tree.links.remove(l)
scene.render.filepath = str(O / 'board_flat.png'); bpy.ops.render.render(write_still=True)

bpy.ops.wm.save_as_mainfile(filepath=str(O / 'board_paint.blend'))
print('PAINT_BASE_DONE', cam_loc)
