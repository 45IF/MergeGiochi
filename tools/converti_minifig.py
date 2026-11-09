"""
MergeGiochi — conversione di un minifig LEGO Marvel Super Heroes (LMSH1) in un pedone di GTA San Andreas.
Scritto da Claude. Richiede Blender 5.x (anche come modulo bpy) e l'add-on DragonFF.

Principi (vincolanti, vedi AGENTS.md):
  - si usano SOLO mesh, pesi e texture originali del file LMSH1 importato con TTGames-Workshop;
  - lo scheletro è quello dei pedoni di SA (preso da un DFF originale di SA, es. player.img/torso.dff),
    con le articolazioni spostate sui perni del minifig;
  - i pesi sono quelli originali di TT, solo rinominati verso le ossa SA (nessun peso inventato);
  - il bacino viene abbassato rispetto alla radice: le animazioni di SA traslano solo l'osso radice
    ("Root"/"Normal"), quindi le gambe corte del minifig toccano terra senza deformarlo.

Uso:
  python3 converti_minifig.py config.json
"""
import sys, json, math, os
import bpy, bmesh
from mathutils import Vector, Matrix

# ---------------------------------------------------------------- configurazione
CFG = json.load(open(sys.argv[-1]))
DRAGONFF_DIR = CFG.get("dragonff_dir", "/home/claude/ref")
sys.path.insert(0, DRAGONFF_DIR)
import DragonFF
try:
    DragonFF.register()
except Exception as e:
    print("DragonFF register:", e)

SA_SKELETON_DFF = CFG["sa_skeleton_dff"]      # DFF originale SA con scheletro dei ped (es. torso.dff)
LMSH_BLEND = CFG["lmsh_blend"]                # .blend importato da TTGames-Workshop
OUT_DIR = CFG["out_dir"]
NAME = CFG["model_name"]                      # nome del modello in gioco (max 7-8 caratteri consigliato)
TARGET_HEIGHT = CFG.get("target_height", 1.8) # altezza totale in metri in SA
GROUND_Z = CFG.get("ground_z", -1.0)          # quota del suolo rispetto alla radice del ped
INCLUDE = CFG["include_meshes"]               # sottostringhe dei nomi delle mesh da includere

# mappa ossa TT -> ossa SA (nomi SA esatti, con gli spazi iniziali come nel DFF)
TT2SA = {
    "Character": " Pelvis", "Hips": " Pelvis", "Pelvis": " Pelvis",
    "SpineBase": " Spine 1", "Spine": " Spine 1",
    "Spine1": " Spine 2", "Spine2": " Spine 2",
    "LeftShoulder": " Spine 2", "RightShoulder": " Spine 2",
    "Neck": " Head", "NeckDriven": " Head", "NeckPeg_driven": " Head", "HeadPeg_driven": " Head",
    "Head": " Head", "Hat": " Head", "Hair": " Head", "Glasses": " Head",
    "Root": " Head", "Visor_Jnt": " Head", "Face": " Head", "L_eye": " Head", "R_eye": " Head",
    "L_Pupil": " Head", "R_Pupil": " Head",
}
TT2SA.update({"Breathe": " Spine 2", "Boobs_Jnt": " Spine 2", "BellyMid_Jnt": " Spine 1",
              "BellyLeft_Jnt": " Spine 1", "BellyRight_Jnt": " Spine 1", "Attach": " Pelvis"})
for side, sa in (("Left", "L"), ("Right", "R")):
    TT2SA.update({f"{side}ArmPegDriven": f" {sa} UpperArm", f"{side}WristStabiliser": f" {sa} Hand",
                  f"{side}IndexFinger": f" {sa} Hand", f"{side}PinkyFinger": f" {sa} Hand", f"{side}Thumb": f" {sa} Hand"})
for side, sa in (("Left", "L"), ("Right", "R")):
    TT2SA.update({
        f"{side}Arm": f" {sa} UpperArm", f"{side}ArmRoll": f" {sa} UpperArm", f"{side}ArmPeg_driven": f" {sa} UpperArm",
        f"{side}ForeArm": f" {sa} ForeArm", f"{side}Sleeve": f" {sa} ForeArm",
        f"{side}ForeArmRoll": f" {sa} Hand", f"{side}Hand": f" {sa} Hand", f"{side}Weapon": f" {sa} Hand",
        f"{side}UpLeg": f" {sa} Thigh", f"{side}Leg": f" {sa} Calf",
        f"{side}Foot": f" {sa} Foot", f"{side}ToeBase": f" {sa} Toe0",
    })
    for f in ("Index", "Pinky"):
        for i in (1, 2, 3):
            TT2SA[f"{side}{f}Finger{i}"] = f" {sa} Hand"
    for i in (1, 2, 3):
        TT2SA[f"{side}Thumb{i}"] = f" {sa} Hand"

