# Compito 06 — Codex: stud LEGO in 3D al posto dei soldi (come in LMSH1)

Richiesta di Dario (9/11): i mazzi di soldi di San Andreas devono diventare gli **stud di LEGO Marvel Super Heroes 1**,
con il **modello 3D originale** del gioco, non un'immagine piatta.

## Situazione attuale (fatta da Claude, funziona)
- `GTA-SA-mod\modloader\MergeGiochi\Money.dff` + `dyn_cash.txd`: sostituiscono il pickup dei soldi (modello 1212 "Money",
  txd "dyn_cash", definito in `data\maps\generic\dynamic.ide`). Ora è un **quad piatto** a doppia faccia con la texture
  dello stud (THINGS_NXG / gold_pan_coin1 + texture 103). Si vede e si raccoglie, ma è piatto: va sostituito.
- `src/hud/ContatoreStud.cpp`: contatore del HUD (icona stud + numero oro + barra True Believer da 10 stud).
  **La texture `lmsh_stud` dentro `dyn_cash.txd` serve al HUD: deve restare** (con lo stesso nome) anche nel txd nuovo.
- Tasto di prova **F6**: crea un pickup soldi da 500 a 2 m davanti al giocatore. Usalo per i test.
- Leggi `tools/crea_stud_pickup.py` per vedere come è stato fatto il quad (Blender + DragonFF, export DFF 3.6.0.3).
  Attenzione al bug trovato: il quad importato sta nel piano XZ, non XY (prima veniva una linea invisibile).

## Cosa fare
### A. Modello 3D dello stud (priorità)
1. File originali già estratti: `estratti\lmsh\CHARS\MISC\ITEMS\COIN_NXG.GSC` (+ `.NXG_TEXTURES`, `.PC_SHADERS`).
   Importato con TTGames-Workshop contiene 2 mesh: `coin / 0` (24 vertici, faccia superiore, materiale `TPage_LEGO_Hard`)
   e `coin / 1` (72 vertici, bordo/lato, materiale `LEGO_Hard:Shader_p92947_Side`), texture `COIN_NXG / texture 3`.
   Dimensioni originali circa 0.074 × 0.074 × 0.012.
2. Converti in `Money.dff` + `dyn_cash.txd` per SA (Blender + DragonFF, come `crea_stud_pickup.py`; scrivi lo script
   nuovo in `tools/crea_stud_3d.py`). Regole:
   - **solo geometria e texture originali di LMSH1**, niente modelli rifatti a mano o presi altrove;
   - scala: diametro circa **0.35 m** in gioco (deve vedersi bene da lontano, come in LMSH1 rispetto alla minifig);
   - centrato sull'origine; in piedi (la faccia con il "bottoncino" rivolta verso l'orizzontale), perché il gioco fa
     girare i pickup sull'asse Z: deve sembrare la moneta che gira in LMSH1;
   - colore oro come lo stud d'oro di LMSH1 (se il materiale originale prende il colore dallo shader e non dalla texture,
     metti il colore nel materiale DFF; il flag "modulate material color" è già acceso negli export di DragonFF);
   - normali corrette (il gioco illumina i pickup), niente facce mancanti;
   - nel txd tieni anche `lmsh_stud` (texture del HUD).
3. Prova in gioco: avvia `gta_sa.exe` dalla cartella `GTA-SA-mod`, premi F6, guarda lo stud da vicino e da 20-30 m,
   raccoglilo (il contatore deve salire di 500). Salva uno screenshot in `estratti\verifica-stud\`.

### B. Colore secondo il valore (come in LMSH1) — solo se A funziona
In LMSH1: argento = 10, oro = 100, blu = 1000, viola = 10000.
- Aggiungi 3 modelli (stud argento, blu, viola: stessa geometria, cambia solo il colore del materiale) nella sezione
  `objs` di `modloader\MergeGiochi\peds.ide` con ID liberi (19301, 19302, 19303; il 19300 è `mgscudo`) e un txd loro.
- Codice: in un file NUOVO `src/hud/StudPickup.cpp` scegli il modello in base al valore quando nasce un pickup soldi
  (`CPickups::GenerateNewOne` 0x456F20, tipo 8 = PICKUP_MONEY; il valore è il parametro "ammo"). Il valore resta
  quello di SA: cambia solo il modello disegnato.
  Avvertenza MinGW (Claude compila con MinGW i686): le funzioni del gioco che prendono `CVector` per valore vanno chiamate
  con puntatori a funzione "grezzi" con tre float, vedi `StudDiProva()` in `ContatoreStud.cpp`.

### C. Facoltativo, dopo A e B
- Un pedone ucciso che lascia soldi: invece di un solo pickup, più stud sparsi intorno (come gli stud che "esplodono"
  in LMSH1), con la stessa somma totale.

## Regole di coordinamento (importanti)
- **Non sovrascrivere `modloader\MergeGiochi\MergeGiochi.asi`**: lo compila e installa Claude, che intanto lavora sullo
  scudo di Captain America (`src/abilita/ModuloAbilita.cpp`, `mgscudo.*`). Non toccare quei file.
- Tu tocchi solo: `Money.dff`, `dyn_cash.txd`, i nuovi modelli degli stud, le righe nuove in `peds.ide`,
  `tools/crea_stud_3d.py`, `src/hud/StudPickup.cpp`, `docs/`.
- Prima di iniziare: in `MergeGiochi\` esegui `git pull ..\strumenti\claude_<hash>.bundle main` (il bundle più recente
  in `strumenti\`), poi `git push`: contiene i sorgenti aggiornati di Claude che lui non riesce a pushare.
- Per i test di A basta l'ASI già installato (F6). Per B scrivi il codice, fai commit e push, e scrivi in `docs/stato.md`
  "StudPickup pronto da compilare": lo integra Claude.
- Il gioco va chiuso prima di sovrascrivere file in `modloader`.
- Alla fine aggiorna `docs/stato.md` (cosa fatto, cosa non va, screenshot) e fai commit + push (niente asset nel repo).
