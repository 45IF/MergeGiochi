# Compito 03 — Codex: compilare e provare il modulo Volo

Prerequisiti: SA 1.0 US in GTA-SA-mod (compito 02, parte A) + ASI Loader + Build Tools/plugin-sdk funzionanti.

1. `src/volo/ModuloVolo.cpp` è stato scritto da Claude con le API di plugin-sdk (commit 15f15b6). Ha passato un controllo di sintassi con gli header di plugin-sdk (compilatore mingw, C++23), ma NON è mai stato compilato con MSVC né provato in gioco.
2. Aggiungi a `premake5.lua` un progetto ASI `MergeGiochi` (x86, C++20 o successivo, stesse impostazioni di ProvaASI) che compila `src/volo/*.cpp`. Output in `build/`, file `MergeGiochi.asi`.
3. Compila. Se MSVC dà errori, correggili restando fedele alla logica e annota le modifiche in docs/stato.md.
4. Copia `MergeGiochi.asi` in `GTA-SA-mod\scripts\` (o nella cartella letta da Ultimate ASI Loader) e avvia una partita nuova.
5. Test (descrivi l'esito in docs/stato.md, con screenshot se riesci):
   - salto, poi di nuovo SALTO in aria → parte il volo?
   - SALTO tenuto sale, C scende, SCATTO boost, movimento relativo alla camera;
   - C vicino al suolo → atterra e torna normale;
   - contro un edificio si ferma invece di attraversarlo;
   - F8 attiva/disattiva il modulo;
   - il personaggio va in animazione di caduta o in ragdoll durante il volo? (punto più incerto)
6. Commit e push (solo sorgenti e documentazione).
