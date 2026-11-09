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

## Decisioni e risultati aggiornati — 8 novembre 2026
- Autorizzato esplicitamente il push pubblico di solo codice/documentazione. Asset LMSH1 esclusivamente originali; nessuna sostituzione o texture rifatta. Regole aggiornate in AGENTS.md.
- Risolta la mancanza dei dettagli del busto usando IRONMANM6.CD, già tra i 185 file estratti. IRONMAN.CD dichiara invece LEGO_DARKRED per fronte e retro: il primo test non usava la definizione dell'armatura Mark 6.
- Mark 6: importazione e render completati, CHARACTER_PACKAGE_CHECK_PASSED 1; 20 mesh visibili e 63 ossa. File locali: estratti/verifica-ironman-m6/LMSH1_IRONMANM6.blend e .png, validation.json.
- Texture originali: BODY_IRONMANM6_FRONT, BODY_IRONMANM6_BACK, LEGRIGHT_IRONMANM6 nelle cartelle condivise CHARS/MINIFIGS/SUPER_CHARACTERS. Il CD dichiara MATERIAL_REMAP_IRONMANM6 con MAT_IRONMANM6GLOW_FRONT, MAT_IRONMANM6GOLD_BACK e MAT_IRONMANM6GOLD_LEGS.
- Documentazione consultata: TTGames-Workshop/docs/ISSUE_1_MINIFIGS.md e DIAGNOSTIC_TOOLS.md. L'importatore gestisce già il layer additivo comune del reattore; ricostruzione completa di shader metallici/BRDF e luminosità nativa ancora limitata. Non trovato un interruttore generico decal necessario per questo caso.
- Audit locale estratti/audit-ironman-m6/audit.json: mesh del busto con Source uv 0 e Source uv 1; tutti i vertici del busto hanno gruppi/pesi. Ossa, mesh e texture originali presenti; non ancora testata animazione/deformazione nel gioco. Il render mostra armatura stampata, dettagli gambe e reattore. Nessun asset esterno o generato.
- Noesis non necessario per questo test: importazione Mark 6 riuscita in Blender. Nessuna equivalenza perfetta dello shader di gioco dichiarata.

## Jetpack: nuovo tentativo amministratore
- Versione tool 2.3.0.1. Tentato Start-Process -Verb RunAs sulla sola GTA-SA-mod; chiamata ha restituito PID 1788, ma nessun log catturato. Successivo wrapper elevato per acquisire output restituisce PID 19924 ma non produce il log previsto. Non è possibile confermare la versione rilevata dal programma: nessun messaggio runtime acquisito.
- Messaggio esatto dei tentativi precedenti: "This command cannot be run due to the error: Per eseguire l'operazione richiesta è necessaria l'esecuzione con privilegi elevati."
- Dal codice ufficiale gli hash dell'eseguibile attuale non risultano supportati; questa è un'analisi statica, NON un messaggio osservato di Jetpack. Versione gioco resta non identificata.
- SHA-256 gta-sa.exe ricontrollato e invariato: AA07FCDC8723A2831F77A35491DEA6CDD58F6733E3AC69A10DDF5CDF95034FE3. Non forzato alcun downgrade. Punto affidato a Claude/Grok come richiesto.

## Toolchain e prova ASI
- Installazione Microsoft.VisualStudio.2022.BuildTools 17.14.44 con workload VCTools tentata via winget. Esito: "Installer failed with exit code: 2147942512" (0x80070070, spazio insufficiente). Spazio libero al controllo 3.616.550.912 byte. Nessun file utente cancellato per fare spazio.
- plugin-sdk ufficiale scaricato in strumenti/plugin-sdk, commit 15f15b60bbf74c106e1b496ff92c98764abf4605.
- Preparato src/ProvaASI.cpp: messaggio al caricamento della partita, guardia per SA 1.0 US. NON compilato né testato in gioco: compilatore non installato completamente. Il sorgente non va considerato validato.
- Occorre liberare spazio o indicare un disco per completare la toolchain; nessun ASI binario dichiarato pronto.

- Verifica finale toolchain: vswhere restituisce [] (nessuna istanza Visual Studio installata). Hash installer registrato in docs/strumenti.md.
- Push avviato dopo controllo git status, file tracciati e cronologia in uscita: solo .md, .gitignore, .cpp e .py. Al momento attende git-credential-manager: richiesta a Dario di completare accesso GitHub. Nessun successo remoto ancora confermato.


