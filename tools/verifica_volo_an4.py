"""Blender: prova clip di volo LMSH1 su una copia del rig nativo Iron Man M6."""
import bpy
import json
import math
from pathlib import Path
import sys

base = Path(sys.argv[sys.argv.index('--') + 1]).resolve()
out = base / 'estratti/verifica-volo-an4'
out.mkdir(parents=True, exist_ok=True)
sys.path.insert(0, str(base / 'estratti/verifica-ironman-m6/addon'))
import io_scene_tt_character as addon
addon.register()
from io_scene_tt_character.importer import import_catalog_entry
bpy.ops.wm.open_mainfile(filepath=str(base / 'estratti/verifica-ironman-m6/LMSH1_IRONMANM6.blend'))
rig = next(o for o in bpy.data.objects if o.type == 'ARMATURE' and 'tt_character_skeleton' in o)
entries = [json.loads(item.payload) for item in rig.tt_animation_assets]
flight = [e for e in entries if 'IRONMAN_FLY_AN4_NXG.PAK' in str(e).upper()]
report = {'available': flight, 'tests': []}
for entry in flight:
    if entry['name'].lower() not in ('hover', 'fly', 'fly_land'):
        continue
    result = import_catalog_entry(bpy.context, rig, entry)
    finite = True
    if result.get('imported'):
        frames = result['clips'][0]['frames']
        for frame in (1, max(1, frames // 2), frames):
            bpy.context.scene.frame_set(frame)
            finite &= all(math.isfinite(v) for bone in rig.pose.bones for row in bone.matrix for v in row)
    report['tests'].append({'entry': entry, 'result': result, 'finite': finite})
(out / 'esito.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
if any(t['result'].get('imported') for t in report['tests']):
    bpy.ops.wm.save_as_mainfile(filepath=str(out / 'IRONMANM6_volo.blend'))
print('AN4_TEST', len(flight), len(report['tests']))