# osso SA -> osso TT da cui prendere la posizione dell'articolazione
SA_JOINT_FROM_TT = {
    " Pelvis": "LeftUpLeg@mid",  # al centro fra le due anche
    " Spine 1": "SpineBase", " Spine 2": "Spine1", " Neck": "Neck", " Head": "Head",
    "Bip01 L Clavicle": "Neck", "Bip01 R Clavicle": "Neck",
}
for side, sa in (("Left", "L"), ("Right", "R")):
    SA_JOINT_FROM_TT.update({
        f" {sa} UpperArm": f"{side}Arm", f"{sa} breast": f"{side}Arm",
        f" {sa} ForeArm": f"{side}ForeArm", f" {sa} Hand": f"{side}Hand",
        f" {sa} Finger": f"{side}IndexFinger1", f"{sa} Finger01": f"{side}IndexFinger2",
        f" {sa} Finger0Nub": f"{side}IndexFinger3",
        f" {sa} Thigh": f"{side}UpLeg", f" {sa} Calf": f"{side}Leg",
        f" {sa} Foot": f"{side}Foot", f" {sa} Toe0": f"{side}ToeBase",
    })

# ---------------------------------------------------------------- 1. dati dal minifig LMSH1
bpy.ops.wm.open_mainfile(filepath=LMSH_BLEND)
tt_arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE' and o.parent is None)
# le superfici del viso stanno in una raccolta esclusa dalla vista: esclusa, Blender non la valuta e le mesh
# restano all'origine, fuori dalla testa. Si includono tutte le raccolte prima di valutare.
def _includi(lc):
    lc.exclude = False
    for c in lc.children: _includi(c)
_includi(bpy.context.view_layer.layer_collection)
bpy.context.view_layer.update()

# Raddrizzamento dei gomiti (big-fig come Hulk): nella posa di riposo TT gli avambracci sono piegati in avanti
# di ~90°, mentre lo scheletro SA ha il braccio dritto; le animazioni di SA aggiungono la loro piega a quella
# già presente e le mani finiscono sopra le spalle mentre corre. Si mette in posa l'armatura TT con
# l'avambraccio allineato al braccio e si usa quella posa (mesh deformate e posizioni delle articolazioni).
if CFG.get("raddrizza_gomiti"):
    from mathutils import Quaternion
    bpy.context.view_layer.objects.active = tt_arm
    bpy.ops.object.mode_set(mode='POSE')
    for side in ("Left", "Right"):
        pb = tt_arm.pose.bones
        arm_b, fore_b, hand_b = pb[side + "Arm"], pb[side + "ForeArm"], pb[side + "Hand"]
        bpy.context.view_layer.update()
        a, e, h = arm_b.head.copy(), fore_b.head.copy(), hand_b.head.copy()   # spazio armatura, posa attuale
        q = (h - e).rotation_difference(e - a)
        M = Matrix.Translation(e) @ q.to_matrix().to_4x4() @ Matrix.Translation(-e)
        fore_b.matrix = M @ fore_b.matrix
        bpy.context.view_layer.update()
        print(f"gomito {side} raddrizzato di {math.degrees(q.angle):.0f} gradi")
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update()
dg = bpy.context.evaluated_depsgraph_get()

# trasformazione LMSH -> SA: in LMSH il fronte è +Y e "Left" sta a +X; in SA il fronte è +X e "L" sta a +Y.
# Lo scambio X<->Y è una riflessione: le facce vanno poi invertite.
def swap_xy(v):
    return Vector((v.y, v.x, v.z))

# altezza del minifig (dalle mesh incluse)
def _visibile(o):
    return bpy.context.view_layer.objects.get(o.name) is not None and not o.hide_get()
