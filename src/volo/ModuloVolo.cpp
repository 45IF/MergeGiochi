// MergeGiochi — Modulo Volo (prototipo per Iron Man)
// Scritto da Claude. Da compilare con plugin-sdk (GTA SA 1.0 US), x86.
//
// Comandi (tasti di default di SA PC, letti tramite CPad, quindi rispettano la configurazione del giocatore):
//   - SALTO mentre si è già in aria  -> decollo / passa in volo
//   - SALTO tenuto in volo           -> sale
//   - ACCOVACCIATI (C) in volo       -> scende; vicino al suolo -> atterra
//   - ACCOVACCIATI due volte rapido  -> caduta libera (SALTO in aria la interrompe e riprende il volo)
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
#include "CModelInfo.h"
#include "CAnimManager.h"
#include "CAnimBlendAssociation.h"
#include "CAnimBlock.h"
#include "CStreaming.h"
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
    unsigned int doppioTocco = 350;   // ms massimi fra due pressioni di ACCOVACCIATI per la caduta libera
    unsigned int protezioneDopoAtterraggio = 1500; // ms di protezione dai danni da caduta dopo aver toccato terra
};

// Animazioni di volo: per ora animazioni ORIGINALI di SA (ped.ifp), scelte per somigliare agli stati
// di LMSH1 (IRONMAN_FLY.AS): hover -> Swim_Tread (in piedi, gambe e braccia che "galleggiano"),
// fly / fly_sprint -> Swim_Glide (corpo orizzontale, braccia avanti). Quando le animazioni AN4 di LMSH1
// saranno convertite in IFP, basterà cambiare questa tabella.
// blocco: nome del file IFP in anim.img che va caricato prima di usare il gruppo (nullptr = ped.ifp, sempre carico).
// ATTENZIONE: il gruppo "swim" sta in swim.ifp, che il gioco carica solo quando si entra in acqua: usarlo senza
// caricarlo fa crashare a 0x4CE15C (provato il 9/11).
struct AnimVolo { int gruppo; int id; const char* blocco; };
constexpr AnimVolo ANIM_HOVER  { 0, 14, nullptr };   // default / ANIM_DEFAULT_SWIM_TREAD
constexpr AnimVolo ANIM_VOLO   { 71, 315, "swim" };  // swim / ANIM_SWIM_SWIM_GLIDE
constexpr int ID_BASE_IFP = 25575;                   // SA 1.0: primo indice di streaming dei file IFP
constexpr int FLAG_RIMUOVI_A_FINE_SFUMATURA = 0x4; // in SA: l'associazione si cancella quando la fusione arriva a 0

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
    bool cadutaLibera = false;
    int idIronMan = -2;
    unsigned int ultimoAccovaccia = 0;   // ms dell'ultima pressione di ACCOVACCIATI in volo
    unsigned int fineProtezione = 0;     // ms: fino a quando il pedone resta protetto dai danni da caduta
    bool protezioneMessa = false;
    CAnimBlendAssociation* animCorrente = nullptr;
    int idAnimCorrente = -1;

    // --- animazioni ---------------------------------------------------------------------------------
    // L'animazione di volo viene aggiunta come animazione "parziale" a fusione 1: in SA le parziali si
    // sovrappongono alle animazioni di base (camminata/inattività del compito del giocatore) e non vengono
    // sfumate via da CAnimManager::BlendAnimation, quindi il compito del giocatore non le "combatte".
    static bool AnimAncoraPresente(CPed* ped, CAnimBlendAssociation* a) {
        if (!a || !ped->m_pRwClump) return false;
        for (CAnimBlendAssociation* x = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump); x; x = RpAnimBlendGetNextAssociation(x))
            if (x == a) return true;
        return false;
    }

    // true se il blocco di animazioni è in memoria; altrimenti ne chiede il caricamento (asincrono) e
    // tiene un riferimento perché il gioco non lo scarichi quando esce dall'acqua
    bool riferimentoSwim = false;
    bool BloccoPronto(const char* nome) {
        if (!nome) return true;
        int idx = CAnimManager::GetAnimationBlockIndex(nome);
        if (idx < 0) return false;
        CAnimBlock& blocco = CAnimManager::ms_aAnimBlocks[idx];
        if (!blocco.bLoaded) {
            CStreaming::RequestModel(ID_BASE_IFP + idx, 0x8); // 0x8 = tieni in memoria
            return false;
        }
        if (!riferimentoSwim) {
            CAnimManager::AddAnimBlockRef(idx);
            riferimentoSwim = true;
        }
        return true;
    }

    void SuonaAnimVolo(CPed* ped, const AnimVolo& animRichiesta, float velocitaSfumatura = 6.0f) {
        if (!ped->m_pRwClump) return;
        // se il blocco non è ancora pronto si resta con l'animazione di hover (sempre disponibile)
        const AnimVolo& anim = BloccoPronto(animRichiesta.blocco) ? animRichiesta : ANIM_HOVER;
        if (idAnimCorrente == anim.id && AnimAncoraPresente(ped, animCorrente)) {
            animCorrente->m_bPartial = 1;
            if (animCorrente->m_fBlendDelta < 0.0f) animCorrente->m_fBlendDelta = velocitaSfumatura;
            return;
        }
        // sfuma via la precedente
        if (AnimAncoraPresente(ped, animCorrente)) {
            animCorrente->m_nFlags |= FLAG_RIMUOVI_A_FINE_SFUMATURA;
            animCorrente->m_fBlendDelta = -velocitaSfumatura;
        }
        CAnimBlendAssociation* a = CAnimManager::AddAnimation(ped->m_pRwClump, anim.gruppo, anim.id);
        if (!a) { animCorrente = nullptr; idAnimCorrente = -1; return; }
        a->m_nFlags |= ANIMATION_STARTED | ANIMATION_LOOPED | ANIMATION_PARTIAL;
        a->m_nFlags &= ~(ANIMATION_MOVEMENT | ANIMATION_TRANLSATE_X | ANIMATION_TRANLSATE_Y | FLAG_RIMUOVI_A_FINE_SFUMATURA);
        a->m_fBlendAmount = 0.0f;
        a->m_fBlendDelta = velocitaSfumatura;
        animCorrente = a;
        idAnimCorrente = anim.id;
    }

    void FermaAnimVolo(CPed* ped) {
        if (AnimAncoraPresente(ped, animCorrente)) {
            animCorrente->m_nFlags |= FLAG_RIMUOVI_A_FINE_SFUMATURA;
            animCorrente->m_fBlendDelta = -8.0f;
        }
        animCorrente = nullptr;
        idAnimCorrente = -1;
    }

    // --- protezione dai danni da caduta (Iron Man "FallsWithStyle" in LMSH1) ------------------------
    void Proteggi(CPed* ped, bool si) {
        if (si == protezioneMessa) return;
        ped->bCollisionProof = si;
        protezioneMessa = si;
    }

    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    void Messaggio(const char* testo) {
        CHud::SetHelpMessage(testo, true, false, false);
    }

    void IniziaVolo(CPed* ped) {
        inVolo = true;
        cadutaLibera = false;
        Proteggi(ped, true);
        BloccoPronto(ANIM_VOLO.blocco); // avvia subito il caricamento di swim.ifp
        velocita = CVector(ped->m_vecMoveSpeed.x, ped->m_vecMoveSpeed.y, 0.0f) * TICK_AL_SECONDO;
        ped->bApplyGravity = false;
        if (ped->m_pIntelligence)
            ped->m_pIntelligence->FlushImmediately(true); // true = rimette il compito di base del giocatore
    }

    // libera = true: caduta libera (si resta protetti finché non si tocca terra)
    void FineVolo(CPed* ped, bool libera = false) {
        inVolo = false;
        cadutaLibera = libera;
        FermaAnimVolo(ped);
        fineProtezione = CTimer::m_snTimeInMilliseconds + p.protezioneDopoAtterraggio;
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

        // il volo è l'abilità di movimento di Iron Man: con gli altri personaggi il modulo è spento
        if (idIronMan == -2) { int id = -1; CModelInfo::GetModelInfo("ironman", &id); idIronMan = id; }
        bool eIronMan = ped->m_nModelIndex == idIronMan;

        // niente volo dentro i veicoli, da morto o con un personaggio che non vola
        if (ped->m_fHealth <= 0.0f || ped->bInVehicle || !eIronMan) {
            if (inVolo) FineVolo(ped);
            return;
        }

        const unsigned int ora = CTimer::m_snTimeInMilliseconds;

        if (!inVolo) {
            // caduta libera: si resta protetti finché non si tocca terra, poi ancora per un attimo
            if (cadutaLibera) {
                if (ped->bIsStanding) {
                    cadutaLibera = false;
                    fineProtezione = ora + p.protezioneDopoAtterraggio;
                }
            } else if (protezioneMessa && ora > fineProtezione) {
                Proteggi(ped, false);
            }

            // decollo: salto premuto mentre si è già in aria (doppio salto, o durante la caduta libera)
            if ((ped->bIsInTheAir || (cadutaLibera && !ped->bIsStanding)) && pad->JumpJustDown())
                IniziaVolo(ped);
            return;
        }

        // caduta libera: ACCOVACCIATI premuto due volte di fila rapidamente
        if (pad->DuckJustDown()) {
            if (ultimoAccovaccia != 0 && ora - ultimoAccovaccia <= p.doppioTocco && DistanzaSuolo(ped) > p.distanzaAtterraggio) {
                ultimoAccovaccia = 0;
                FineVolo(ped, true);
                Messaggio("Caduta libera");
                return;
            }
            ultimoAccovaccia = ora;
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

        // animazione: hover (in piedi) quando quasi fermo, volo orizzontale quando si va veloci
        float vel2D = velocita.Magnitude2D();
        if (vel2D > p.velHover * 0.9f)
            SuonaAnimVolo(ped, ANIM_VOLO);
        else if (vel2D < p.velHover * 0.6f)
            SuonaAnimVolo(ped, ANIM_HOVER);
        else if (idAnimCorrente < 0)
            SuonaAnimVolo(ped, ANIM_HOVER);
        else
            SuonaAnimVolo(ped, idAnimCorrente == ANIM_VOLO.id ? ANIM_VOLO : ANIM_HOVER); // isteresi
        Proteggi(ped, true);

        // evita che il gioco rimetta il pedone in caduta (da verificare in gioco)
        if (ped->m_pIntelligence)
            ped->m_pIntelligence->FlushImmediately(true); // true = rimette il compito di base del giocatore
    }
} moduloVolo;

} // namespace
