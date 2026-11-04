"""Perspective/toon visual target for the planned OpenGL passes (not a game capture)."""
from pathlib import Path
import json
p=Path(__file__).with_name('render_design_board.py')
exec(p.read_text(encoding='utf-8').split("scene.render.filepath=str(O/'board_battle.png')")[0])
from bpy_extras.object_utils import world_to_camera_view
scene.render.resolution_x=1600;scene.render.resolution_y=900
scene.render.film_transparent=False;scene.cycles.samples=16
cam.data.type='PERSP';cam.data.lens=42
cam.location=(1,-12.8,12.6)
cam.rotation_euler=(Vector((0,-1.1,0))-cam.location).to_track_quat('-Z','Y').to_euler()
# Thin board; muted single surface. No nested mat or tall decorative rim.
board=bpy.data.objects['wood_board'];board.dimensions.z=.2;board.location.z=-.12
for ob in list(bpy.data.objects):
 if ob.name.startswith('wood_edge'):ob.hide_render=True
table=mat('table',(.23,.19,.16));cube('table',(0,0,-.34),(200,200,.2),table,0)

# The same normal/light quantization intended for lit.frag, with existing OBJ albedo.
for m in list(bpy.data.materials):
 if not m.use_nodes:continue
 nodes=m.node_tree.nodes;links=m.node_tree.links
 old=nodes.get('Principled BSDF')
 if not old:continue
 base=old.inputs['Base Color'].default_value[:]
 texture=next((n for n in nodes if n.type=='TEX_IMAGE' and n.image),None)
 for n in list(nodes):
  if n!=texture:nodes.remove(n)
 out=nodes.new('ShaderNodeOutputMaterial');em=nodes.new('ShaderNodeEmission')
 geom=nodes.new('ShaderNodeNewGeometry');dot=nodes.new('ShaderNodeVectorMath');dot.operation='DOT_PRODUCT'
 dot.inputs[1].default_value=(-.35,-.5,.79);links.new(geom.outputs['Normal'],dot.inputs[0])
 ramp=nodes.new('ShaderNodeValToRGB');ramp.color_ramp.interpolation='CONSTANT'
 ramp.color_ramp.elements[0].position=0;ramp.color_ramp.elements[0].color=(.42,.46,.52,1)
 ramp.color_ramp.elements[1].position=.65;ramp.color_ramp.elements[1].color=(1,1,1,1)
 ramp.color_ramp.elements.new(.2).color=(.73,.76,.79,1)
 links.new(dot.outputs['Value'],ramp.inputs[0])
 mix=nodes.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1;mix.inputs[1].default_value=base
 if texture:links.new(texture.outputs['Color'],mix.inputs[1])
 links.new(ramp.outputs[0],mix.inputs[2]);links.new(mix.outputs[0],em.inputs[0]);links.new(em.outputs[0],out.inputs[0])
scene.render.use_freestyle=True
scene.view_layers[0].freestyle_settings.linesets[0].linestyle.thickness=1.1
scene.view_layers[0].freestyle_settings.linesets[0].linestyle.color=(.10,.08,.075)

def glow(name,color):
 m=bpy.data.materials.new(name);m.use_nodes=True;n=m.node_tree.nodes;n.clear()
 o=n.new('ShaderNodeOutputMaterial');e=n.new('ShaderNodeEmission');e.inputs[0].default_value=(*color,1);m.node_tree.links.new(e.outputs[0],o.inputs[0]);return m
blue=glow('magic',(.22,.73,1));gold=glow('impact',(1,.68,.18));green=glow('range',(.25,.9,.66))
effects=[]
def line(name,pts,m,r=.025):
 c=bpy.data.curves.new(name,'CURVE');c.dimensions='3D';c.bevel_depth=r;c.bevel_resolution=2
 s=c.splines.new('POLY');s.points.add(len(pts)-1)
 for q,v in zip(s.points,pts):q.co=(*v,1)
 ob=bpy.data.objects.new(name,c);scene.collection.objects.link(ob);ob.data.materials.append(m);effects.append(ob);return ob
def ball(pos,r,m):
 bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1,radius=r,location=pos);ob=bpy.context.object;ob.data.materials.append(m);effects.append(ob)
# Mage projectile and compact hit burst, anchored to actual unit/enemy coordinates.
line('projectile',[(.5,-1.5,.85),(.25,-2,.9),(-.1,-2.5,.75)],blue,.035)
ball((-.1,-2.5,.75),.10,blue)
for i in range(7):
 t=i*math.tau/7;start=Vector((-3,-1,.65));d=Vector((math.cos(t)*.36,math.sin(t)*.36,.15))
 line('hit',[start+d*.5,start+d],gold,.023)
for ob in enemies:
 if ob.location.y==1 and ob.location.x==-3:ob.location.z+=.28
def project(v):
 q=world_to_camera_view(scene,cam,Vector(v));return [q.x*1600,(1-q.y)*900]
bpy.context.view_layer.update()
(O/'ux_v4_anchors.json').write_text(json.dumps({'hit':project((-3,-1,1.3)),'enemy':project((3,1,1.25)),'spawn':project((1.5,-.5,.3))}),encoding='utf-8')
scene.render.filepath=str(O/'stage_v4_battle.png');bpy.ops.render.render(write_still=True)
for ob in effects:ob.hide_render=True
# Placement view: world-space ring and actual translucent OBJ ghost.
ghost=figure('mage_1',1.5,-.5,size=1)
ghostmat=bpy.data.materials.new('placement_ghost');ghostmat.use_nodes=True
n=ghostmat.node_tree.nodes;n.clear();o=n.new('ShaderNodeOutputMaterial');tr=n.new('ShaderNodeBsdfTransparent');em=n.new('ShaderNodeEmission');em.inputs[0].default_value=(.2,1,.7,1);mix=n.new('ShaderNodeMixShader');mix.inputs[0].default_value=.38
ghostmat.node_tree.links.new(tr.outputs[0],mix.inputs[1]);ghostmat.node_tree.links.new(em.outputs[0],mix.inputs[2]);ghostmat.node_tree.links.new(mix.outputs[0],o.inputs[0])
for ob in ghost:ob.data.materials.clear();ob.data.materials.append(ghostmat)
line('range_ring',[(1.5+1.8*math.cos(i*math.tau/96),-.5+1.8*math.sin(i*math.tau/96),.27) for i in range(97)],green,.018)
line('target_tile',[(1.04,-.96,.28),(1.96,-.96,.28),(1.96,-.04,.28),(1.04,-.04,.28),(1.04,-.96,.28)],green,.025)
scene.render.filepath=str(O/'stage_v4_place.png');bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(O/'stage_v4.blend'))