if INCLUDE == ["*"]:
    src_meshes = [o for o in bpy.data.objects if o.type == 'MESH' and _visibile(o) and "ZHead" not in o.name
                  and not (o.data.materials and ("colourWriteMask" in o.data.materials[0].name
                                                 or "MouthBack" in o.data.materials[0].name))]
    # superfici del viso (occhi, pupille, bocca): TTGames-Workshop le mette nella raccolta "TT native facial
    # surfaces", esclusa dalla vista; servono, tranne la maschera di profondità (colourWriteMask) e l'interno
    # della bocca aperta (MouthBack), che senza la maschera sporgerebbe dalla testa
    for o in bpy.data.objects:
        if o.type == 'MESH' and o not in src_meshes and "FACE" in o.name and o.data.materials:
            mn = o.data.materials[0].name
            if "colourWriteMask" not in mn and "MouthBack" not in mn:
                src_meshes.append(o)
else:
    src_meshes = [o for o in bpy.data.objects if o.type == 'MESH' and any(s in o.name for s in INCLUDE)]
ESCLUDI = CFG.get("exclude_meshes", [])
src_meshes = [o for o in src_meshes if not any(e in o.name for e in ESCLUDI)]
OSSO_DEFAULT = CFG.get("default_bone", " Head")  # per mesh senza pesi (es. capelli attaccati alla testa)
print("mesh incluse:", [o.name for o in src_meshes])
assert src_meshes, "nessuna mesh trovata con include_meshes"
zs = []
for o in src_meshes:
    me = o.evaluated_get(dg).to_mesh()
    zs += [(o.matrix_world @ v.co).z for v in me.vertices]
    o.evaluated_get(dg).to_mesh_clear()
zmin, zmax = min(zs), max(zs)
S = TARGET_HEIGHT / (zmax - zmin)
def to_sa(p):  # punto mondo LMSH -> punto SA
    q = swap_xy(p) * S
    q.z += GROUND_Z - zmin * S
    return q
print(f"altezza minifig {zmax - zmin:.3f} -> scala {S:.2f}")

tt_joint = {b.name: tt_arm.matrix_world @ tt_arm.pose.bones[b.name].head for b in tt_arm.data.bones}  # posa attuale (= riposo se non toccata)
def joint(spec):
    if spec.endswith("@mid"):
        a = tt_joint["LeftUpLeg"]; b = tt_joint["RightUpLeg"]
        return to_sa((a + b) / 2)
    for nome in (spec, spec.replace("Finger1", "Finger"), spec.replace("Finger2", "Finger2")):
        if nome in tt_joint:
            return to_sa(tt_joint[nome])
    raise KeyError(spec)

# geometria valutata (in posa di riposo), coordinate SA, gruppi rinominati
parts = []
for o in src_meshes:
    oe = o.evaluated_get(dg)
    me = oe.to_mesh()
    verts = [to_sa(o.matrix_world @ v.co) for v in me.vertices]
    weights = []
    for v in o.data.vertices:   # pesi originali TT (stessa numerazione dei vertici)
        w = {}
        for g in v.groups:
            tt = o.vertex_groups[g.group].name
            sa = TT2SA.get(tt)
            if sa is None:
                print("ATTENZIONE osso TT non mappato:", tt, "->", " Head"); sa = " Head"
            w[sa] = w.get(sa, 0.0) + g.weight
        if not w or sum(w.values()) <= 0.0:
            w = {OSSO_DEFAULT: 1.0}
        weights.append(w)
    faces = [tuple(p.vertices) for p in me.polygons]
    # denti e lingua stanno nelle stesse mesh di pupille e occhi: sono le facce pesate solo sull'osso "Face"
    # (gli occhi seguono R_eye/L_eye...). In LMSH1 a bocca chiusa sono nascosti dalla maschera di profondità,
    # che qui non c'è: si tolgono quelle facce, altrimenti compare un blocco bianco al posto della bocca.
    mn = o.data.materials[0].name if o.data.materials else ""
    if "Teeth" in mn or "Tongue" in mn:
        def solo_face(i):
            gs = [o.vertex_groups[g.group].name for g in o.data.vertices[i].groups if g.weight > 0.01]
            return gs == ["Face"]
        tieni = [k for k, f in enumerate(faces) if not all(solo_face(i) for i in f)]
        print(f"  {o.name}: tolte {len(faces) - len(tieni)} facce di denti/lingua")
        faces = [faces[k] for k in tieni]
        _tieni_idx = tieni
    else:
        _tieni_idx = None
    uvs = None
    if me.uv_layers:
        uvl = me.uv_layers[0].data
        uvs = [[tuple(uvl[li].uv) for li in p.loop_indices] for p in me.polygons]
        if _tieni_idx is not None:
            uvs = [uvs[k] for k in _tieni_idx]
    cols = None
    ca = me.color_attributes.get("SourceColor")
    if ca is not None and ca.domain == 'POINT':
        cols = [tuple(c.color) for c in ca.data]
    mat = o.data.materials[0] if o.data.materials else None
    parts.append(dict(name=o.name, verts=verts, weights=weights, faces=faces, uvs=uvs, cols=cols, mat=mat))
    oe.to_mesh_clear()

