"""Full 3D stage with coherent table, board, lighting and contact shadows."""
from pathlib import Path
p=Path(__file__).with_name('render_design_board.py')
exec(p.read_text(encoding='utf-8').split("scene.render.filepath=str(O/'board_battle.png')")[0])
scene.render.resolution_x=1600;scene.render.resolution_y=900
scene.render.film_transparent=False
table=mat('table_surface',(.22,.18,.14))
cube('single_3d_table',(0,0,-.66),(200,200,.22),table,.02)
cam.location=(2,-15,12)
cam.rotation_euler=(Vector((0,-.65,0))-cam.location).to_track_quat('-Z','Y').to_euler()
cam.data.ortho_scale=15.5
scene.cycles.samples=32
scene.render.filepath=str(O/'stage_v3_battle.png');bpy.ops.render.render(write_still=True)
for ob in enemies:ob.hide_render=True
scene.render.filepath=str(O/'stage_v3_prepare.png');bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(O/'stage_v3.blend'))

