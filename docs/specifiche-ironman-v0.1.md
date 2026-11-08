# Specifiche Iron Man v0.1 (ChatGPT-B, revisione Claude)

## Abilità da prototipo (in quest'ordine)
1. **Flight** — decollo con doppio salto (GROUND → TAKEOFF → HOVER); hover a quota stabile; WASD/stick sinistro per muoversi, camera con mouse/stick destro; Spazio/A sale, Ctrl/B scende; Shift/RT boost. Velocità iniziali: 8 m/s hover, 20 crociera, 35 boost. Atterraggio controllato vicino al suolo, poi si torna alla fisica normale del pedone. Gestire collisioni e cadute.
2. **Repulsor** — proiettile energetico, danno 20, cooldown 0,35 s.
3. **Armor** — salute 100 + armatura 200, resistenza agli urti.
4. Dopo: Unibeam (carica 1,5 s, danno 100, cooldown 6 s), missili (esplosione locale, cooldown 3 s).
I valori sono di bilanciamento, non statistiche originali di LMSH1.

## Interazioni ambientali
SA non ha gli oggetti LEGO dorati: più avanti, oggetti metallici selezionati marcati "HeatTarget" distruttibili col laser. Hacking: terminali dedicati. Non per il prototipo.

## Animazioni
- Riuso da SA: camminata, corsa, salto, atterraggio, pugni, mira.
- Nuove: hover, volo orizzontale, decollo (all'inizio bastano pose statiche sullo scheletro SA).

## Checklist modello (DFF/TXD)
Altezza simile a CJ; scheletro completo dei pedoni SA con ID HAnim corretti; pesi dello skinning; UV, texture, alpha, normali; punti di emissione su mani e petto; controllare le deformazioni in corsa e salto.

## Tassonomia abilità (ordine di implementazione)
1 proiettili/energia · 2 salute/armatura/rigenerazione · 3 volo · 4 superforza · 5 fuoco/calore · 6 gelo · 7 scudo/lancio · 8 hacking/interazioni · 9 elettricità/magnetismo · 10 invisibilità · 11 telecinesi/controllo mentale · 12 ragnatele/rampino/arrampicata · 13 acrobazie/scavo/sensi · 14 big-fig/trasformazione · 15 passaggi stretti/teletrasporto · 16 poteri speciali (Galactus, ecc.).
Architettura: moduli indipendenti + parametri per personaggio + sistema comune dei bersagli ambientali.
