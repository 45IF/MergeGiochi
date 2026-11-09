"""MergeGiochi - converte animazioni LMSH1 (azioni sullo scheletro TT importate con TTGames-Workshop) in un IFP di SA
per i nostri modelli (scheletro dei pedoni SA con le articolazioni del minifig). Scritto da Claude.
Uso: python3 retarget_an4_ifp.py <blend con rig TT e azioni> <nome rig> <dff SA del personaggio> <ped.ifp di SA>
                                 <uscita.ifp> <nome blocco> <json: {"nome_sa": "azione TT", ...}>
Metodo: si confronta ogni osso TT con la sua posa di riposo (delta di rotazione nel mondo), si porta il delta nello
spazio di gioco di SA (fronte +Y, sinistra -X: specchio dell'asse X rispetto a LMSH) e lo si applica alla posa "in
piedi" del nostro modello (rotazioni di riposo del DFF, radice dritta come in idle_stance). Convenzione dei
quaternioni IFP verificata su ped.ifp: quaternione della matrice con assi (right, up, at) come colonne."""
import sys, json, math
sys.path.insert(0, '/home/claude/ref'); sys.path.insert(0, '/home/claude/mergegiochi/tools')
import bpy
from mathutils import Matrix, Quaternion, Vector
from DragonFF.gtaLib import dff
import ifp

blend, rig_name, dff_path, ped_ifp, out_ifp, blocco, mappa_json = sys.argv[-7:]
MAPPA = json.loads(open(mappa_json).read()) if mappa_json.endswith('.json') else json.loads(mappa_json)

# osso SA (nome nel DFF, senza spazi) -> osso TT che lo guida
GUIDA = {'Root': 'Character', 'Pelvis': 'Hips', 'Spine': 'Spine', 'Spine1': 'Spine2', 'Neck': 'Neck', 'Head': 'Head'}
for s, t in (('L', 'Left'), ('R', 'Right')):
    GUIDA.update({f'Bip01 {s} Clavicle': f'{t}Shoulder', f'{s} UpperArm': f'{t}Arm', f'{s} ForeArm': f'{t}ForeArm',
                  f'{s} Hand': f'{t}Hand', f'{s} Finger': f'{t}IndexFinger1', f'{s} Thigh': f'{t}UpLeg',
                  f'{s} Calf': f'{t}Leg', f'{s} Foot': f'{t}Foot', f'{s} Toe0': f'{t}ToeBase'})
T = Matrix(((-1, 0, 0), (0, 1, 0), (0, 0, 1)))   # LMSH (fronte +Y, sinistra +X) -> SA gioco (fronte +Y, sinistra -X)

# --- scheletro SA del personaggio: rotazioni locali di riposo e id delle ossa
d = dff.dff(); d.load_file(dff_path)
fl = d.clumps[0].frame_list
_, anims_sa = ifp.leggi(ped_ifp)
idle = [a for a in anims_sa if a['nome'].lower() == 'idle_stance'][0]
ID = {o['nome'].strip(): o['osso'] for o in idle['oggetti']}
ID['Root'] = ID.pop('Normal', 0)
x, y, z, w = [o for o in idle['oggetti'] if o['osso'] == 0][0]['frames'][0][0]
Q_DRITTO = Quaternion((w, x, y, z)).normalized().to_matrix()
nomi, genitore, loc_rip = [], [], []
for fr in fl:
    rm = fr.rotation_matrix
    nomi.append(fr.name.strip()); genitore.append(fr.parent)
    loc_rip.append(Matrix(((rm.right.x, rm.right.y, rm.right.z), (rm.up.x, rm.up.y, rm.up.z), (rm.at.x, rm.at.y, rm.at.z))).transposed())
ossa = [i for i, n in enumerate(nomi) if n in ID]
def fk(locali):
    W = {}
    for i in range(len(nomi)):
        W[i] = locali[i] if genitore[i] < 0 else W[genitore[i]] @ locali[i]
    return W
loc_ref = list(loc_rip); loc_ref[nomi.index('Root')] = Q_DRITTO
G_ref = fk(loc_ref)

# --- rig TT con le azioni
bpy.ops.wm.open_mainfile(filepath=blend)
rig = bpy.data.objects[rig_name]
if rig.animation_data is None: rig.animation_data_create()
rig.animation_data.action = None
bpy.context.view_layer.update()
# riposo dalle ossa (non dalla posa: senza azione la posa resta quella dell'ultima animazione valutata)
R_rip = {b.name: (rig.matrix_world @ b.bone.matrix_local).to_3x3().normalized() for b in rig.pose.bones}

uscita = []
for nome_sa, nome_azione in MAPPA.items():
    azione = bpy.data.actions[nome_azione]
    rig.animation_data.action = azione
    if hasattr(rig.animation_data, 'action_slot') and azione.slots:
        rig.animation_data.action_slot = azione.slots[0]
    f0, f1 = int(azione.frame_range[0]), int(azione.frame_range[1])
    tracce = {i: [] for i in ossa}
    for f in range(f0, f1 + 1):
        bpy.context.scene.frame_set(f)
        bpy.context.view_layer.update()
        G = {}
        for i in range(len(nomi)):
            n = nomi[i]
            tt = GUIDA.get(n)
            if tt and tt in rig.pose.bones:
                Rf = (rig.matrix_world @ rig.pose.bones[tt].matrix).to_3x3().normalized()
                D = T @ (Rf @ R_rip[tt].inverted()) @ T
                G[i] = D @ G_ref[i]
            elif genitore[i] >= 0 and genitore[i] in G:
                G[i] = G[genitore[i]] @ loc_ref[i]
            else:
                G[i] = G_ref[i]
        tempo = (f - f0) / 30.0
        for i in ossa:
            L = G[i] if genitore[i] < 0 or nomi[genitore[i]] not in nomi else (G[genitore[i]].inverted() @ G[i] if genitore[i] in G else G[i])
            q = L.to_quaternion()
            tr = (0.0, 0.0, 0.0) if nomi[i] == 'Root' else None
            tracce[i].append(((q.x, q.y, q.z, q.w), tempo, tr))
    ogg = [dict(nome=(' ' + nomi[i]) if not nomi[i].startswith('Bip') and nomi[i] not in ('Root',) else ('Normal' if nomi[i] == 'Root' else nomi[i]),
                osso=ID[nomi[i]], frames=tracce[i]) for i in ossa]
    uscita.append(dict(nome=nome_sa, oggetti=ogg))
    print('animazione', nome_sa, '<-', nome_azione, f1 - f0 + 1, 'fotogrammi')
ifp.scrivi(out_ifp, blocco, uscita)
print('scritto', out_ifp)
