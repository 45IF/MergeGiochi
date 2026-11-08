# Compito 01 — Codex: preparare l'ambiente (Fase 0)

Leggi prima `AGENTS.md`. Base: `C:\Users\utente\Desktop\Merge-Giochi`.

1. Clona https://github.com/45IF/MergeGiochi.git in `Merge-Giochi\MergeGiochi` (se non c'è già) e copia dentro i file preparati da Claude che trovi in `Merge-Giochi\_da-claude\` (AGENTS.md, .gitignore, docs\). Commit e push.
2. Copia `GTA-San-Andreas\Grand Theft Auto San Andreas\` in `Merge-Giochi\GTA-SA-mod\`. Non toccare l'originale.
3. Registra in `docs/stato.md` dimensione e SHA-256 di `gta-sa.exe` per identificarne la versione.
4. Downgrade a 1.0 US nella copia `GTA-SA-mod`: usa Jetpack Downgrader (vedi `docs/strumenti.md`). Chiedi conferma a Dario prima di eseguirlo.
5. Installa in `GTA-SA-mod`: Ultimate ASI Loader, Modloader, CLEO, Open Limit Adjuster. Avvia il gioco e verifica che parta (fai fare il test a Dario se serve).
6. Prepara la toolchain: Visual Studio 2022 (C++), plugin-sdk (DK22Pac) compilabile per GTA SA; compila un plugin ASI di prova che scrive un messaggio a schermo.
7. LMSH: in `strumenti\` metti QuickBMS e lo script per i giochi TT Games. Lancialo in modalità **lista** (senza estrarre) sui GAME*.DAT e salva l'elenco in `estratti\elenco_file.txt`. Poi cerca i file di Iron Man (es. cartella chars) ed estrai solo quelli in `estratti\ironman\`.
8. Test di fattibilità (il più importante): prova ad aprire il modello di Iron Man estratto in Noesis e/o Blender. Scrivi in `docs/stato.md` se si vedono mesh, texture e scheletro, e con quale strumento.
9. Aggiorna `docs/stato.md`, commit, push.

Fermati e scrivi in `docs/stato.md` se un passo fallisce dopo 2-3 tentativi.
