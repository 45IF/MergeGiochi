// MergeGiochi — Modulo Volo (prototipo per Iron Man)
// Scritto da Claude. Da compilare con plugin-sdk (GTA SA 1.0 US), x86.
//
// Comandi (tasti di default di SA PC, letti tramite CPad, quindi rispettano la configurazione del giocatore):
//   - SALTO mentre si è già in aria  -> decollo / passa in volo
//   - SALTO tenuto in volo           -> sale
//   - ACCOVACCIATI (C) in volo       -> scende; vicino al suolo -> atterra
//   - SCATTO in volo                 -> boost
//   - movimento                      -> direzione relativa alla camera
//   - tasto F8                       -> attiva/disattiva il modulo (debug)
//
// Velocità dalle specifiche v0.1 (ChatGPT-B): hover 8 m/s, crociera 20 m/s, boost 35 m/s.
// In SA m_vecMoveSpeed è espresso in unità per "tick" da 1/50 s, quindi m/s / 50.
//
// DA VERIFICARE IN GIOCO (incertezze note, vedi docs/ricerca-volo.md):
//   - se FlushImmediately ogni frame basta a impedire animazione di caduta/ragdoll senza effetti collaterali;
//   - segni degli assi dell'input (GetPedWalkUpDown negativo = avanti, secondo le convenzioni di SA).

#include "plugin.h"
#include "common.h"
#include "CPed.h"
#include "CPlayerPed.h"
#include "CPad.h"
#include "CCamera.h"
#include "CTimer.h"
#include "CWorld.h"
#include "CColPoint.h"
#include "CHud.h"
#include "CPedIntelligence.h"
#include <cmath>
#include <algorithm>

using namespace plugin;

namespace {

struct ParametriVolo {
    float velHover    = 8.0f;   // m/s velocità orizzontale lenta
    float velCrociera = 20.0f;  // m/s
    float velBoost    = 35.0f;  // m/s
    float velVerticale = 10.0f; // m/s salita/discesa
    float accelerazione = 4.0f; // quanto velocemente si raggiunge la velocità desiderata (1/s)
    float quotaMinima = 1.2f;   // m dal suolo durante l'hover
    float distanzaAtterraggio = 1.6f; // m: sotto questa quota, ACCOVACCIATI fa atterrare
};

constexpr float TICK_AL_SECONDO = 50.0f;

class ModuloVolo {
public:
    ModuloVolo() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return; // solo SA 1.0 US

        Events::gameProcessEvent += [this] { Aggiorna(); };
    }

private:
    ParametriVolo p;
    bool attivo = true;      // modulo abilitato (F8)
    bool inVolo = false;
    bool f8Prima = false;
    CVector velocita{0.0f, 0.0f, 0.0f}; // m/s, mondo

    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    void Messaggio(const char* testo) {
        CHud::SetHelpMessage(testo, true, false, false);
    }

    void IniziaVolo(CPed* ped) {
        inVolo = true;
        velocita = CVector(ped->m_vecMoveSpeed.x, ped->m_vecMoveSpeed.y, 0.0f) * TICK_AL_SECONDO;
        ped->bApplyGravity = false;
        if (ped->m_pIntelligence)
            ped->m_pIntelligence->FlushImmediately(true); // true = rimette il compito di base del giocatore
    }

    void FineVolo(CPed* ped) {
        inVolo = false;
        ped->bApplyGravity = true;
        // riparte con il compito di base del giocatore (senza, il gestore dei compiti trova un puntatore
        // nullo: crash a 0x523E6B osservato l'8/11 all'atterraggio)
        if (ped->m_pIntelligence)
            ped->m_pIntelligence->FlushImmediately(true);
        velocita = CVector(0.0f, 0.0f, 0.0f);
    }

    // distanza dal suolo sotto il pedone (m); valore grande se niente sotto entro 50 m
    float DistanzaSuolo(CPed* ped) {
        CVector pos = ped->GetPosition();
        CColPoint punto;
        CEntity* entita = nullptr;
        if (CWorld::ProcessVerticalLine(pos, pos.z - 50.0f, punto, entita, true, true, false, true, false, false, nullptr))
            return pos.z - punto.m_vecPoint.z;
        return 999.0f;
    }

