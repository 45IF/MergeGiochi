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

