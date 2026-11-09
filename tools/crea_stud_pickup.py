"""MergeGiochi - modello del pickup dei soldi (Money, ID 1212, txd dyn_cash) con lo stud d'oro di LMSH1.
Scritto da Claude. Geometria e UV dal quad originale dello stud di LMSH1 (THINGS_NXG / gold_pan_coin1: 4 vertici,
UV 0..1), scalato come i personaggi; texture originale THINGS_NXG / texture 103 (stud d'oro). Doppia faccia,
perché il gioco fa girare i pickup su se stessi."""
import sys, os
sys.path.insert(0, '/home/claude/ref')
import bpy, bmesh
from mathutils import Vector
import DragonFF
try: DragonFF.register()
except Exception as e: print(e)
THINGS, OUT = sys.argv[-2], sys.argv[-1]
os.makedirs(OUT, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=THINGS)
q = next(o for o in bpy.data.objects if o.type == 'MESH' and 'gold_pan_coin1' in o.name)
pts = [q.matrix_world @ v.co for v in q.data.vertices]
facce = [(tuple(p.vertices), [tuple(q.data.uv_layers[0].data[li].uv) for li in p.loop_indices]) for p in q.data.polygons]
img = bpy.data.images['THINGS_NXG / texture 103']
w, h = img.size; px = list(img.pixels[:])
centro = sum(pts, Vector()) / len(pts)
lato = max((p - centro).length for p in pts)
S = 0.32 / lato          # raggio circa 0.32 m in gioco
bpy.ops.wm.read_factory_settings(use_empty=True)
DragonFF.register() if not hasattr(bpy.ops.import_scene, 'dff') else None
tex = bpy.data.images.new('lmsh_stud', w, h, alpha=True); tex.pixels[:] = px
tex.filepath_raw = os.path.join(OUT, 'lmsh_stud.png'); tex.file_format = 'PNG'; tex.save()
bm = bmesh.new(); uvl = bm.loops.layers.uv.new('UVMap')
# il quad di LMSH1 (importato) sta nel piano XZ: in gioco resta in piedi (piano XZ) e il gioco lo fa girare.
# (prima usava p.y, che sul quad è costante: il pickup diventava una linea di spessore zero, cioè invisibile)
# due copie dei vertici (fronte e retro): con gli stessi vertici bmesh scarta la faccia rovesciata
for verso in (1, -1):
    vs = [bm.verts.new(((p.x - centro.x) * S, 0.001 * verso, (p.z - centro.z) * S)) for p in pts]
    for f, uv in facce:
        idx = list(f) if verso == 1 else list(reversed(f))
        uvs = uv if verso == 1 else list(reversed(uv))
        fa = bm.faces.new([vs[i] for i in idx])
        for loop, c in zip(fa.loops, uvs): loop[uvl].uv = c
mesh = bpy.data.meshes.new('Money'); bm.to_mesh(mesh); bm.free()
mat = bpy.data.materials.new('lmsh_stud'); mat.use_nodes = True
tn = mat.node_tree.nodes.new('ShaderNodeTexImage'); tn.image = bpy.data.images.load(os.path.join(OUT, 'lmsh_stud.png')); tn.image.name = 'lmsh_stud'
mat.node_tree.links.new(tn.outputs['Color'], mat.node_tree.nodes['Principled BSDF'].inputs['Base Color'])
mesh.materials.append(mat)
obj = bpy.data.objects.new('Money', mesh); bpy.context.scene.collection.objects.link(obj)
from DragonFF.ops import txd_exporter as _txe
_txe.txd_exporter.version = 0x36003; _txe.txd_exporter.only_used_textures = True; _txe.txd_exporter.path = OUT
_txe.txd_exporter.export_textures([obj], os.path.join(OUT, 'dyn_cash.txd'))
bpy.ops.export_dff.scene(filepath=os.path.join(OUT, 'Money.dff'), directory=OUT, only_selected=False, export_version='0x36003')
print('fatto', sorted((f, os.path.getsize(os.path.join(OUT, f))) for f in os.listdir(OUT)))
