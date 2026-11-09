// MergeGiochi — soldi come stud di LEGO Marvel Super Heroes (richiesta di Dario del 9/11).
// Scritto da Claude. plugin-sdk, GTA SA 1.0 US.
//
//  - I mazzi di soldi (pickup "Money", ID 1212) sono sostituiti via Mod Loader da Money.dff/dyn_cash.txd:
//    lo stud d'oro originale di LMSH1 (STUFF\THINGS_NXG: quad gold_pan_coin1 + texture), che il gioco fa girare.
//  - Il contatore "$00000350" del HUD diventa quello dei livelli di LMSH1: icona dello stud, numero senza
//    zeri davanti e, sotto, la barra "True Believer" fatta di 10 stud (in LMSH1 è THINGS_NXG / coin_side1..10,
//    materiali TrueBar1..10, tutti con la stessa texture dello stud d'oro): si riempie con gli stud raccolti e,
//    quando è piena, appare "TRUE BELIEVER!" e riparte.
//  Tecnica: in CHud::DrawPlayerInfo la chiamata a CFont::PrintString dei soldi (0x58F607, verificata
//  disassemblando gta_sa.exe 1.0 US: segue sprintf con "$%08d" a 0x866C94) è deviata su StampaStud.

#include "plugin.h"
#include "common.h"
#include "CFont.h"
#include "CSprite2d.h"
#include "CTxdStore.h"
#include "CStreaming.h"
#include "CWorld.h"
#include "CPlayerInfo.h"
#include "CHud.h"
#include "CTimer.h"
#include "CPlayerPed.h"
#include <cstdio>
#include <cstring>

using namespace plugin;

namespace {

constexpr int ID_MONEY = 1212;
constexpr int OBIETTIVO_TRUE_BELIEVER = 20000;   // stud da raccogliere per riempire la barra

// puntatore mai distrutto: il distruttore di CSprite2d cancellerebbe la texture del txd all'uscita
CSprite2d& spriteStud = *new CSprite2d();
bool texturaCercata = false;
int ultimiSoldi = -1;
int raccolti = 0;                 // stud raccolti verso il prossimo True Believer
unsigned int fineScritta = 0;     // ms: fino a quando mostrare "TRUE BELIEVER!"

RwTexture* TexturaStud() {
    if (spriteStud.m_pTexture) return spriteStud.m_pTexture;
    if (texturaCercata && CTimer::m_snTimeInMilliseconds % 2000 > 30) return nullptr; // riprova ogni tanto
    texturaCercata = true;
    // la texture sta nel txd del pickup (dyn_cash): si tiene in memoria il modello del pickup
    if (CStreaming::ms_aInfoForModel[ID_MONEY].m_nLoadState != LOADSTATE_LOADED) {
        CStreaming::RequestModel(ID_MONEY, GAME_REQUIRED | KEEP_IN_MEMORY);
        return nullptr; // arriverà nei prossimi frame
    }
    int slot = CTxdStore::FindTxdSlot("dyn_cash");
    if (slot < 0) return nullptr;
    TxdDef* def = CTxdStore::ms_pTxdPool->GetAt(slot);
    if (!def || !def->m_pRwDictionary) return nullptr;
    RwTexture* t = RwTexDictionaryFindNamedTexture(def->m_pRwDictionary, "lmsh_stud");
    if (t) { CTxdStore::AddRef(slot); spriteStud.m_pTexture = t; }
    return t;
}

void ImpostaColore(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    // CFont::SetColor(CRGBA) prende il colore PER VALORE: chiamata diretta con i 4 byte (vedi FuocoInPunto)
    unsigned int c = r | (g << 8) | (b << 16) | (a << 24);
    plugin::Call<0x719430, unsigned int>(c);
}

void __cdecl StampaStud(float x, float y, const char*) {
    CPlayerInfo& pi = CWorld::Players[CWorld::PlayerInFocus];
    int soldi = pi.m_nDisplayMoney;
    char testo[32];
    std::snprintf(testo, sizeof(testo), soldi < 0 ? "-%d" : "%d", soldi < 0 ? -soldi : soldi);

    float altezza = SCREEN_HEIGHT * 0.045f;               // circa l'altezza delle cifre del HUD
    ImpostaColore(255, 214, 40, 255);                      // oro, come il contatore di LMSH1
    CFont::PrintString(x, y, testo);
    float largo = CFont::GetStringWidth(testo, true, false);

    if (RwTexture* t = TexturaStud()) {
        (void)t;
        float lato = altezza * 1.25f;
        float sx = x - largo - lato - altezza * 0.2f;
        spriteStud.Draw(CRect(sx, y - lato * 0.1f, sx + lato, y + lato * 0.9f), CRGBA(255, 255, 255, 255));

        // barra True Believer: 10 stud, da sinistra a destra, sotto il contatore
        float piccolo = altezza * 0.75f;
        float passo = piccolo * 0.95f;
        float bx = x - passo * 10.0f;
        float by = y + altezza * 1.35f;
        int pieni = raccolti * 10 / OBIETTIVO_TRUE_BELIEVER;
        for (int i = 0; i < 10; i++) {
            unsigned char a = i < pieni ? 255 : 70;
            spriteStud.Draw(CRect(bx + passo * i, by, bx + passo * i + piccolo, by + piccolo), CRGBA(255, 255, 255, a));
        }
    }
    if (CTimer::m_snTimeInMilliseconds < fineScritta) {
        ImpostaColore(255, 214, 40, 255);
        CFont::PrintString(x, y + altezza * 2.4f, "TRUE BELIEVER!");
    }
}

// Tasto di prova F6: crea un pickup dei soldi (lo stud) 2 m davanti al giocatore.
// CPickups::GenerateNewOne (0x456F20) prende il CVector PER VALORE: chiamata diretta con x, y, z.
void StudDiProva() {
    CPed* p = FindPlayerPed();
    if (!p) return;
    CVector pos = p->GetPosition() + p->GetMatrix().up * 2.0f;
    using Fn = int (__cdecl*)(float, float, float, unsigned int, unsigned char, unsigned int, unsigned int, bool, char*);
    reinterpret_cast<Fn>(0x456F20)(pos.x, pos.y, pos.z, ID_MONEY, 8 /* PICKUP_MONEY */, 500, 0, false, nullptr);
}

class ContatoreStud {
public:
    ContatoreStud() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return;
        patch::RedirectCall(0x58F607, reinterpret_cast<void*>(StampaStud));
        Events::gameProcessEvent += [] {
            static bool f6Prima = false;
            bool f6 = KeyPressed(VK_F6);
            if (f6 && !f6Prima) StudDiProva();
            f6Prima = f6;
            int soldi = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;
            if (ultimiSoldi >= 0 && soldi > ultimiSoldi) {
                raccolti += soldi - ultimiSoldi;
                if (raccolti >= OBIETTIVO_TRUE_BELIEVER) {
                    raccolti -= OBIETTIVO_TRUE_BELIEVER;
                    if (raccolti >= OBIETTIVO_TRUE_BELIEVER) raccolti = 0;
                    fineScritta = CTimer::m_snTimeInMilliseconds + 4000;
                }
            }
            ultimiSoldi = soldi;
        };
    }
} contatoreStud;

} // namespace
