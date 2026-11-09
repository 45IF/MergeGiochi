"""Esperimento COIN_NXG respinto: NON produce lo stud richiesto da Dario.

Solo per riprodurre la diagnosi:
Blender -b --python crea_stud_3d.py -- --diagnostic-coin coin.blend vecchio_dyn_cash.txd uscita
TTGames-Workshop deve avere importato COIN_NXG.GSC nel blend di ingresso.
DragonFF si trova in ../../strumenti. Gli output rimangono fuori dal repository.
COIN_NXG e' una moneta con 10, non il pickup LEGO: non installare gli output.
"""
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'strumenti'))
import DragonFF
from DragonFF.gtaLib.txd import txd
from DragonFF.ops.txd_exporter import txd_exporter


def main():
    args = sys.argv[sys.argv.index('--') + 1:]
    if not args or args[0] != '--diagnostic-coin':
        raise SystemExit('COIN_NXG respinto da Dario: non e lo stud richiesto. '
                         'Solo diagnosi esplicita con --diagnostic-coin; vedere docs/stud-ricerca.md.')
    source, old_txd, out = [Path(p).resolve() for p in args[1:]]
    out.mkdir(parents=True, exist_ok=True)
    assert source.resolve() != (out / 'stud.blend').resolve()
    hud = txd()
    hud.load_file(str(old_txd))
    icon = next(t for t in hud.native_textures if t.name == 'lmsh_stud')
    icon_hash = hashlib.sha256(icon.to_mem()).hexdigest()
    bpy.ops.wm.open_mainfile(filepath=str(source))
    DragonFF.register()
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    assert sorted(len(o.data.vertices) for o in meshes) == [24, 72], 'COIN originale inatteso'
    points = [o.matrix_world @ v.co for o in meshes for v in o.data.vertices]
    lo = Vector([min(p[i] for p in points) for i in range(3)])
    hi = Vector([max(p[i] for p in points) for i in range(3)])
    center = (lo + hi) / 2
    scale = .35 / max((hi - lo).x, (hi - lo).y)
    rotation = Matrix.Rotation(math.pi / 2, 4, 'X')
    original_image = next(i for i in bpy.data.images if i.name.lower() == 'coin_nxg / texture 3')
    # Blender carica i pixel impacchettati pigramente: has_data prima dell'accesso
    # può essere falso anche per una DDS valida (audit: 128x128, 65536 componenti).
    original_pixels = original_image.pixels[:]
    assert len(original_pixels) == original_image.size[0] * original_image.size[1] * 4
    image = bpy.data.images.new('lmsh_coin3', *original_image.size, alpha=True)
    image.pixels[:] = original_pixels
    image.filepath_raw = str(out / 'lmsh_coin3.png')
    image.file_format = 'PNG'
    image.save()
    report = {'source': str(source), 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
              'diameter_m': .35, 'hud_native_sha256': icon_hash, 'parts': [],
              'tint_note': 'Oro nel materiale RW: approssimazione del colore shader, non una nuova texture.'}
    # Il colore di pickup viene dal materiale/shader; RW non riproduce lo shader TT.
    gold = (1.0, .72, .08, 1.0)
    for index, obj in enumerate(meshes):
        mesh = obj.data
        uv_before = [tuple(v.uv) for v in mesh.uv_layers[0].data]
        report['parts'].append({'source_name': obj.name, 'vertices': len(mesh.vertices),
                                'faces': len(mesh.polygons)})
        world = obj.matrix_world.copy()
        obj.parent = None
        obj.matrix_world = Matrix.Identity(4)
        for v in mesh.vertices:
            v.co = rotation @ ((world @ v.co - center) * scale)
        # Rotazione propria: conserva ordine delle facce e orientamento delle normali.
        mesh.update()
        assert all(p.area > 0 for p in mesh.polygons)
        for attr in list(mesh.color_attributes):
            mesh.color_attributes.remove(attr)  # evita doppia modulazione col colore TT
        side = obj.name.endswith('/ 1')
        mat = bpy.data.materials.new('StudGoldSide' if side else 'StudGoldFace')
        mat.use_nodes = True
        mat.diffuse_color = gold
        bsdf = mat.node_tree.nodes.get('Principled BSDF')
        bsdf.inputs['Base Color'].default_value = gold
        if side:
            tex = mat.node_tree.nodes.new('ShaderNodeTexImage')
            tex.image = image
            mat.node_tree.links.new(tex.outputs['Color'], bsdf.inputs['Base Color'])
        mesh.materials.clear()
        mesh.materials.append(mat)
        for p in mesh.polygons:
            p.material_index = 0
        assert uv_before == [tuple(v.uv) for v in mesh.uv_layers[0].data]
        obj.name = 'Money' if index == 0 else 'MoneySide'
        obj.dff.export_normals = True
        obj.dff.light = True
        obj.dff.modulate_color = True
    for obj in list(bpy.context.scene.objects):
        if obj not in meshes:
            bpy.data.objects.remove(obj, do_unlink=True)
    txd_exporter.version = 0x36003
    txd_exporter.only_used_textures = True
    txd_exporter.path = str(out)
    target_txd = out / 'dyn_cash.txd'
    txd_exporter.export_textures(meshes, str(target_txd))
    result = txd()
    result.load_file(str(target_txd))
    result.native_textures.append(icon)
    result.write_file(str(target_txd), 0x36003)
    verify = txd()
    verify.load_file(str(target_txd))
    assert hashlib.sha256(next(t for t in verify.native_textures if t.name == 'lmsh_stud').to_mem()).hexdigest() == icon_hash
    report['textures'] = [t.name for t in verify.native_textures]
    bpy.ops.export_dff.scene(filepath=str(out / 'Money.dff'), directory=str(out),
                             only_selected=False, export_version='0x36003')
    bpy.ops.wm.save_as_mainfile(filepath=str(out / 'stud.blend'))
    (out / 'verifica.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print('STUD_3D_OK', report)


if __name__ == '__main__':
    main()
