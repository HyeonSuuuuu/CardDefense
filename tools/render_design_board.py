"""Render the user's existing OBJ figures for UI composition; no game code edits."""
import bpy, math
from pathlib import Path
from mathutils import Vector
R=Path(__file__).resolve().parents[1]; O=R/'assets'/'design_preview';O.mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24
scene.render.resolution_x=1500;scene.render.resolution_y=870;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA';scene.render.film_transparent=True
scene.view_settings.view_transform='Standard'
def mat(n,c):
 m=bpy.data.materials.new(n);m.diffuse_color=(*c,1);m.use_nodes=True;p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*c,1);p.inputs['Roughness'].default_value=.85;return m
wood=mat('sunlit_wood',(.50,.29,.15));edge=mat('edge',(.71,.44,.23));grass=mat('grass',(.40,.61,.24));stone=mat('stone',(.81,.71,.53));soil=mat('soil',(.20,.25,.12))
for material,filename in [(wood,'board_wood'),(edge,'board_wood'),(grass,'grass'),(stone,'path_stone')]:
 nodes=material.node_tree.nodes;links=material.node_tree.links;tex=nodes.new('ShaderNodeTexImage');tex.image=bpy.data.images.load(str(R/'assets'/'textures'/(filename+'.png')))
 coords=nodes.new('ShaderNodeTexCoord');links.new(coords.outputs['Generated'],tex.inputs['Vector']);links.new(tex.outputs['Color'],nodes.get('Principled BSDF').inputs['Base Color'])
def cube(n,loc,scale,m,bevel=.06):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.name=n;o.dimensions=scale;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(m)
 if bevel:mod=o.modifiers.new('rounded','BEVEL');mod.width=bevel;mod.segments=3;o.modifiers.new('normals','WEIGHTED_NORMAL')
 return o
cube('wood_board',(0,0,-.25),(7.6,7.6,.6),wood,.16)
cube('inset',(0,0,.04),(7.1,7.1,.15),soil)
for x in [-3.65,3.65]:cube('wood_edge',(x,0,.08),(.22,7.6,.2),edge)
for y in [-3.65,3.65]:cube('wood_edge',(0,y,.08),(7.6,.22,.2),edge)
for i in range(4):
 for j in range(4):cube('placement_%s_%s'%(i,j),(i-1.5,j-1.5,.16),(.95,.95,.16),grass,.06)
for i in range(-3,4):
 for j in range(-3,4):
  if abs(i)==3 or abs(j)==3:cube('path_%s_%s'%(i,j),(i,j,.14),(.95,.95,.12),stone,.08)
figures=[]
def figure(id,x,y,rot=0,size=1):
 before=set(bpy.data.objects);bpy.ops.wm.obj_import(filepath=str(R/'assets'/'obj'/(id+'.obj')),forward_axis='Z',up_axis='Y')
 new=set(bpy.data.objects)-before
 for ob in new:
  ob.location=Vector((x,y,.26));ob.rotation_euler.z=rot;ob.scale*=size
  for m in ob.data.materials:
   if m and m.use_nodes:
    p=m.node_tree.nodes.get('Principled BSDF')
    if p:p.inputs['Roughness'].default_value=1;p.inputs['Specular IOR Level'].default_value=.1
 figures.extend(new);return list(new)
figure('warrior_1',-1.5,-1.5);figure('archer_1',-.5,-1.5);figure('mage_1',.5,-1.5)
figure('mage_2',-.5,.5);figure('archer_2',1.5,.5);figure('warrior_2',-1.5,1.5)
enemies=[]
for id,x,y in [('enemy_skeleton',-3,-1),('enemy_skeleton',-3,1),('enemy_goblin',-2,3),('enemy_goblin',1,3),('enemy_orc',3,1)]:enemies+=figure(id,x,y,size=.82)
bpy.ops.object.camera_add(location=(8,-13,12));cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,.1))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=11.7;scene.camera=cam
scene.world.color=(.65,.65,.65)
for pos,energy,size in [((-4,-5,10),1600,7),((5,4,8),1100,6)]:
 bpy.ops.object.light_add(type='AREA',location=pos);l=bpy.context.object;l.data.energy=energy;l.data.shape='DISK';l.data.size=size;l.rotation_euler=(-l.location).to_track_quat('-Z','Y').to_euler()
scene.render.filepath=str(O/'board_battle.png');bpy.ops.render.render(write_still=True)
for ob in enemies:ob.hide_render=True
scene.render.filepath=str(O/'board_prepare.png');bpy.ops.render.render(write_still=True)
for ob in figures:ob.hide_render=True
scene.render.filepath=str(O/'board_empty.png');bpy.ops.render.render(write_still=True)
