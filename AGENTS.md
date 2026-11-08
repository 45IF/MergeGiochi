# MergeGiochi — istruzioni per gli agenti (Codex / ChatGPT-A)

Progetto PRIVATO di Dario: mod di GTA San Andreas (PC) con i personaggi e le abilità di LEGO Marvel Super Heroes (2013). Non va mai pubblicato.

## Squadra
- Claude: coordinatore (piano, architettura, revisione, integrazione). Scrive i compiti in `docs/compiti/`.
- ChatGPT-A / Codex (tu): implementazione sul PC Windows: build, script, estrazione, installazione strumenti, test, commit e push.
- ChatGPT-B: specifiche di asset e gameplay (via web, non tocca i file).
- Grok: ricerca strumenti e revisione critica.

## Cartelle sul PC (base: C:\Users\utente\Desktop\Merge-Giochi)
- `GTA-San-Andreas\Grand Theft Auto San Andreas\`  → ORIGINALE. SOLO LETTURA. Non modificarla mai: è il backup.
- `LEGO-MARVEL-Super-Heroes\LEGO Marvel Super Heroes\` → ORIGINALE. SOLO LETTURA.
- `GTA-SA-mod\`   → copia di lavoro di San Andreas (downgrade 1.0 US + mod). Si fanno qui tutte le modifiche.
- `estratti\`     → file estratti da LMSH (fuori dal repo).
- `strumenti\`    → QuickBMS, downgrader, Blender add-on, ecc.
- `MergeGiochi\`  → questo repository.

## Regole
1. Mai modificare le due cartelle originali.
2. Mai committare asset o binari dei giochi (vedi .gitignore): nel repo vanno solo codice, script, configurazioni, documentazione, script di conversione.
3. Scarica strumenti solo da fonti ufficiali (release GitHub del progetto o sito dell'autore). Annota in `docs/strumenti.md` nome, versione, URL e hash SHA-256 di ogni cosa scaricata. Prima di eseguire un installer o un downgrader, chiedi conferma a Dario.
4. Dopo ogni compito aggiorna `docs/stato.md` (cosa fatto, cosa non va, prossimo passo) e fai commit con messaggio in italiano.
5. Se qualcosa non funziona dopo 2-3 tentativi, fermati e scrivi il problema in `docs/stato.md` invece di andare avanti a caso.