# ---------------------------------------------------------------- 1b. oggetti di LMSH1 portati dal personaggio
# (richiesta di Dario: artigli e scudo sono "oggetti di gioco" che esistono solo con la skin giusta).
# Si fondono nel modello, pesati al 100% sull'osso SA indicato: con un'altra skin spariscono da soli.
# Config: "oggetti": [{"blend": ..., "mesh": sottostringa, "osso_sa": " R Hand",
#                      "ancora_mesh": sottostringa mesh del personaggio (centro del suo ingombro) | "ancora_osso": osso TT,
#                      "offset": [x,y,z] (unità LMSH), "rotazione": [x,y,z] gradi, "specchia_x": bool}]
from mathutils import Euler
for ogg in CFG.get("oggetti", []):
    with bpy.data.libraries.load(ogg["blend"], link=False) as (src, dst):
        dst.objects = [n for n in src.objects if ogg["mesh"] in n]
    for o in dst.objects:
        if o is None or o.type != 'MESH':
            continue
        bpy.context.scene.collection.objects.link(o)
        if "ancora_mesh" in ogg:
            am = next(x for x in bpy.data.objects if x.type == 'MESH' and ogg["ancora_mesh"] in x.name and x is not o)
            ame = am.evaluated_get(dg).to_mesh()
            ps = [am.matrix_world @ v.co for v in ame.vertices]
            ancora = Vector([(min(q[i] for q in ps) + max(q[i] for q in ps)) / 2 for i in range(3)])
            am.evaluated_get(dg).to_mesh_clear()
        else:
            ancora = tt_joint[ogg["ancora_osso"]].copy()
        R = Euler([math.radians(a) for a in ogg.get("rotazione", [0, 0, 0])]).to_matrix()
        off = Vector(ogg.get("offset", [0, 0, 0]))
        sx = -1.0 if ogg.get("specchia_x") else 1.0
        me = o.data
        verts = []
        for v in me.vertices:
            q = R @ (o.matrix_world @ v.co)
            q.x *= sx
            verts.append(to_sa(ancora + off + q))
        faces = [tuple(pl.vertices) for pl in me.polygons]
        if sx < 0:
            faces = [tuple(reversed(f)) for f in faces]
        uvs = None
        if me.uv_layers:
            uvl = me.uv_layers[0].data
            uvs = [[tuple(uvl[li].uv) for li in pl.loop_indices] for pl in me.polygons]
            if sx < 0:
                uvs = [list(reversed(u)) for u in uvs]
        cols = None
        ca = me.color_attributes.get("SourceColor")
        if ca is not None and ca.domain == 'POINT':
            cols = [tuple(c.color) for c in ca.data]
        parts.append(dict(name="oggetto " + o.name + (" (specchiato)" if sx < 0 else ""), verts=verts,
                          weights=[{ogg["osso_sa"]: 1.0} for _ in verts], faces=faces, uvs=uvs, cols=cols,
                          mat=me.materials[0] if me.materials else None))
        print("oggetto aggiunto:", o.name, "su", ogg["osso_sa"], "vertici", len(verts))

# immagini e colori dei materiali originali
def lin2srgb(c):
    return 12.92 * c if c <= 0.0031308 else 1.055 * (c ** (1 / 2.4)) - 0.055
