# Stato — 8 novembre 2026

## Compito 01 — ambiente: bloccato al passo 1
- Lette AGENTS.md, docs/compiti/01-codex-ambiente.md e docs/strumenti.md in _da-claude.
- La cartella MergeGiochi del repository non esiste.
- Git non disponibile nel PATH e non trovato nei percorsi comuni controllati (Program Files/Git, installazioni utente, GitHubDesktop, .codex e WindowsApps accessibile).
- Nessun installer o downgrader eseguito; nessuna cartella originale modificata.
- Questo stato è salvato temporaneamente in _da-claude/docs/stato.md: va trasferito nel repository dopo il clone.

## Rilevazione preliminare in sola lettura
- File originale: GTA-San-Andreas/Grand Theft Auto San Andreas/gta-sa.exe
- Dimensione: 5971456 byte.
- SHA-256: AA07FCDC8723A2831F77A35491DEA6CDD58F6733E3AC69A10DDF5CDF95034FE3
- Versione del gioco non identificata: dimensione e hash da soli non sono stati confrontati con una fonte verificata.

## Prossimo passo
Ottenere conferma esplicita di Dario prima di eseguire un installer Git, oppure usare un percorso Git già installato fornito da Dario. Quindi clonare il repository e verificare accesso/autenticazione e natura privata prima del push. Passi 2–8 non eseguiti, in attesa di risolvere il passo 1. Commit e push non eseguiti.

## Ripresa — 8 novembre 2026
- Ricevuta autorizzazione a installare Git con winget e a eseguire Jetpack sulla copia.
- Passo 2 completato: GTA-SA-mod creata, 415 file, 4.926.067.734 byte; robocopy concluso con codice 1 (successo), zero errori.
- Passo 3: SHA-256 della copia gta-sa.exe identico all'originale: AA07FCDC8723A2831F77A35491DEA6CDD58F6733E3AC69A10DDF5CDF95034FE3.
- LMSH: presenti GAME.DAT e GAME0.DAT–GAME10.DAT. Estrazione e verifica Iron Man ancora da eseguire.
- winget non accessibile con i permessi iniziali; installazione Git rilanciata con richiesta di autorizzazione di sistema, in corso.


- Git installato e clone di 45IF/MergeGiochi completato. Documenti di coordinamento copiati nel repository.
- QuickBMS e ttgames.bms scaricati dal sito autore; hash registrati. Elenco GAME.DAT in corso, già identificati CHARS/MINIFIGS/IRONMAN e risorse condivise.


- Passo 1: clone riuscito, commit locale 8f73d85. Identità locale dell'agente Codex <codex@localhost>; push non ancora eseguito, visibilità privata da verificare.
- GAME.DAT: modalità lista completata, 6084 file trovati, codice uscita 0; elenco in estratti/elenco_GAME.txt. Altri archivi ancora da elencare.
- Estrazione selettiva Iron Man avviata. TTGames-Workshop scaricato dalla fonte ufficiale https://github.com/Loomirr/TTGames-Workshop, importatore 0.5.31 da provare; Blender in installazione tramite winget.


- ATTENZIONE: API GitHub conferma private=false per 45IF/MergeGiochi. Push sospeso per rispettare il vincolo di progetto privato; nessun contenuto caricato.
- Estrazione GAME.DAT completata: 185 file selezionati dal filtro IRONMAN, disponibili in estratti/ironman. Prima invocazione non aveva creato la cartella; risolto creando la destinazione esplicitamente.
- Test Blender predisposto in estratti/ironman-cases.json; primo avvio non possibile perché installer ancora in corso.


## Esito verifica Iron Man — 8 novembre 2026
- Blender 5.2.3 installato tramite winget; TT Character Importer 0.5.31 dalla repository ufficiale.
- Eseguito check_blender.py su IRONMAN.CD estratto, con archivio LMSH originale in sola lettura e cache separata.
- Test completato: CHARACTER_PACKAGE_CHECK_PASSED 1. Modello composto da 16 mesh visibili, scheletro con 63 ossa.
- Risultati locali: estratti/verifica-ironman/LMSH1_IRONMAN.blend, LMSH1_IRONMAN.png, validation.json. Log: estratti/verifica-ironman.log.
- Immagine ispezionata: casco riconoscibile, corpo rosso senza dettagli del busto. Texture/materiali solo parzialmente corretti; NON soddisfatto il criterio completo mesh + scheletro + texture fedeli.
- Segnalazioni: DDS non caricate correttamente; attachment FACE_ARAGORN non trovato; traduzione shader incompleta. Animazioni e trasferimento dello scheletro a GTA non verificati.
- L'importatore ha recuperato automaticamente risorse condivise dagli altri archivi nella cache separata (278 file al controllo). Non basta estrarre soltanto file con IRONMAN nel nome.

## Blocco Jetpack e passi restanti
- Jetpack 2.3.0.1 con patch scaricato e decompresso. Hash documentato.
- Configurazione conservativa: niente reset salvataggi/impostazioni, niente modifiche registro o DirectPlay, niente downgrade forzato. Destinazione solo GTA-SA-mod.
- Avvio CLI fallito: primo metodo errore redirezione; Start-Process richiede elevazione, anche ripetuto tramite esecuzione autorizzata. Fermati i tentativi secondo AGENTS.md. Downgrade NON eseguito.
- MD5 attuale gta-sa.exe: D9CB35C898D3298CA904A63E10EE18D7; non coincide con gli hash supportati nel codice ufficiale Jetpack. Versione esatta ancora non identificata; l'indicazione Steam nel piano non è confermata.
- Passi 4-6 incompleti: downgrade, ASI/CLEO/Modloader/limit adjuster, Visual Studio/plugin-sdk e plugin di prova non completati. Non avviato il gioco.
- Passo 7 parziale: elenco GAME.DAT e 185 file selezionati estratti; elenco completo dei 12 archivi ancora da produrre.
- Passo 8 eseguito con esito PARZIALE: geometria e ossa presenti, texture da correggere.
- Push bloccato: repository remoto pubblico (private=false verificato via API). Occorre renderlo privato prima di caricare documentazione del progetto.
- Prossimo intervento: chiarire la versione dell'eseguibile GTA e risolvere elevazione Jetpack; diagnosticare caricamento DDS/importazione materiali sul modello già salvato. Nessun originale modificato.
