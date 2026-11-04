from pathlib import Path
p=Path(__file__).with_name('render_ux_v4.py')
exec(p.read_text(encoding='utf-8-sig').split("scene.render.filepath=str(O/'stage_v4_battle.png')")[0])
for ob in enemies+effects:ob.hide_render=True
scene.render.filepath=str(O/'stage_v5_prepare.png');bpy.ops.render.render(write_still=True)
