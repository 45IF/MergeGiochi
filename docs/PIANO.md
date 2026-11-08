# Piano — MergeGiochi (GTA SA × LEGO Marvel Super Heroes)

Obiettivo finale: tutti i personaggi di LMSH1 giocabili in San Andreas, con le loro abilità, tutti sbloccati dall'inizio. Asset: mix di originali SA + estratti da LMSH (nessun asset generato).

Strategia: **prima un prototipo verticale con un solo personaggio (Iron Man)**, poi un sistema di abilità riusabile, poi l'intero roster.

## Fase 0 — Ambiente (compito Codex 01)
- Copia di lavoro di SA in `GTA-SA-mod\`, originale intatto.
- L'eseguibile attuale è `gta-sa.exe` (~5,9 MB, versione Steam): va riportato alla **1.0 US** (`gta_sa.exe`) nella copia di lavoro.
- Installare: Ultimate ASI Loader, Modloader, CLEO, Open Limit Adjuster (o fastman92 LA); toolchain plugin-sdk (Visual Studio 2022 + plugin-sdk).
- LMSH: QuickBMS + script ttgames.bms; elencare il contenuto dei GAME*.DAT ed estrarre solo i file di Iron Man.
- Criterio di fine: SA 1.0 parte con ASI/CLEO/modloader attivi; esiste l'elenco dei file LMSH; i file di Iron Man sono in `estratti\`.

## Fase 1 — Prototipo Iron Man
1. Asset: importare il modello di Iron Man in Blender (TTGames-Workshop o Noesis: da verificare), adattarlo allo **scheletro dei pedoni di SA** (si tengono le animazioni di SA), esportare DFF/TXD con DragonFF / Magic.TXD.
2. Inserimento: nuova skin via modloader (ped dedicato), selezionabile dal giocatore.
3. Abilità (plugin ASI con plugin-sdk, in C++): volo (decollo, hover, spinta, atterraggio), raggio repulsore, armatura (più salute/danno ridotto).
4. Criterio di fine: si gioca come Iron Man e si vola in tutta la mappa senza crash.

## Fase 2 — Sistema
- Selettore personaggi (menu in gioco, cambio istantaneo come in LMSH).
- Abilità come moduli riusabili per categoria (volo, forza, gelo, fuoco, ragnatela, magnetismo, scudo, piccolo/grande, ecc.), così ogni personaggio è una combinazione di moduli e non codice su misura.
- Tutti sbloccati dall'inizio.

## Fase 3 — Roster
- Conversione in serie dei personaggi (script Blender per automatizzare), a gruppi, partendo da quelli con abilità già implementate.

## Rischi principali
- Scheletri e animazioni LEGO molto diversi da quelli di SA (mitigazione: adattare i modelli allo scheletro SA).
- Estrazione LMSH: strumenti parziali, formati non tutti documentati.
- Limiti del motore SA (numero di modelli/ped): limit adjuster.
