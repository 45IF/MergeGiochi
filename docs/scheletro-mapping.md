# Riconversione scheletro LMSH1 → pedone SA — ChatGPT-B v0.2, revisione Claude

Fonte modello: SOLO file originali LMSH1 (IRONMAN.CD / IRONMANM6.CD).

## Corrispondenza (funzionale; i nomi delle 63 ossa TT vanno letti dal modello importato)
| Pezzo LEGO | Osso SA | ID HAnim |
|---|---|---|
| Root | Root | 0 |
| Fianchi | Pelvis | 1 |
| Busto | Spine / Spine1 | 2 / 3 |
| Collo | Neck | 4 |
| Testa/casco | Head | 5 |
| Spalla DX/SX | R/L Clavicle | 21 / 31 |
| Braccio DX/SX | R/L UpperArm | 22 / 32 |
| (gomito virtuale) | R/L ForeArm | 23 / 33 |
| Mano DX/SX | R/L Hand | 24 / 34 |
| Dita/gancio | R/L Finger | 25 / 35 |
| Gamba SX/DX | L/R Thigh | 41 / 51 |
| (ginocchio virtuale) | L/R Calf | 42 / 52 |
| Piede | L/R Foot | 43 / 53 |
| (punta) | L/R Toe0 | 44 / 54 |
ID da verificare sul DFF di un ped vanilla importato con DragonFF.

## Strategia
- Importare un ped SA vanilla e tenerne scheletro, gerarchia, orientamenti e HAnim.
- Skinning rigido: ogni pezzo LEGO al 100% su un osso (casco→Head, busto→Spine1, gambe→Thigh, mani→Hand, braccia→UpperArm).
- Pivot SA spostati sui perni LEGO; ForeArm/Calf restano nella gerarchia come segmenti "virtuali" senza deformare pezzi rigidi.
- Scala uniforme fino a un'altezza vicina a CJ, senza allungare gli arti.
- Previsti clipping e piedi che scivolano: si correggono dopo il primo test.

## Checklist prima dell'export DFF (DragonFF)
1. Mesh e texture solo dal file LMSH1.  2. Gerarchia SA completa con HAnim/bone_id.  3. Pivot di spalle, anche, mani, piedi allineati.  4. Nessun vertice senza peso.  5. Scala/orientamento/trasformazioni applicate, posa di riposo corretta.  6. Test con idle, walk, run, jump, aim.  7. UV, materiali, trasparenze.  8. Export e re-import di prova.
Priorità: prima Iron Man che cammina senza deformazioni evidenti, poi volo e combattimento.
