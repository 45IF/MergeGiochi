"""Audit Blender del modello LMSH1; ripristina su disco solo DDS originali impacchettate."""
import bpy
import json
from pathlib import Path
import sys

out = Path(sys.argv[sys.argv.index('--') + 1]).resolve()
out.mkdir(parents=True, exist_ok=True)
audit_only = '--audit-only' in sys.argv
report = {'images': [], 'meshes': [], 'materials': []}
for i, image in enumerate(bpy.data.images):
    row = {'name': image.name, 'size': list(image.size), 'packed': bool(image.packed_file)}
    if image.packed_file and not audit_only:
        data = bytes(image.packed_file.data)
        if data[:4] == b'DDS ':
            path = out / ('original_%03d.dds' % i)
            path.write_bytes(data)
            image.filepath_raw = str(path)
            image.reload()
            row['original_dds'] = str(path)
    row['pixels'] = len(image.pixels)
    report['images'].append(row)
for obj in bpy.data.objects:
    if obj.type == 'MESH':
        report['meshes'].append({'name': obj.name, 'vertices': len(obj.data.vertices),
          'uv': [uv.name for uv in obj.data.uv_layers], 'groups': len(obj.vertex_groups),
          'weighted_vertices': sum(bool(v.groups) for v in obj.data.vertices),
          'materials': [m.name if m else None for m in obj.data.materials]})
for mat in bpy.data.materials:
    report['materials'].append({'name': mat.name, 'images': [n.image.name for n in mat.node_tree.nodes
       if n.type == 'TEX_IMAGE' and n.image] if mat.use_nodes else []})
(out / 'audit.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
if audit_only:
    print('AUDIT_COMPLETATO', out / 'audit.json')
    sys.exit(0)
bpy.context.scene.render.filepath = str(out / 'verifica.png')
bpy.ops.wm.save_as_mainfile(filepath=str(out / 'ironman-texture.blend'))
bpy.ops.render.render(write_still=True)
