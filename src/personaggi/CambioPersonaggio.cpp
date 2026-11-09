// MergeGiochi — Cambio personaggio (prototipo)
// Scritto da Claude. plugin-sdk, GTA SA 1.0 US, x86.
//
// F9 / F10: personaggio successivo / precedente del roster (Iron Man, Spider-Man, Captain America,
//           Wolverine, Hulk). Versione 3 (9/11): selettore al posto del solo Iron Man.
// L'ID del modello non è scritto nel codice: viene cercato per nome, così funziona con qualunque ID libero
// scelto in peds.ide.
//
// Versione 2 (8/11 23:20): corretto il crash al cambio modello (vedi CambiaModello).

#include "plugin.h"
#include "common.h"
#include "CPlayerPed.h"
#include "CModelInfo.h"
#include "CStreaming.h"
#include "CStreamingInfo.h"
#include "CClothes.h"
#include "CHud.h"
#include "CClumpModelInfo.h"
#include "CCutsceneMgr.h"
#include "CCutsceneObject.h"
#include "CAnimBlendClumpData.h"
#include "RenderWare.h"
#include <cstdio>

using namespace plugin;

namespace {

// log semplice in GTA-SA-mod\MergeGiochi.log (diagnostica)
static void Log(const char* fmt, int a = 0, int b = 0) {
    if (FILE* f = std::fopen("MergeGiochi.log", "a")) {
        std::fprintf(f, fmt, a, b); std::fputc('\n', f); std::fclose(f);
    }
}

// Roster in ordine di selezione. "cj" = modello 0 con i vestiti. I personaggi il cui modello non è in peds.ide
// vengono saltati, così il ciclo funziona anche mentre il roster cresce.
struct Personaggio { const char* modello; const char* nome; };
// CJ è stato tolto su richiesta di Dario (9/11): si parte da Iron Man e CJ non compare più, né in gioco né
// nei filmati realizzati col motore (il "csplay" dei filmati diventa il personaggio scelto, vedi sotto).
constexpr Personaggio ROSTER[] = {
    { "ironman", "Iron Man" },
    { "spidey",  "Spider-Man" },
    { "capam",   "Captain America" },
    { "wolvie",  "Wolverine" },
    { "hulk",    "Hulk" },
};
constexpr int N_ROSTER = sizeof(ROSTER) / sizeof(ROSTER[0]);

// modello del personaggio scelto (letto anche dal gancio dei filmati)
int g_idScelto = -1;
int g_idFilmato = -1; // modello speciale "csplay" dei filmati

static int TrovaModelloG(const char* nome) { int id = -1; CModelInfo::GetModelInfo(nome, &id); return id; }

// Al posto di CClothes::RebuildCutscenePlayer (0x5A8270), che costruisce CJ vestito nel modello speciale usato
// dai filmati: ci si mette una copia del modello del personaggio scelto (stesso scheletro dei pedoni di SA,
// quindi le animazioni dei filmati si applicano).
static void __cdecl FilmatoSenzaCJ(CPlayerPed* player, int idModelloFilmato) {
    int id = g_idScelto > 0 ? g_idScelto : TrovaModelloG("ironman");
    Log("Filmato: csplay (%d) -> personaggio %d", idModelloFilmato, id);
    g_idFilmato = idModelloFilmato;
    if (id <= 0) return;
    if (CStreaming::ms_aInfoForModel[id].m_nLoadState != LOADSTATE_LOADED) {
        CStreaming::RequestModel(id, GAME_REQUIRED | KEEP_IN_MEMORY);
        CStreaming::LoadAllRequestedModels(false);
    }
    CBaseModelInfo* src = CModelInfo::ms_modelInfoPtrs[id];
    CBaseModelInfo* dst = CModelInfo::ms_modelInfoPtrs[idModelloFilmato];
    if (!src || !dst || !src->m_pRwObject) { Log("Filmato: modello non disponibile", 0, 0); return; }
    RpClump* copia = RpClumpClone(reinterpret_cast<RpClump*>(src->m_pRwObject));
    if (dst->m_pRwObject) dst->DeleteRwObject();
    static_cast<CClumpModelInfo*>(dst)->SetClump(copia);
}


// I filmati di SA animano anche la TRASLAZIONE di ogni osso con le misure di CJ: sul nostro scheletro (minifig,
// bacino abbassato) il personaggio veniva stirato e deformato ("CJ distorto", prova di Dario del 9/11).
// Dopo l'aggiornamento delle animazioni si rimettono le traslazioni a quelle del nostro modello:
// radice = quella del filmato (posizione nella scena), tutte le altre ossa = misure del nostro modello
// (come nelle animazioni di gioco, dove si muove solo la radice).
static void RaddrizzaFilmato() {
    if (!CCutsceneMgr::ms_running || g_idFilmato < 0) return;
    for (unsigned int i = 0; i < CCutsceneMgr::ms_numCutsceneObjs; i++) {
        CCutsceneObject* o = CCutsceneMgr::ms_pCutsceneObjects[i];
        if (!o || o->m_nModelIndex != g_idFilmato || !o->m_pRwClump) continue;
        CAnimBlendClumpData* dati = RpClumpGetAnimBlendClumpData(o->m_pRwClump);
        if (!dati || !dati->m_pFrames) continue;
        for (int f = 0; f < dati->m_nNumFrames; f++) {
            AnimBlendFrameData& fd = dati->m_pFrames[f];
            if (!fd.m_pIFrame || fd.m_nNodeId == 0) continue;
            CVector* t = reinterpret_cast<CVector*>(reinterpret_cast<char*>(fd.m_pIFrame) + 16);
            // anche il bacino torna alla sua misura: sommargli l'abbassamento si accumulava di frame in frame
            // quando il filmato non anima la traslazione del bacino (Iron Man sprofondava, prova del 9/11)
            *t = fd.m_vecOffset;
        }
        if (RpHAnimHierarchy* h = GetAnimHierarchyFromSkinClump(o->m_pRwClump))
            RpHAnimHierarchyUpdateMatrices(h);
    }
}

// Al posto di CClothes::RebuildPlayer (0x5A82C0): il gioco lo chiama per (ri)costruire CJ vestito (inizio
// partita, caricamenti, morte, arresto, negozi, missioni). Qui il giocatore diventa subito il personaggio scelto:
// prima restava CJ finché non scendeva dal veicolo.
static void __cdecl RicostruisciSenzaCJ(CPlayerPed* player, bool) {
    int id = g_idScelto > 0 ? g_idScelto : TrovaModelloG("ironman");
    if (!player || id <= 0) return;
    if (CStreaming::ms_aInfoForModel[id].m_nLoadState != LOADSTATE_LOADED) {
        CStreaming::RequestModel(id, GAME_REQUIRED | KEEP_IN_MEMORY);
        CStreaming::LoadAllRequestedModels(false);
    }
    Log("RebuildPlayer intercettato: personaggio %d", id);
    unsigned int gruppoAnim = player->m_nAnimGroup;
    player->DeleteRwObject();
    player->m_nModelIndex = -1;
    player->SetModelIndex(id);
    player->m_nAnimGroup = gruppoAnim;
    g_idScelto = id;
}

class CambioPersonaggio {
public:
    CambioPersonaggio() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return;
        Events::gameProcessEvent += [this] { Aggiorna(); };
        patch::RedirectJump(0x5A8270, reinterpret_cast<void*>(FilmatoSenzaCJ));
        patch::RedirectJump(0x5A82C0, reinterpret_cast<void*>(RicostruisciSenzaCJ));
        Events::gameProcessEvent += [] { RaddrizzaFilmato(); };
        Events::objectRenderEvent += [](CObject*) { RaddrizzaFilmato(); };
    }

private:
    bool f9Prima = false;
    bool f10Prima = false;
    int attuale = 0; // indice in ROSTER (0 = Iron Man)

