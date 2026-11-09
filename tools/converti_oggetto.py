"""MergeGiochi - converte un oggetto statico di LMSH1 (es. lo scudo di Captain America) in DFF+TXD per GTA SA.
Scritto da Claude. Uso: python3 converti_oggetto.py <blend LMSH1> <sottostringa mesh> <nome> <scala> <cartella uscita>
Stessa convenzione del convertitore dei personaggi: scambio X<->Y (LMSH +Y avanti -> SA +X avanti), scala come
il personaggio che lo porta, centrato sull'origine. Solo geometria, UV e texture originali."""
import sys, os
sys.path.insert(0, '/home/claude/ref')
import bpy, bmesh
from mathutils import Vector
import DragonFF
try: DragonFF.register()
except Exception as e: print('DragonFF', e)
blend, chiave, nome, scala, out = sys.argv[-5], sys.argv[-4], sys.argv[-3], float(sys.argv[-2]), sys.argv[-1]
os.makedirs(out, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=blend)
src = next(o for o in bpy.data.objects if o.type == 'MESH' and chiave in o.name)
me = src.data
pts = [src.matrix_world @ v.co for v in me.vertices]
centro = Vector([(min(p[i] for p in pts) + max(p[i] for p in pts)) / 2 for i in range(3)])
img = None
for n in me.materials[0].node_tree.nodes:
    if n.type == 'TEX_IMAGE' and n.image and 'normal' not in n.image.name.lower():
        img = n.image; break
w, h = img.size
pixels = list(img.pixels[:])
bpy.ops.wm.read_factory_settings(use_empty=True)
DragonFF.register() if not hasattr(bpy.ops.import_scene, 'dff') else None
tex = bpy.data.images.new(nome, w, h, alpha=True); tex.pixels[:] = pixels
tex.filepath_raw = os.path.join(out, nome + '.png'); tex.file_format = 'PNG'; tex.save()
bm = bmesh.new(); uvl = bm.loops.layers.uv.new('UVMap')
vs = []
for p in pts:
    q = (p - centro) * scala
    vs.append(bm.verts.new((q.y, q.x, q.z)))
src_uv = None
# UV dalla mesh originale (riaperta per leggerle)
bpy.ops.wm.open_mainfile(filepath=blend)
src = next(o for o in bpy.data.objects if o.type == 'MESH' and chiave in o.name)
facce = [(tuple(pl.vertices), [tuple(src.data.uv_layers[0].data[li].uv) for li in pl.loop_indices]) for pl in src.data.polygons]
bpy.ops.wm.read_factory_settings(use_empty=True)
DragonFF.register() if not hasattr(bpy.ops.import_scene, 'dff') else None
bm2 = bmesh.new(); uvl2 = bm2.loops.layers.uv.new('UVMap')
vs2 = [bm2.verts.new(((p - centro) * scala).yxz) for p in pts]
bm2.verts.ensure_lookup_table()
for f, uvs in facce:
    try: fa = bm2.faces.new([vs2[i] for i in reversed(f)])   # invertite: lo scambio X<->Y è una riflessione
    except ValueError: continue
    for loop, uv in zip(fa.loops, reversed(uvs)): loop[uvl2].uv = uv
mesh = bpy.data.meshes.new(nome); bm2.to_mesh(mesh); bm2.free()
mat = bpy.data.materials.new(nome); mat.use_nodes = True
tn = mat.node_tree.nodes.new('ShaderNodeTexImage'); tn.image = bpy.data.images.load(os.path.join(out, nome + '.png')); tn.image.name = nome
mat.node_tree.links.new(tn.outputs['Color'], mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'])
mesh.materials.append(mat)
obj = bpy.data.objects.new(nome, mesh); bpy.context.scene.collection.objects.link(obj)
from DragonFF.ops import txd_exporter as _txe
_txe.txd_exporter.version = 0x36003; _txe.txd_exporter.only_used_textures = True; _txe.txd_exporter.path = out
_txe.txd_exporter.export_textures([obj], os.path.join(out, nome + '.txd'))
bpy.ops.export_dff.scene(filepath=os.path.join(out, nome + '.dff'), directory=out, only_selected=False, export_version='0x36003')
print('fatto', [(f, os.path.getsize(os.path.join(out, f))) for f in os.listdir(out)])
