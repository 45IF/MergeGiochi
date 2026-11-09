# Compito 05 — Codex: commit, limit adjuster, estrazione dei 4 personaggi

Leggi prima la sezione "Notte 8→9 novembre" in docs/stato.md (ci sono 7 lezioni importanti).

## A. Commit e push
- Nel repo ci sono file nuovi scritti da Claude: `tools/converti_minifig.py`, `tools/scrivi_dff_ped.py`, `tools/compila_mingw.sh`, `tools/config/ironman_m6.json`, `src/volo/ModuloVolo.cpp`, `src/personaggi/CambioPersonaggio.cpp`, `docs/*`. Commit + push (solo codice e documentazione: controlla git status, niente asset).

## B. Limit adjuster per gli slot dei pedoni
- Il gioco crasha al caricamento con 3 pedoni aggiunti in peds.ide. Serve aumentare il limite dei modelli dei pedoni (ped model infos).
- Verifica quale limit adjuster lo supporta davvero per SA 1.0 US (Open Limit Adjuster `III.VC.SA.LimitAdjuster`, oppure fastman92 Limit Adjuster) e installalo in GTA-SA-mod. Annota in docs/strumenti.md.
- Prova: aggiungi temporaneamente 8 righe di prova in una COPIA del peds.ide di modloader e verifica che il gioco si carichi; poi rimuovile. Non toccare `modloader\MergeGiochi\ironman.*`.

## C. Estrazione e import in Blender (come per Iron Man M6)
Per ciascuno: estrai con QuickBMS tutti i file della cartella del personaggio + le risorse condivise che servono (texture in SUPER_CHARACTERS, ecc.), poi importa con TTGames-Workshop e salva un .blend con le texture impacchettate, come `estratti\verifica-ironman-m6\LMSH1_IRONMANM6.blend`.
| Personaggio | File | Uscita |
|---|---|---|
| Spider-Man | `CHARS\MINIFIGS\SPIDERMAN\SPIDERMAN.CD` | `estratti\verifica-spiderman\LMSH1_SPIDERMAN.blend` |
| Captain America | `CHARS\MINIFIGS\CAPTAINAMERICA\CAPTAINAMERICA.CD` | `estratti\verifica-captainamerica\LMSH1_CAPTAINAMERICA.blend` |
| Wolverine | `CHARS\MINIFIGS\WOLVERINE\WOLVERINE.CD` | `estratti\verifica-wolverine\LMSH1_WOLVERINE.blend` |
| Hulk (big-fig) | `CHARS\BIGFIGS\HULK\HULK.CD` | `estratti\verifica-hulk\LMSH1_HULK.blend` |
- Per ognuno salva anche un render PNG di controllo e annota in docs/stato.md: numero di mesh, ossa, texture caricate, cosa manca.
- Estrai anche (solo estrazione, nessun import) i file `.TXT` e `.AS` delle cartelle dei 4 personaggi: servono a Claude per le abilità.
- Se TTGames-Workshop sa importare le animazioni `.AN4` (`*_AN4_NXG.PAK`), prova a importare `CHARS\MINIFIGS\IRONMAN\IRONMAN_FLY_AN4_NXG.PAK` sull'armatura di Iron Man M6 e scrivi l'esito in stato.md (serve per l'animazione di volo).

## D. Fine
Aggiorna docs/stato.md, commit + push. Se qualcosa non va dopo 2-3 tentativi, scrivilo e passa al punto successivo.