    static int TrovaModello(const char* nome) {
        int id = -1;
        CModelInfo::GetModelInfo(nome, &id);
        return id;
    }

    static bool Carica(int id) {
        CStreaming::RequestModel(id, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        return CStreaming::ms_aInfoForModel[id].m_nLoadState == LOADSTATE_LOADED;
    }

    // Sequenza presa dall'esempio ufficiale di plugin-sdk (examples/HandSignals):
    // senza DeleteRwObject il vecchio modello resta agganciato alle animazioni e il gioco va in crash
    // (crash osservato l'8/11 a 0x4D2C3A, nell'aggiornamento delle animazioni skinnate).
    static void CambiaModello(CPlayerPed* player, int id) {
        unsigned int gruppoAnim = player->m_nAnimGroup;
        Log("CambiaModello: da %d a %d", player->m_nModelIndex, id);
        player->DeleteRwObject();
        player->m_nModelIndex = -1;
        player->SetModelIndex(id);
        player->m_nAnimGroup = gruppoAnim;
        Log("CambiaModello: fatto, gruppo anim %d", gruppoAnim);
    }

    // passa al personaggio successivo (passo = +1) o precedente (passo = -1) disponibile
    void Cambia(CPlayerPed* player, int passo) {
        for (int tentativi = 1; tentativi <= N_ROSTER; tentativi++) {
            int i = ((attuale + passo * tentativi) % N_ROSTER + N_ROSTER) % N_ROSTER;
            const Personaggio& p = ROSTER[i];
            int id = TrovaModello(p.modello);
            if (id < 0) { Log("modello %d assente, saltato", i); continue; }
            if (!Carica(id)) { Log("modello %d non caricabile, saltato", i); continue; }
            int vecchio = player->m_nModelIndex;
            CambiaModello(player, id);
            if (vecchio > 0 && vecchio != id) CStreaming::SetModelIsDeletable(vecchio);
            attuale = i;
            g_idScelto = id;
            static char testo[128];
            std::snprintf(testo, sizeof(testo), "Ora sei %s (F9 avanti, F10 indietro)", p.nome);
            CHud::SetHelpMessage(testo, true, false, false);
            return;
        }
    }

    void Aggiorna() {
        CPlayerPed* player = FindPlayerPed();
        if (!player) return;

        // niente CJ: all'avvio, dopo un caricamento, una morte, un arresto o un cambio d'abiti il gioco rimette il
        // modello 0 (CJ); appena possibile si torna al personaggio scelto
        if (player->m_nModelIndex == 0 && !player->bInVehicle && !CCutsceneMgr::ms_running && player->m_pRwClump) {
            int id = TrovaModello(ROSTER[attuale].modello);
            if (id > 0 && Carica(id)) {
                Log("CJ sostituito dal personaggio %d", id, 0);
                CambiaModello(player, id);
                g_idScelto = id;
            }
        }

        bool f9 = KeyPressed(VK_F9), f10 = KeyPressed(VK_F10);
        int passo = (f9 && !f9Prima) ? 1 : (f10 && !f10Prima) ? -1 : 0;
        f9Prima = f9; f10Prima = f10;
        if (!passo) return;
        if (player->bInVehicle) {
            CHud::SetHelpMessage("Scendi dal veicolo per cambiare personaggio", true, false, false);
            return;
        }
        Cambia(player, passo);
    }
} cambioPersonaggio;

} // namespace
