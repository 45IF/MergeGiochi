// MergeGiochi — Cambio personaggio (prototipo)
// Scritto da Claude. plugin-sdk, GTA SA 1.0 US, x86.
//
// F9: il giocatore diventa Iron Man (modello "ironman" registrato in peds.ide tramite Modloader);
//     premendo di nuovo torna CJ con i suoi vestiti.
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
#include <cstdio>

using namespace plugin;

namespace {

// log semplice in GTA-SA-mod\MergeGiochi.log (diagnostica)
static void Log(const char* fmt, int a = 0, int b = 0) {
    if (FILE* f = std::fopen("MergeGiochi.log", "a")) {
        std::fprintf(f, fmt, a, b); std::fputc('\n', f); std::fclose(f);
    }
}

class CambioPersonaggio {
public:
    CambioPersonaggio() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return;
        Events::gameProcessEvent += [this] { Aggiorna(); };
    }

private:
    bool f9Prima = false;
    bool f10Prima = false;
    bool f11Prima = false;
    bool comeIronMan = false;

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

    void Aggiorna() {
        CPlayerPed* player = FindPlayerPed();
        if (!player) return;

        // F10 (diagnostica): ciclo CJ -> male01 (ID 7, originale) -> mgtest (busto originale di CJ) -> CJ.
        bool f10 = KeyPressed(VK_F10);
        if (f10 && !f10Prima && !player->bInVehicle) {
            int idTest = TrovaModello("mgtest");
            if (player->m_nModelIndex == 0 || comeIronMan) {
                Log("F10: prova con male01");
                if (Carica(7)) { CambiaModello(player, 7); CStreaming::SetModelIsDeletable(7); }
            } else if (player->m_nModelIndex == 7 && idTest >= 0) {
                Log("F10: prova A (mgtest), ID %d", idTest);
                if (Carica(idTest)) { CambiaModello(player, idTest); CStreaming::SetModelIsDeletable(idTest); }
                Log("F10: prova A applicata");
            } else if (player->m_nModelIndex == idTest && TrovaModello("mgtest2") >= 0) {
                int id2 = TrovaModello("mgtest2");
                Log("F10: prova B (mgtest2), ID %d", id2);
                if (Carica(id2)) { CambiaModello(player, id2); CStreaming::SetModelIsDeletable(id2); }
                Log("F10: prova B applicata");
            } else {
                Log("F10: ritorno a CJ");
                CambiaModello(player, 0); CClothes::RebuildPlayer(player, false);
            }
            comeIronMan = false;
        }
        f10Prima = f10;

        bool f9 = KeyPressed(VK_F9);
        bool premuto = f9 && !f9Prima;
        f9Prima = f9;
        if (!premuto) return;
        if (player->bInVehicle) {
            CHud::SetHelpMessage("Scendi dal veicolo per cambiare personaggio", true, false, false);
            return;
        }

        if (!comeIronMan) {
            int id = TrovaModello("ironman");
            Log("F9: modello ironman ha ID %d", id);
            if (id < 0) {
                CHud::SetHelpMessage("Modello ironman non trovato: controlla peds.ide in modloader", true, false, false);
                return;
            }
            if (!Carica(id)) {
                CHud::SetHelpMessage("Impossibile caricare ironman.dff/txd", true, false, false);
                return;
            }
            CambiaModello(player, id);
            CStreaming::SetModelIsDeletable(id);
            comeIronMan = true;
            CHud::SetHelpMessage("Ora sei Iron Man (F9 per tornare CJ)", true, false, false);
        } else {
            CambiaModello(player, 0);               // 0 = modello del giocatore (CJ)
            CClothes::RebuildPlayer(player, false); // ricostruisce i vestiti di CJ
            comeIronMan = false;
            CHud::SetHelpMessage("Sei tornato CJ", true, false, false);
        }
    }
} cambioPersonaggio;

} // namespace