- Autenticazione risolta: push confermato dal remoto, main aggiornato da 0cad3f7 a 68c113c. Nessun asset/binario incluso. Aggiornamenti successivi documentano provenienza e blocchi.


## Ripresa dopo spazio liberato
- Dario conferma spazio liberato e accesso GitHub. C: dispone di 32.366.632.960 byte liberi al controllo.
- Riavviata installazione Visual Studio Build Tools 2022 con workload C++. Installazione in corso.
- Preparati premake5.lua e scripts/compila.ps1 per compilazione x86 con plugin-sdk, output esclusivamente in build/ ignorata da Git.


## Lavoro di Claude mentre Codex era fermo (8 novembre, 22:30–23:15)
- **Iron Man convertito per SA** con `tools/converti_minifig.py` + `tools/config/ironman_m6.json` (Blender 5.2 + DragonFF, eseguito nel cloud di Claude):
  - mesh, pesi e texture SOLO da `LMSH1_IRONMANM6.blend` (IRONMANM6.CD originale); casco = Helmet/Visor originali con colori per vertice tradotti in colori materiale.
  - scheletro: quello dei pedoni SA preso da `player.img/torso.dff` (37 ossa, bone_id originali), articolazioni spostate sui perni del minifig; pesi TT rinominati verso le ossa SA (nessun peso inventato).
  - altezza 1,80 m; piedi a z = -1,0. Il bacino è abbassato di 0,53 m rispetto alla radice: verificato su `anim/ped.ifp` che le animazioni traslano solo l'osso radice ("Root"/"Normal"), il bacino solo in 8 animazioni su 294 (lì Iron Man potrebbe sollevarsi: da controllare).
  - verifiche: reimport del DFF, render da fermo e in posa (braccia, gambe, testa seguono lo scheletro). File: `estratti\ironman-sa\` (dff, txd, png di controllo).
- **Pronto in Modloader**: `GTA-SA-mod\modloader\MergeGiochi\` con ironman.dff, ironman.txd e peds.ide (copia dell'originale + riga ID 3, verificato libero nel peds.ide del gioco):
  `3, ironman, ironman, CIVMALE, STAT_STD_MISSION, player, 1FFF, 0, null, 9,9, PED_TYPE_SPC, VOICE_GEN_NOVOICE, VOICE_GEN_NOVOICE`
- **Codice nuovo** (sintassi controllata con gli header di plugin-sdk, mai compilato con MSVC né provato):
  - `src/volo/ModuloVolo.cpp` (compito 03).
  - `src/personaggi/CambioPersonaggio.cpp`: F9 trasforma il giocatore in Iron Man (modello cercato per nome "ironman") e viceversa (CJ + RebuildPlayer).
- **Downgrade verificato nel codice sorgente**: gtasa-open-downgrader (`downgrader/main.py`, riga 698) riconosce l'MD5 D9CB35C8... come "NewSteam R2 (German)" e lo porta a 1.0 US Hoodlum. Le patch stanno su files.xxanqw.me (non raggiungibile dal cloud di Claude): usare la build **offline** per Windows dalla release GitHub. Lo stesso programma installa anche ASI Loader e ModLoader.
- **Roster dai file**: `docs/roster-dai-file.md` (554 file .CD: 192 minifig principali, big-fig, varianti, comparse).

## Notte 8→9 novembre: Iron Man FUNZIONA in gioco (Claude + test di Dario)
- Downgrade fatto da Dario con gtasa-open-downgrader offline (gta_sa.exe 1.0 US Hoodlum), con ASI Loader, ModLoader, SilentPatch.
- **Plugin compilato da Claude con MinGW-w64** in cloud (`tools/compila_mingw.sh`), installato come `GTA-SA-mod\modloader\MergeGiochi\MergeGiochi.asi`. Log diagnostico: `modloader\MergeGiochi\MergeGiochi.log`.
- Tasti attuali: **F9** = Iron Man ↔ CJ; **F10** = ciclo diagnostico (male01 → mgtest → CJ); volo: Maiusc due volte, Maiusc tenuto sale, C scende/atterra, Spazio boost, F8 attiva/disattiva il volo. Sul ThinkPad F7 e F11 NON arrivano al gioco: usare altri tasti.
- **Lezioni (importanti per tutti i personaggi):**
  1. Cambio modello del giocatore: `DeleteRwObject()` + `m_nModelIndex = -1` + `SetModelIndex(id)` + ripristino `m_nAnimGroup` (come examples/HandSignals di plugin-sdk). Senza → crash 0x4D2C3A.
  2. Lo scheletro dei vestiti di CJ (player.img/torso.dff) fa crashare il gioco (0x4D2C3A) se usato da un altro modello come giocatore. **Si usa lo scheletro di un pedone normale: `male01`** (32 ossa, mesh agganciata al Pelvis). Estratto da gta3.img con `strumenti\ESTRAI_PEDONI.bat` in `estratti\sa\`.
  3. Convenzione skin RW verificata (errore 0): `skin_k = inv(W_k · inv(W_frameAtomic))`; vertici nello spazio del frame dell'atomic.
  4. DragonFF: riesportando un'armatura azzera la rotazione dell'osso radice; la sua libreria condivide la lista dei clump fra istanze (caricare il file "base" per ultimo). Per questo il DFF finale si scrive con `tools/scrivi_dff_ped.py` partendo dal DFF originale di male01.
  5. **Limite slot pedoni**: con 3 righe aggiunte a peds.ide il gioco crasha al caricamento (0x5B74AC → 0x4C67BB). Serve un limit adjuster per avere più personaggi.
  6. Volo: `FlushImmediately(true)` (mai `false`, altrimenti crash 0x523E6B all'atterraggio).
  7. Il bridge file di Claude a volte ha copiato versioni vecchie: ogni file installato va verificato con l'impronta MD5.
- Pipeline personaggio: `converti_minifig.py config.json` (Blender+DragonFF) → `scrivi_dff_ped.py male01.dff <blend> <dff> torso.dff <uscita>` → TXD da DragonFF.
- Studio LMSH: il volo di Iron Man è definito in `IRONMAN_FLY.AS` con stati hover / hover_up / hover_down / hover_left / hover_right / fly / flyleft / flyright / fly_sprint(±left/right) / fly_turn_180 / fly_slowdown / fly_land / fly_intro / glide; animazioni in `IRONMAN_FLY_AN4_NXG.PAK`.

## Prossimo passo
Vedi `docs/compiti/05-codex-quattro-personaggi.md` (Codex) — Claude lavora su volo/caduta libera e conversione.

## Compito 05 — 9 novembre, avanzamento Codex
- Commit 96a91d2 e push completati: pipeline e codice di Claude, solo sorgenti/documentazione.
- Open Limit Adjuster 2.0.6 ufficiale installato nella radice GTA-SA-mod. PedModels=unlimited, altre voci presenti impostate a 0, Log=1. Il log riconosce SA 1.0 US HOODLUM e Mod Loader.
- Prova temporanea: 8 definizioni con ID liberi 2,5,6,8,42,65,74,86 e copie di male01. Osservato caricamento, log SCM inizializzato; arrivo in partita NON confermato perché finestra ripetutamente minimizzata. Test quindi parziale.
- Sessione di prova chiusa; peds.ide ripristinato con MD5 916CA09EEA05C6E9F59C7C1176FA0548; TestPedModels spostata fuori dal gioco in estratti/test-pedmodels.
- ironman.dff MD5 D7435804AA03F5FEEEB84471F222471F e ironman.txd MD5 255E574186E5AFCAAD6934281DAF089A invariati.
- QuickBMS: 59 risorse dei quattro personaggi estratte da GAME.DAT; estrazione dagli altri archivi e import Blender in corso.


### AN4 volo — test completato
- Script tools/verifica_volo_an4.py, uscita estratti/verifica-volo-an4/IRONMANM6_volo.blend ed esito.json. Nessun file Iron Man originale o installato modificato.
- 42 voci di catalogo fanno riferimento a IRONMAN_FLY_AN4_NXG.PAK; provate 7 voci: Hover_Up, Hover, Hover_Down (144 frame), FlyLeft, Fly, FlyRight (156 frame), fly_land (18 frame). Tutte importate sul rig nativo a 63 ossa; matrici finite ai frame iniziale, centrale e finale.
- Avviso comune: nessun attore univoco per gli attachment FACE_TONYSTARK e IronManHelmet. Clip del corpo importate, animazione indipendente degli attachment non risolta. Non è un test di retargeting SA né di volo in gioco.


### Prime tre importazioni verificate
| Personaggio | Mesh visibili | Ossa | Texture caricate e impacchettate | Controllo visivo |
|---|---:|---:|---:|---|
| Spider-Man | 12 | 63 | 5/5 | Maschera e busto visibili; materiali approssimano lo shader nativo |
| Captain America | 16 | 63 | 6/6 | Costume visibile; scudo assente nel render, da verificare come risorsa equipaggiata |
| Wolverine | 18 | 63 | 7/7 | Costume e capelli visibili; artigli assenti nella posa importata |
Le verifiche texture riaprono i .blend e controllano dati immagine e packed_file. Nessuna immagine senza pixel nei tre file. Report locali nelle cartelle estratti/audit-<personaggio>.


### Chiusura verifiche locali compito 05
- Hulk: 13 mesh visibili, 70 ossa, 2/2 texture caricate e impacchettate; render verificato in estratti/verifica-hulk. Il bigfig mantiene il rig nativo: non è ancora un test del rig SA.
- Estrazione dei quattro personaggi completata su GAME.DAT e GAME0..10.DAT: confronto con l'indice originale, 120 risorse attese, zero mancanti. Report estratti/quattro-personaggi/indici/copertura.json. Rimane parziale la prova degli 8 slot pedoni in partita descritta sopra.

## Compito 06 — stud 3D, 9 novembre (Codex)
- Bundle claude_9a79c41.bundle integrato con fast-forward e push completato fino a 9a79c41. Modifiche locali di Claude conservate; stash di sicurezza mantenuto.
- COIN_NXG.GSC originale importato in estratti/verifica-stud/originale: due parti, 24+72 vertici, 24+68 facce; una texture 128x128 impacchettata e verificata (65536 componenti pixel).
- Nuovo tools/crea_stud_3d.py: geometria/UV originali, diametro 0,35 m, rotazione di 90 gradi attorno a X e centratura. Colore oro nel materiale RW; approssimazione dello shader TT, nessuna texture ridisegnata.
- Prima esportazione riuscita in estratti/verifica-stud/conversione. La texture lmsh_stud del HUD è copiata come texture nativa, senza ricompressione: SHA256 f49af1ae1260d235ecbfd9089971688a808564c3dbc77bb8ac97f80565d8513d invariato.
- Risolto caricamento pigro delle immagini Blender: has_data inizialmente falso non indica pixel mancanti; audit separato conferma la DDS originale. Verifica del DFF e prova in gioco in corso; B rimane subordinato alla prova A.
- DFF riletto con DragonFF: 96 vertici, 92 triangoli, 96 normali finite e normalizzate; dimensioni 0,35 x 0,071826 x 0,35 m. Render di controllo: estratti/verifica-stud/conversione/reimport.png. Nota: Blender salda alcuni vertici in reimport, il DFF mantiene i 96 originali.
- Installati Money.dff (5162 byte, SHA256 75E0BB1870273FFA367E8B394E951C92A9D9A4CFB6A21CA01742D4EBB6A435EF) e dyn_cash.txd (131368 byte, SHA256 1EC0C95C546D609D10F1C7D34C56CD75ABBB8CDC367AAFB157327FBF8206B97A) a gioco chiuso. Backup in estratti/verifica-stud/backup.
- In partita osservati personaggio e HUD funzionanti, valore iniziale 350. Prova F6 ancora da completare: prima sessione usata contemporaneamente da Dario, poi chiusa. Dario ha ora lasciato i comandi a Codex.
- CORREZIONE dopo riscontro diretto di Dario: COIN_NXG è una moneta con il numero 10, NON lo stud LMSH1 richiesto. La riuscita tecnica della conversione non valida l'identità dell'asset. Prova A respinta; B non avviato. A gioco chiuso ripristinati Money.dff e dyn_cash.txd dal backup precedente. Ricerca dello stud autentico in STUFF/THINGS e risorse correlate in corso.
- Ripristino verificato: Money.dff MD5 7A54DB7D47801FE0577022E09E86D2A5; dyn_cash.txd MD5 3CD1A6936DA3068D028AF194D41E7547, entrambi identici ai backup. Rimane installato il precedente pickup piatto di Claude.
- Ricerca documentata in docs/stud-ricerca.md: THINGS gold/silver/blue/purple_pan_coin sono quad; COIN.ATS e COIN_PAUSE.ATS animano indici texture 0..31. coin_side appartiene a TrueBar. Ispezionati anche i pezzi round e MAGNETOSTUDS: nessuno verificato come pickup corretto, nessuno installato.
- tools/crea_stud_3d.py ora rifiuta l'uso normale e conserva l'esperimento solo con --diagnostic-coin, per evitare di reinstallare il candidato respinto. Compito 06 NON completato: identità asset irrisolta, raccolta +500 e distanza 20–30 m non validate. B/C non avviati. Non indicare StudPickup pronto da compilare.