part_mats = []
for p in parts:
    m = p["mat"]
    img = None
    if m and m.node_tree:
        for n in m.node_tree.nodes:
            if n.type == 'TEX_IMAGE' and n.image and "glow" not in n.image.name.lower() \
                    and "normal" not in n.image.name.lower() and "normal" not in (n.label or "").lower():
                img = n.image; break
    p["mini"] = None
    if img is not None and img.size[0] <= 16 and img.size[1] <= 16:
        # texture LEGO minuscola (8x8, palette di colori): si campiona al centro UV di ogni faccia e il colore
        # va nel materiale (moltiplicato per il colore dei vertici, che in TT contiene l'occlusione)
        p["mini"] = (img.size[0], img.size[1], list(img.pixels[:]))
        img = None
    # colore costante del materiale TT (nodo Mix MULTIPLY con un ingresso non collegato, es. capelli neri)
    p["costante"] = None
    if m and m.node_tree:
        for n in m.node_tree.nodes:
            if n.type == 'MIX_RGB' and n.blend_type == 'MULTIPLY':
                for nome in ("Color1", "Color2"):
                    i = n.inputs[nome]
                    if not i.is_linked:
                        p["costante"] = tuple(lin2srgb(x) for x in i.default_value[:3])
    p["image"] = img.name if img else None
    p["image"] = img.name if img else None
# salvataggio delle texture originali come PNG (servono al TXD)
os.makedirs(OUT_DIR, exist_ok=True)
tex_files = {}
for p in parts:
    img = bpy.data.images[p["image"]] if p["image"] else None
    if img and img.name not in tex_files:
        short = CFG.get("texture_names", {}).get(img.name) or (NAME + "_" + str(len(tex_files)))
        path = os.path.join(OUT_DIR, short + ".png")
        # copia dei pixel originali (la DDS impacchettata non si salva direttamente)
        w, h = img.size
        copia = bpy.data.images.new(short, w, h, alpha=True)
        copia.pixels[:] = img.pixels[:]
        copia.filepath_raw = path; copia.file_format = 'PNG'; copia.save()
        tex_files[img.name] = (short, path)
print("texture:", tex_files)

# ---------------------------------------------------------------- 2. scheletro SA
bpy.ops.wm.read_factory_settings(use_empty=True)
DragonFF.register() if not hasattr(bpy.ops.import_scene, "dff") else None
bpy.ops.import_scene.dff(filepath=SA_SKELETON_DFF)
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
for o in list(bpy.data.objects):        # via la mesh di CJ: resta solo lo scheletro originale
    if o.type == 'MESH':
        bpy.data.objects.remove(o, do_unlink=True)

bpy.context.view_layer.objects.active = arm
bpy.ops.object.mode_set(mode='EDIT')
eb = arm.data.edit_bones
orig_head = {b.name: b.head.copy() for b in eb}
new_head = {}
for b in eb:
    if b.name in SA_JOINT_FROM_TT:
        new_head[b.name] = joint(SA_JOINT_FROM_TT[b.name])
# ossa senza corrispondenza (radice, nub, viso, belly): seguono il genitore con lo stesso offset scalato
scale_rel = S * 0.26   # rapporto indicativo minifig/CJ per gli offset secondari
def place(b):
    if b.name in new_head:
        return new_head[b.name]
    if b.parent is None:
        new_head[b.name] = b.head.copy(); return new_head[b.name]
    ph = place(b.parent)
    off = orig_head[b.name] - orig_head[b.parent.name]
    new_head[b.name] = ph + off * min(1.0, scale_rel)
    return new_head[b.name]
for b in eb:
    place(b)
# Nub della testa in cima al casco
if " HeadNub" in new_head:
    new_head[" HeadNub"] = Vector((new_head[" Head"].x, new_head[" Head"].y, GROUND_Z + TARGET_HEIGHT))
for b in eb:   # traslazione rigida: stessa direzione, lunghezza e roll dell'osso SA originale
    d = new_head[b.name] - b.head
    b.head += d; b.tail += d
bpy.ops.object.mode_set(mode='OBJECT')
print("radice->bacino:", (new_head[" Pelvis"] - new_head[arm.data.bones[0].name]))

