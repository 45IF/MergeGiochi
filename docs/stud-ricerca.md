# Stud LMSH1: identità dell'asset ancora da risolvere

## Esito del 9 novembre

Dario ha respinto il modello indicato nel compito 06: `CHARS/MISC/ITEMS/COIN_NXG.GSC`
è una moneta con il numero 10 in rilievo, non lo stud richiesto. L'export tecnicamente
riuscito e la sua visibilità in SA non dimostrano che sia l'asset giusto.
Il modello è stato ritirato e sono stati ripristinati i due file precedenti.

`tools/crea_stud_3d.py` conserva l'esperimento per diagnosi, ma richiede esplicitamente
`--diagnostic-coin` e non installa nulla. Non usarlo come pipeline dello stud definitivo.
L'esperimento contiene anche due atomic separati: un futuro pickup statico dovrà
riunire le parti in un unico atomic, mantenendo materiali e UV.

## Evidenze locali

- `COIN_NXG`: 24+72 vertici, 24+68 triangoli, texture 3; moneta con 10.
- `STUFF/THINGS_NXG.GSC`: `gold_pan_coin1..4`, `silver_pan_coin1..4`,
  `blue_pan_coin1..4`, `purple_pan_coin1..4` sono ciascuno quad di 4 vertici,
  2 triangoli, nel piano XZ. Materiali `CoinsNextGen_TTShader_*`.
- Le immagini collegate dall'importer sono texture 4 (oro/argento), 36 (blu),
  71 (viola). Non confondere la prima immagine esposta dall'importer con
  l'intero comportamento dello shader nativo.
- `STUFF/ATS/COIN.ATS` e `COIN_PAUSE.ATS` contengono `TexAdj 1,0,31` e
  `UntilTex = 31`: animazione di 32 indici texture. Non prova da sola che
  *tutti* i pickup nel gioco siano esclusivamente sprite.
- `coin_side1..10` sono altri quad, materiali `TrueBar*`, texture 103:
  non sono il bordo 3D del pickup, ma risorse della barra.
- `THINGS / SwapCol_1x1_plateround` e `ClimbStud`: 74 vertici, 72 triangoli;
  pezzi circolari originali. Nessun collegamento al pickup da raccogliere
  dimostrato. Render con discontinuità: non approvati per sostituzione.
- `CHARS/MISC/MAGNETOSTUDS`: tre pezzi `p6141_`, ciascuno composto da
  42+32 vertici e 44+28 triangoli, shader magnetico. Non è dimostrato che
  siano il modello dei soldi. Non installati, non ricolorati per sostituirlo.

File di verifica fuori dal repo: `estratti/verifica-stud/`, con blend originali
importati, `things/candidati-mesh.json`, `candidati.json`, render e log.
`in-gioco-vicino.png` documenta il candidato COIN **respinto**, non un test superato.

## Stato installato e prossimo passo

Ripristinati a gioco chiuso, hash MD5 uguali ai backup:

- Money.dff: `7A54DB7D47801FE0577022E09E86D2A5`.
- dyn_cash.txd: `3CD1A6936DA3068D028AF194D41E7547`.

Il pickup torna quindi alla precedente versione piatta di Claude, con il HUD
preservato. Nessuna modifica di Codex a MergeGiochi.asi, ModuloAbilita.cpp,
mgscudo.*, ironman.* o alle definizioni peds.ide durante questo compito.

Per proseguire serve identificare e verificare visivamente l'asset corretto
e ricostruire i riferimenti delle texture animate/shader a partire dai file
originali. Non estrudere i quad, non modellare uno stud sostitutivo e non
scambiare un pezzo simile per il pickup originale. Le varianti B e C restano
non avviate: A non ha superato la verifica di identità e la prova completa.
