# Compito 02 — Codex: downgrade corretto + primo Iron Man in SA

## A. Downgrade (sblocca il compito 01)
- L'exe è **Steam NewSteam R2 tedesca** (MD5 D9CB35C898D3298CA904A63E10EE18D7), secondo Grok. Jetpack non la supporta.
- Prima scelta: **gtasa-open-downgrader** (github.com/xxanqw/gtasa-open-downgrader). Verifica nelle release notes/codice che l'hash D9CB35C8... sia supportato prima di usarlo.
- Alternativa: "Steam to version 1.0 downgrader" (Mars Icecream, PCGamingWiki file 842 / gtaforums topic 753764), che dichiara supporto a NewSteam r2 tedesca.
- Solo su GTA-SA-mod. Se serve l'esecuzione come amministratore, Dario approva. Dopo: verifica che esista gta_sa.exe 1.0 US (dimensione circa 14,4 MB) e registra l'hash.
- Poi completa i passi 5-6 del compito 01 (ASI Loader, Modloader, CLEO 4.4, Open Limit Adjuster; Build Tools C++ minimi + plugin-sdk; plugin ASI di prova).

## B. Texture
- Continua con IRONMANM6.CD / IRONMAN.CD: risolvi il caricamento DDS e i materiali (le risorse condivise vengono da altri archivi: estrai anche quelle, non solo i file con IRONMAN nel nome). Solo texture originali.

## C. Rig (vedi docs/scheletro-mapping.md)
- In Blender: importa un ped vanilla di SA con DragonFF, applica la strategia di skinning rigido sul modello LMSH1, esporta DFF/TXD in `estratti\ironman-sa\` (fuori dal repo).
- Script Blender riutilizzabili per la conversione → nel repo (`tools/`), perché serviranno per tutto il roster.
- Inserisci Iron Man come skin del giocatore nella copia GTA-SA-mod (via modloader) e fai uno screenshot di prova in gioco se possibile.

## D. Documenta tutto in docs/stato.md e fai commit + push (solo codice e documentazione).
