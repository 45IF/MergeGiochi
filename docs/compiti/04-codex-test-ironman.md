# Compito 04 — Codex: primo test di Iron Man in gioco

Leggi la sezione "Lavoro di Claude mentre Codex era fermo" in docs/stato.md.

1. Downgrade di `GTA-SA-mod` con **gtasa-open-downgrader** (release GitHub xxanqw/gtasa-open-downgrader, build `windows-offline`). Il codice sorgente riconosce il nostro MD5 (NewSteam R2 German). Dal programma installa anche **ASI Loader e ModLoader**. Verifica poi che esista `gta_sa.exe` 1.0 US e registra l'hash. Se servono permessi di amministratore, Dario approva.
2. Aggiungi CLEO 4.4 e Open Limit Adjuster (fonti in docs/strumenti.md).
3. `GTA-SA-mod\modloader\MergeGiochi\` è già pronto (ironman.dff/.txd + peds.ide con ID 3). Non modificarlo senza motivo.
4. Compila `MergeGiochi.asi` (src/volo + src/personaggi) e copialo in `GTA-SA-mod\scripts\`.
5. Avvia una partita: F9 → Iron Man? Prova camminata, corsa, salto, pugno, mira; poi doppio salto → volo (vedi compito 03). Annota tutto (anche crash e log di modloader) in docs/stato.md, screenshot in `estratti\test\`.
6. Commit + push (solo codice e documentazione).
