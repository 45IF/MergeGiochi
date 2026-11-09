# Volo di Iron Man in un plugin ASI (plugin-sdk, SA 1.0 US) — ricerca Grok, revisione Claude

## Classi e approccio
- `CPed`/`CPlayerPed` (eredita `CPhysical`): scrivere ogni frame `m_vecMoveSpeed` calcolato da heading della camera + input; durante il volo disattivare la gravità (flag fisici di `CPhysical`, es. `bApplyGravity`). Nomi esatti dei flag da verificare negli header di plugin-sdk (`CPhysical.h`, `CPed.h`).
- Input: `CPad::GetPad(0)` (stato tasti/stick, salto).
- Task: il task manager rimette il pedone in caduta/ragdoll → durante il volo pulire/forzare i task (`CTaskManager`), evitando la risposta fisica (physical response). Da verificare empiricamente.
- Hook: logica nel loop di gioco (evento `Events::gameProcessEvent` di plugin-sdk o hook su `CPed::ProcessControl`).

## Collisioni
- Tenere le collisioni attive; hover con breve raycast verso il basso; `CWorld::ProcessLineOfSight` per evitare di entrare negli edifici; limitare la velocità massima. Crash tipici: velocità eccessive e task rimasti attivi.

## Animazioni
- IFP personalizzati per hover/volo caricati a parte (non sostituire `ped.ifp`); creazione in Blender con add-on per IFP; blend con `RpAnimBlendClumpGetAssociation` / `CAnimBlendAssociation`.

## Riferimenti
- Maxirp/IronManMod (GitHub): logica di volo/propulsori — SOLO studio del codice, mai i modelli.

## Trappole principali (Grok)
1. Il task manager riattiva caduta/ragdoll → serve forzatura continua.
2. Gravità + collisioni residue → velocità verticale instabile e crash a contatto col terreno.
