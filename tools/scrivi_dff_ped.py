"""
MergeGiochi — scrive il DFF finale di un pedone sullo scheletro di un PEDONE ORIGINALE di SA (es. male01).
Scritto da Claude.

Lezione dell'8/11: lo scheletro dei vestiti di CJ (player.img) fa crashare il gioco (0x4D2C3A) quando un
altro modello lo usa come giocatore; i pedoni normali (32 ossa, es. male01) invece funzionano.

Cosa fa:
  - frame, rotazioni, HAnim, atomic e flag restano quelli del DFF originale del pedone;
  - si spostano solo le POSIZIONI delle articolazioni (lette dal .blend di converti_minifig.py);
  - la geometria (vertici, UV, normali, materiali, pesi) viene dal DFF esportato da converti_minifig.py,
    portata nello spazio del frame dell'atomic (il bacino in male01);
  - pesi rimappati per ID d'osso; matrici di skin con la convenzione verificata su male01 (errore 0):
        skin_k = inverse( W_k * inverse(W_frameAtomic) )   (vettori riga, RenderWare)

Uso: python3 scrivi_dff_ped.py <dff_pedone_SA> <blend_convertito> <dff_geometria> <dff_scheletro_cj> <dff_uscita>
"""
import sys
sys.path.insert(0, "/home/claude/ref")
import numpy as np
import bpy
from DragonFF.gtaLib import dff

base_path, blend_path, geo_path, cj_path, out_path = sys.argv[-5:]

# nomi delle ossa nello scheletro CJ usato dal convertitore -> nomi nei pedoni normali
CJ2PED = {" Spine 1": " Spine", " Spine 2": " Spine1", "Ripped": "Root"}
# ossa che esistono solo in CJ -> osso del pedone che le sostituisce (per ID)
ID_SOSTITUTI = {303: 5, 304: 36, 305: 26, 306: 44, 307: 54}

def local(f):
    M = np.eye(4); M[:3, :3] = np.array(f.rotation_matrix); M[3, :3] = np.array(f.position); return M

def worlds(F):
    W = [None] * len(F)
    for i, f in enumerate(F):
        W[i] = local(f) if f.parent < 0 else local(f) @ W[f.parent]
    return W

# --- articolazioni nuove (spazio di Blender = spazio locale della radice)
bpy.ops.wm.open_mainfile(filepath=blend_path)
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
new_head = {CJ2PED.get(b.name, b.name): np.array(b.head_local) for b in arm.data.bones}

# --- ordine HAnim dello scheletro CJ (per leggere gli indici dei pesi della geometria)
dcj = dff.dff(); dcj.load_file(cj_path)
ccj = dcj.clumps[0]
cj_order = [b.id for b in next(f for f in ccj.frame_list if f.bone_data and f.bone_data.bones).bone_data.bones]

# --- geometria dal convertitore
dg = dff.dff(); dg.load_file(geo_path)
geo = dg.clumps[-1].geometry_list[0]

# --- pedone originale (caricato per ultimo: DragonFF accoda i clump fra istanze)
d = dff.dff(); d.load_file(base_path); c = d.clumps[-1]; F = c.frame_list
root_i = next(i for i, f in enumerate(F) if f.bone_data and f.bone_data.bones)
a_i = c.atomic_list[0].frame
W = worlds(F)
R_root = W[root_i]

# posizioni nuove nello spazio del clump
nuove = {}
for i, f in enumerate(F):
    if i != root_i and f.name in new_head:
        nuove[i] = (np.append(new_head[f.name], 1.0) @ R_root)[:3]
for i, f in enumerate(F):          # i genitori precedono sempre i figli nella lista dei frame
    W = worlds(F)                   # catena ricalcolata con le posizioni già aggiornate
    if f.parent >= 0 and i in nuove:
        P = W[f.parent]
        loc = (nuove[i] - P[3, :3]) @ np.linalg.inv(P[:3, :3])
        f.position = dff.Vector(*map(float, loc))
W2 = worlds(F)
print("controllo posizioni:", max(np.abs(W2[i][3, :3] - p).max() for i, p in nuove.items()))
print("ossa non spostate (offset originali):", [f.name for i, f in enumerate(F) if i not in nuove and i != root_i and f.bone_data])

# vertici e normali nello spazio del frame dell'atomic
Wa_inv = np.linalg.inv(W2[a_i])
Rr = R_root[:3, :3]
verts = []
for v in geo.vertices:
    p = np.array([v.x, v.y, v.z, 1.0]) @ R_root @ Wa_inv
    verts.append(dff.Vector(*map(float, p[:3])))
geo.vertices = verts
if geo.normals:
    Ra_inv = np.linalg.inv(W2[a_i][:3, :3])
    nn = []
    for n in geo.normals:
        q = np.array([n.x, n.y, n.z]) @ Rr @ Ra_inv
        q = q / (np.linalg.norm(q) or 1.0)
        nn.append(dff.Vector(*map(float, q)))
    geo.normals = nn

# pesi: indice CJ -> ID osso -> indice del pedone
ped_bones = F[root_i].bone_data.bones
ped_order = [b.id for b in ped_bones]
id2idx = {bid: k for k, bid in enumerate(ped_order)}
skin = geo.extensions["skin"]
nuovi_idx = []
for idx4, w4 in zip(skin.vertex_bone_indices, skin.vertex_bone_weights):
    out = []
    for ix, w in zip(idx4, w4):
        bid = cj_order[ix]
        bid = ID_SOSTITUTI.get(bid, bid)
        out.append(id2idx[bid] if w > 0 else 0)
    nuovi_idx.append(tuple(out))
skin.vertex_bone_indices = nuovi_idx
skin.num_bones = len(ped_bones)
id2frame = {f.bone_data.header.id: i for i, f in enumerate(F) if f.bone_data}
skin.bone_matrices = [[list(r) for r in np.linalg.inv(W2[id2frame[bid]] @ Wa_inv)] for bid in ped_order]

geo.prelit_colors = []
for k in [k for k, v in geo.extensions.items() if isinstance(v, list)]:
    geo.extensions.pop(k)
c.geometry_list = [geo]
c.atomic_list[0].geometry = 0
d.clumps = [c]
d.write_file(out_path, 0x36003)
print("scritto", out_path)