# ---------------------------------------------------------------- 3. un'unica mesh skinnata
bm = bmesh.new()
groups = [b.name for b in arm.data.bones]
uv_layer = bm.loops.layers.uv.new("UVMap")
dl = bm.verts.layers.deform.verify()
mats, mat_index = [], {}
def material_for(p, color=None):
    img = p["image"]
    key = (img, tuple(round(c, 3) for c in color) if color else None)
    if key in mat_index:
        return mat_index[key]
    m = bpy.data.materials.new(NAME + "_m" + str(len(mats)))
    m.use_nodes = True
    bsdf = m.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (*(color or (1, 1, 1)), 1.0)
    if img:
        short, path = tex_files[img]
        im = bpy.data.images.load(path); im.name = short
        tn = m.node_tree.nodes.new("ShaderNodeTexImage"); tn.image = im; tn.label = short
        m.node_tree.links.new(tn.outputs["Color"], bsdf.inputs["Base Color"])
    mats.append(m); mat_index[key] = len(mats) - 1
    return mat_index[key]

for p in parts:
    base = len(bm.verts)
    vs = []
    for co, w in zip(p["verts"], p["weights"]):
        v = bm.verts.new(co)
        tot = sum(w.values()) or 1.0
        for g, val in w.items():
            v[dl][groups.index(g)] = val / tot
        vs.append(v)
    bm.verts.ensure_lookup_table()
    for fi, f in enumerate(p["faces"]):
        # colore del materiale: dal colore per vertice originale (solo per mesh senza texture)
        color = None
        if p["image"] is None:
            if p["cols"] is not None:
                conta = {}
                for i in f:
                    k = tuple(round(lin2srgb(x) * 31) / 31 for x in p["cols"][i][:3])
                    conta[k] = conta.get(k, 0) + 1
                c = max(conta, key=conta.get)
            else:
                c = (1.0, 1.0, 1.0)
            t = (1.0, 1.0, 1.0)
            if p["mini"] and p["uvs"]:
                w_, h_, px = p["mini"]
                uu = sum(uv[0] for uv in p["uvs"][fi]) / len(p["uvs"][fi])
                vv = sum(uv[1] for uv in p["uvs"][fi]) / len(p["uvs"][fi])
                x = int((uu % 1.0) * w_) % w_; y = int((vv % 1.0) * h_) % h_
                t = tuple(px[(y * w_ + x) * 4 + k] for k in range(3))
            k_ = p["costante"] or (1.0, 1.0, 1.0)
            color = tuple(round(c[k] * t[k] * k_[k] * 23) / 23 for k in range(3))
        elif p["cols"] is not None:
            media = [sum(lin2srgb(cc[k]) for cc in p["cols"]) / len(p["cols"]) for k in range(3)]
            if min(media) < 0.9:
                color = tuple(round(x * 31) / 31 for x in media)
        mi = material_for(p, color)
        try:
            face = bm.faces.new([vs[i] for i in reversed(f)])   # invertite per la riflessione X<->Y
        except ValueError:
            continue
        face.material_index = mi
        if p["uvs"]:
            for loop, uv in zip(face.loops, reversed(p["uvs"][fi])):
                loop[uv_layer].uv = (uv[0], uv[1])

mesh = bpy.data.meshes.new(NAME)
bm.to_mesh(mesh); bm.free()
for m in mats:
    mesh.materials.append(m)
obj = bpy.data.objects.new(NAME, mesh)
bpy.context.scene.collection.objects.link(obj)
for g in groups:
    obj.vertex_groups.new(name=g)
obj.parent = arm
mod = obj.modifiers.new("Armature", 'ARMATURE'); mod.object = arm
mesh.shade_smooth() if hasattr(mesh, "shade_smooth") else None

# ---------------------------------------------------------------- 4. esportazione
blend_out = os.path.join(OUT_DIR, NAME + ".blend")
bpy.ops.wm.save_as_mainfile(filepath=blend_out)
txd_out = os.path.join(OUT_DIR, NAME + ".txd")
from DragonFF.ops import txd_exporter as _txe
_txe.txd_exporter.version = 0x36003
_txe.txd_exporter.only_used_textures = True
_txe.txd_exporter.path = OUT_DIR
_txe.txd_exporter.export_textures([obj], txd_out)
dff_out = os.path.join(OUT_DIR, NAME + ".dff")
bpy.ops.export_dff.scene(filepath=dff_out, directory=OUT_DIR, only_selected=False, export_version='0x36003')

print("TXD:", txd_out, os.path.getsize(txd_out) if os.path.exists(txd_out) else "MANCANTE")
print("DFF:", dff_out, os.path.getsize(dff_out))
print("materiali:", [(m.name, [round(x, 3) for x in m.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value[:3]]) for m in mats])