    bool OstacoloDavanti(CPed* ped, const CVector& direzione, float distanza) {
        if (direzione.MagnitudeSqr() < 0.0001f) return false;
        CVector da = ped->GetPosition();
        CVector a = da + direzione.Normalized() * distanza;
        CColPoint punto;
        CEntity* entita = nullptr;
        return CWorld::ProcessLineOfSight(da, a, punto, entita, true, true, false, true, false, false, false, false);
    }

    void Aggiorna() {
        CPlayerPed* ped = FindPlayerPed();
        if (!ped) return;
        CPad* pad = CPad::GetPad(0);
        if (!pad) return;

        // F8: abilita/disabilita il modulo
        bool f8 = KeyPressed(VK_F8);
        if (f8 && !f8Prima) {
            attivo = !attivo;
            if (!attivo && inVolo) FineVolo(ped);
            Messaggio(attivo ? "Volo: modulo attivo" : "Volo: modulo disattivato");
        }
        f8Prima = f8;

        if (!attivo) return;

        // niente volo dentro i veicoli o da morto
        if (ped->m_fHealth <= 0.0f || ped->bInVehicle) {
            if (inVolo) FineVolo(ped);
            return;
        }

        if (!inVolo) {
            // decollo: salto premuto mentre si è già in aria (doppio salto)
            if (ped->bIsInTheAir && pad->JumpJustDown())
                IniziaVolo(ped);
            return;
        }

        const float dt = CTimer::ms_fTimeStep / TICK_AL_SECONDO; // secondi di questo frame

        // direzione della camera sul piano orizzontale
        CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
        CVector avanti(cam.m_vecFront.x, cam.m_vecFront.y, 0.0f);
        if (avanti.MagnitudeSqr() < 0.0001f) avanti = CVector(0.0f, 1.0f, 0.0f);
        avanti.Normalize();
        CVector destra(avanti.y, -avanti.x, 0.0f);

        // input analogico in [-1, 1]
        float ingAvanti = -pad->GetPedWalkUpDown() / 128.0f;
        float ingLato = pad->GetPedWalkLeftRight() / 128.0f;
        ingAvanti = std::clamp(ingAvanti, -1.0f, 1.0f);
        ingLato = std::clamp(ingLato, -1.0f, 1.0f);

        CVector dirOrizz = avanti * ingAvanti + destra * ingLato;
        float intensita = std::min(1.0f, dirOrizz.Magnitude());
        float velMax = pad->GetSprint() ? p.velBoost : (intensita > 0.6f ? p.velCrociera : p.velHover);
        CVector desiderata = (intensita > 0.01f) ? dirOrizz.Normalized() * (velMax * intensita) : CVector(0.0f, 0.0f, 0.0f);

        // verticale
        float suolo = DistanzaSuolo(ped);
        if (pad->GetJump()) desiderata.z = p.velVerticale;
        else if (pad->GetDuck()) desiderata.z = -p.velVerticale;
        else desiderata.z = (suolo < p.quotaMinima) ? p.velVerticale * 0.5f : 0.0f; // hover

        // atterraggio
        if (pad->GetDuck() && suolo < p.distanzaAtterraggio) {
            FineVolo(ped);
            return;
        }

        // niente sfondamento di edifici: blocca la componente orizzontale contro un ostacolo vicino
        if (OstacoloDavanti(ped, CVector(desiderata.x, desiderata.y, 0.0f), 1.5f)) {
            desiderata.x = 0.0f;
            desiderata.y = 0.0f;
        }

        // avvicinamento morbido alla velocità desiderata
        float t = std::min(1.0f, p.accelerazione * dt);
        velocita.x = Lerp(velocita.x, desiderata.x, t);
        velocita.y = Lerp(velocita.y, desiderata.y, t);
        velocita.z = Lerp(velocita.z, desiderata.z, t);

        ped->bApplyGravity = false;
        ped->m_vecMoveSpeed = velocita / TICK_AL_SECONDO;

        // orientamento del personaggio verso la direzione di volo
        if (velocita.Magnitude2D() > 1.0f) {
            float angolo = std::atan2(-velocita.x, velocita.y);
            ped->m_fHeadingCurrent = angolo;
            ped->m_fHeadingGoal = angolo;
            ped->SetHeading(angolo);
        }

        // evita che il gioco rimetta il pedone in caduta (da verificare in gioco)
        if (ped->m_pIntelligence)
            ped->m_pIntelligence->FlushImmediately(true); // true = rimette il compito di base del giocatore
    }
} moduloVolo;

} // namespace
