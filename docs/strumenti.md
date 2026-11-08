# Strumenti — ricerca di Grok, controllata da Claude (8/11/2026)

Codex: prima di scaricare, verifica che il link sia la pagina ufficiale/release del progetto. Annota versione e SHA-256 di ogni download qui sotto.

## San Andreas
| Scopo | Strumento | Fonte | Note |
|---|---|---|---|
| Downgrade Steam → 1.0 US | **Jetpack Downgrader** (consigliato da Claude) | github.com/Zalexanninev15/Jetpack-Downgrader | Supporta in pieno Steam e RGL; richiede .NET 4.8 |
| Downgrade (alternativa) | GTA SA Open Downgrader | github.com/xxanqw/gtasa-open-downgrader | Indicato da Grok, da verificare |
| Caricatore ASI | Ultimate ASI Loader | github.com/ThirteenAG/Ultimate-ASI-Loader | |
| Gestione mod | Modloader | github.com/thelink2012/modloader | |
| Script | CLEO 4.4 (prima scelta) / CLEO 5 | cleo.li, github.com/cleolibrary | CLEO 4.4 più compatibile con vecchi script; CLEO 5 più moderno |
| Limiti | Open Limit Adjuster | github.com/GTAmodding/III.VC.SA.LimitAdjuster | fastman92 LA solo se serve di più |
| SDK C++ | plugin-sdk | github.com/DK22Pac/plugin-sdk | Visual Studio 2022 |
| Modelli | DragonFF (add-on Blender), Magic.TXD | github.com/Parik27/DragonFF | |

## LEGO Marvel Super Heroes
| Scopo | Strumento | Note |
|---|---|---|
| Estrarre GAME*.DAT | QuickBMS (`quickbms_4gb_files.exe`) + script `ttgames.bms` di aluigi (aluigi.altervista.org) | Prima solo in modalità lista |
| Importare modelli/scheletri | Noesis (supporto parziale), TTGames-Workshop / altri strumenti della community | **Punto critico non verificato**: secondo Grok non esiste un importatore completo e affidabile. Va testato subito su Iron Man. |

## Riferimenti
- Mod "Iron Man" per SA esistenti (CLEO e ASI) indicate da Grok: utili solo per studiare l'approccio al volo, da cercare su gtaforums/GitHub.

## Download effettuati (compila Codex)
| Data | Strumento | Versione | URL | SHA-256 |
|---|---|---|---|---|

| 2026-11-08 | QuickBMS | 0.12 (pagina autore) | https://aluigi.altervista.org/papers/quickbms.zip | B9D4F9EFB55692994CD42A491CFEA11F86E3375A618B9BD771583CE40DDB3828 |
| 2026-11-08 | ttgames.bms | 0.9.11a | https://aluigi.altervista.org/bms/ttgames.bms | 996EAC13A8C46B8F3724C53C7928036AD2AF801911705DC7A983359662B82D04 |

| 2026-11-08 | Jetpack con patch | 2.3.0.1 | https://github.com/Zalexanninev15/Jetpack-Downgrader/releases/download/2.3.0.1/JPD_WithPatches.zip | F5203358D1905AE0598DA664ADDC1B75F98CEFEE61EA5DA39599A78F7F7825A6 |
| 2026-11-08 | TT Character Importer | 0.5.31 | https://github.com/Loomirr/TTGames-Workshop/blob/main/builds/blender/TT_Character_Importer_0.5.31.zip | 54DE4E06783AF82FD1525D6B4336CB0392DAA213064DA16C68B488DDAE739D14 |

