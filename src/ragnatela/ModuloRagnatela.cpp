// MergeGiochi — Spider-Man: dondolo con le ragnatele per la città.
// Scritto da Claude. plugin-sdk, GTA SA 1.0 US.
//
// Stessi comandi del volo di Iron Man (richiesta di Dario del 9/11):
//   - SALTO mentre si è in aria         -> spara una ragnatela in avanti e in alto e si dondola
//   - un arco di dondolo completo parte da solo (non serve tenere premuto): come in LMSH1 (abilità
//     "SpiderManWebSwing" del hub) l'arco è "guidato", non un pendolo fisico: scende, risale e alla fine lancia
//     in avanti e in alto; un nuovo SALTO in aria (anche a metà arco) spara la ragnatela successiva
//   - movimento laterale                -> curva durante l'arco; la direzione iniziale è quella della camera
//   - AVANTI tenuto                     -> dondolo continuo, un arco dopo l'altro (come nel hub di LMSH1)
//   - ACCOVACCIATI due volte rapido      -> molla tutto e cade (senza danni da caduta)
// Pareti (richiesta di Dario del 9/11): camminando contro un muro Spider-Man ci si attacca e si arrampica
//   - avanti/indietro = su/giù, sinistra/destra = di lato, SCATTO = più veloce
//   - SALTO = si stacca con un balzo all'indietro (poi si può sparare subito una ragnatela)
//   - ACCOVACCIATI = si lascia cadere; arrivato in cima scavalca e torna in piedi sul tetto
//   - animazione: la nuotata di SA (Swim_Crawl), con il corpo ruotato a pancia contro il muro
// Fisica: pendolo con filo inestensibile (vincolo sulla sfera di raggio = lunghezza del filo) e gravità.
// Animazione: Climb_Idle di SA (appeso con le braccia in alto) come animazione parziale, come per il volo.
// Il filo è una striscia bianca disegnata con la texture originale "shad_exp" di SA.

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
#include "CModelInfo.h"
#include "CPedIntelligence.h"
#include "CAnimManager.h"
#include "CAnimBlendAssociation.h"
#include "CAnimBlock.h"
#include "CStreaming.h"
#include "CAnimBlendHierarchy.h"
#include "../comune/Grafica.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

using namespace plugin;

namespace {

constexpr float TICK = 50.0f;
constexpr float GRAVITA = 14.0f;          // m/s² (un po' più della realtà: dondolo più vivace)
constexpr float SPINTA = 9.0f;            // m/s² dall'input durante il dondolo
constexpr float RIAVVOLGI = 7.0f;         // m/s di accorciamento del filo con SCATTO
constexpr float PORTATA = 90.0f;          // m massimi del filo
constexpr float FILO_MIN = 4.0f;
constexpr float ANCORA_AVANTI = 16.0f;    // m: ancoraggio "nel cielo" se non c'è nessuna struttura
constexpr float ANCORA_ALTO = 24.0f;
constexpr float ARCO_LUNGHEZZA = 24.0f;   // m percorsi in orizzontale da un arco
constexpr float ARCO_DURATA = 1.25f;      // s
constexpr float ARCO_RISALITA = 3.5f;     // m sopra la quota di partenza alla fine dell'arco
constexpr float ARCO_DISCESA_MAX = 5.0f;  // m di discesa massima a metà arco
constexpr float CURVA = 1.3f;             // rad/s di sterzata durante l'arco
constexpr unsigned int PAUSA_PARETE = 600; // ms dopo aver lasciato un muro prima di potersi riattaccare
constexpr float VEL_MAX = 45.0f;          // m/s
constexpr int ANIM_APPESO = 129;          // default / ANIM_DEFAULT_CLIMB_IDLE
constexpr int BONE_MANO_DESTRA = 24;   // " R Hand" nello scheletro dei pedoni (25 è il dito)
constexpr int ANIM_PARETE_GRUPPO = 71, ANIM_PARETE = 312;   // swim / Swim_Crawl (in swim.ifp, da caricare)
constexpr int ID_BASE_IFP = 25575;
constexpr float VEL_PARETE = 4.0f, VEL_PARETE_SCATTO = 7.5f;
constexpr float DIST_PARETE = 0.45f;      // m fra la radice del pedone e il muro
constexpr unsigned int DOPPIO_TOCCO = 350;
constexpr unsigned int PROTEZIONE = 1500;

class ModuloRagnatela {
public:
    ModuloRagnatela() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return;
        Events::gameProcessEvent += [this] { Aggiorna(); };
        Events::renderEffectsEvent += [this] { Disegna(); };
        // il gioco rimette il pedone dritto durante la sua elaborazione: prima di disegnarlo si riapplica
        // l'orientamento sulla parete
        Events::pedRenderEvent += [this](CPed* ped) {
            if (inParete && ped == FindPlayerPed()) {
                CVector d = ped->GetMatrix().up;
                (void)d; OrientaSuParete(ped, ultimaDirezione);
            }
        };
    }

private:
    int idSpidey = -2;
    bool attaccato = false;
    bool cadutaProtetta = false;
    bool protezioneMessa = false;
    unsigned int fineProtezione = 0;
    unsigned int ultimoAccovaccia = 0;
    CVector ancora, velocita;
    float lunghezza = 0.0f;
    // arco del dondolo
    float arcoT = 0.0f, arcoDiscesa = 0.0f, arcoRisalita = 3.5f;
    CVector arcoDir;
    unsigned int fineParete = 0;
    CAnimBlendAssociation* anim = nullptr;
    // parete
    bool inParete = false;
    CVector normaleParete;
    CAnimBlendAssociation* animParete = nullptr;
    bool rifSwim = false;
    CVector ultimaDirezione{0.0f, 0.0f, 1.0f};
    // filo da disegnare
    bool disegnaFilo = false;
    CVector filoDa, filoA;

    int IdSpidey() {
        if (idSpidey == -2) { int id = -1; CModelInfo::GetModelInfo("spidey", &id); idSpidey = id; }
        return idSpidey;
    }

    void Proteggi(CPed* ped, bool si) {
        if (si == protezioneMessa) return;
        ped->bCollisionProof = si;
        protezioneMessa = si;
    }

    bool AnimPresente(CPed* ped) {
        if (!anim || !ped->m_pRwClump) return false;
        for (CAnimBlendAssociation* x = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump); x; x = RpAnimBlendGetNextAssociation(x))
            if (x == anim) return true;
        return false;
    }

    // --- animazioni originali di LMSH1 (IFP generato da tools/retarget_an4_ifp.py: ws_left1, ws_right1,
    //     ws_up_l1, ws_up_r1, ws_fall, ws_idle), caricate una volta da modloader\MergeGiochi\mgspidey.ifp.
    //     Se il file manca si usa Climb_Idle di SA come prima.
    CAnimBlock* bloccoLmsh = nullptr;
    bool provatoIfp = false;
    int arcoNumero = 0;
    CAnimBlock* BloccoLmsh() {
        if (provatoIfp) return bloccoLmsh;
        provatoIfp = true;
        bloccoLmsh = CAnimManager::GetAnimationBlock("mgspidey");
        if (bloccoLmsh && bloccoLmsh->bLoaded) return bloccoLmsh;
        // estensione .anim e non .ifp: Mod Loader proverebbe a installare un .ifp come blocco di anim.img
        static char percorso[512];
        std::snprintf(percorso, sizeof(percorso), "%smgspidey.anim", plugin::paths::GetPluginDirPathA());
        RwStream* st = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, percorso);
        if (!st) { bloccoLmsh = nullptr; return nullptr; }
        CAnimManager::LoadAnimFile(st, true, nullptr);
        RwStreamClose(st, nullptr);
        bloccoLmsh = CAnimManager::GetAnimationBlock("mgspidey");
        if (bloccoLmsh) bloccoLmsh->bLoaded = 1;
        return bloccoLmsh;
    }

    void AvviaAnim(CPed* ped, const char* nomeLmsh = nullptr, float durata = 0.0f) {
        if (!ped->m_pRwClump) return;
        if (!nomeLmsh && AnimPresente(ped)) { anim->m_fBlendDelta = 8.0f; return; }
        CAnimBlendHierarchy* h = nullptr;
        if (nomeLmsh && BloccoLmsh()) h = CAnimManager::GetAnimation(nomeLmsh, bloccoLmsh);
        if (h) {
            if (AnimPresente(ped)) { anim->m_nFlags |= 0x4; anim->m_fBlendDelta = -10.0f; }
            CAnimManager::UncompressAnimation(h);
            anim = CAnimManager::BlendAnimation(ped->m_pRwClump, h, ANIMATION_STARTED | ANIMATION_LOOPED | ANIMATION_PARTIAL, 10.0f);
            if (!anim) return;
            anim->m_nFlags &= ~(ANIMATION_MOVEMENT | ANIMATION_TRANLSATE_X | ANIMATION_TRANLSATE_Y | 0x4);
            anim->m_fBlendAmount = 0.0f;
            anim->m_fBlendDelta = 10.0f;
            anim->m_fCurrentTime = 0.0f;
            if (durata > 0.0f && h->m_fTotalTime > 0.0f) anim->m_fSpeed = h->m_fTotalTime / durata;  // un ciclo per arco
            return;
        }
        if (AnimPresente(ped)) { anim->m_fBlendDelta = 8.0f; return; }
        anim = CAnimManager::AddAnimation(ped->m_pRwClump, 0, ANIM_APPESO);
        if (!anim) return;
        anim->m_nFlags |= ANIMATION_STARTED | ANIMATION_LOOPED | ANIMATION_PARTIAL;
        anim->m_nFlags &= ~(ANIMATION_MOVEMENT | ANIMATION_TRANLSATE_X | ANIMATION_TRANLSATE_Y | 0x4);
        anim->m_fBlendAmount = 0.0f;
        anim->m_fBlendDelta = 8.0f;
    }

    void FermaAnim(CPed* ped) {
        if (AnimPresente(ped)) { anim->m_nFlags |= 0x4; anim->m_fBlendDelta = -8.0f; }
        anim = nullptr;
    }

    // cerca un punto a cui attaccare la ragnatela: avanti (direzione della camera) e in alto
    bool CercaAncora(CPed* ped, CVector& punto) {
        CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
        CVector avanti(cam.m_vecFront.x, cam.m_vecFront.y, 0.0f);
        if (avanti.MagnitudeSqr() < 0.0001f) avanti = CVector(0.0f, 1.0f, 0.0f);
        avanti.Normalize();
        const float alzate[] = { 1.0f, 1.6f, 0.6f, 2.5f };   // rapporti alto/avanti provati in ordine
        CVector da = ped->GetPosition() + CVector(0.0f, 0.0f, 1.0f);
        for (float alza : alzate) {
            CVector dir = avanti + CVector(0.0f, 0.0f, alza);
            dir.Normalize();
            CVector a = da + dir * PORTATA;
            CColPoint cp; CEntity* e = nullptr;
            CEntity* ign = CWorld::pIgnoreEntity; CWorld::pIgnoreEntity = ped;
            bool preso = CWorld::ProcessLineOfSight(da, a, cp, e, true, false, false, true, false, false, false, false);
            CWorld::pIgnoreEntity = ign;
            if (preso && (cp.m_vecPoint - da).Magnitude() > 3.0f) { punto = cp.m_vecPoint; return true; }
        }
        // niente a cui attaccarsi (richiesta di Dario: senza questo è ingiocabile fuori dai palazzi):
        // la ragnatela si aggancia comunque a un punto in alto davanti, come nei giochi di Spider-Man
        punto = da + avanti * ANCORA_AVANTI + CVector(0.0f, 0.0f, ANCORA_ALTO);
        return true;
    }

    float DistanzaSuolo(CPed* ped) {
        CVector pos = ped->GetPosition();
        CColPoint p; CEntity* e = nullptr;
        if (CWorld::ProcessVerticalLine(pos, pos.z - 50.0f, p, e, true, true, false, true, false, false, nullptr))
            return pos.z - p.m_vecPoint.z;
        return 999.0f;
    }

    void Attacca(CPed* ped, const CVector& punto) {
        attaccato = true;
        arcoT = 0.0f;
        CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
        arcoDir = CVector(cam.m_vecFront.x, cam.m_vecFront.y, 0.0f);
        if (arcoDir.MagnitudeSqr() < 0.0001f) arcoDir = ped->GetMatrix().up;
        arcoDir.z = 0.0f; arcoDir.Normalize();
        // quanto si può scendere senza toccare terra
        float quota = DistanzaSuolo(ped);
        arcoDiscesa = std::clamp(quota - 1.6f, 0.3f, ARCO_DISCESA_MAX);
        // quota di crociera fra ~10 e ~25 m: sotto si sale di più, sopra si scende piano
        arcoRisalita = quota < 10.0f ? ARCO_RISALITA + 1.5f : (quota > 25.0f ? -1.5f : ARCO_RISALITA * 0.6f);
        // punto del filo: la struttura trovata, oppure in alto sopra la metà dell'arco
        CVector meta = ped->GetPosition() + arcoDir * (ARCO_LUNGHEZZA * 0.5f);
        ancora = (punto - ped->GetPosition()).Magnitude() < PORTATA ? punto : meta + CVector(0.0f, 0.0f, 16.0f);
        if (ancora.z < ped->GetPosition().z + 6.0f) ancora = meta + CVector(0.0f, 0.0f, 16.0f);
        ped->bApplyGravity = false;
        Proteggi(ped, true);
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);
        // come in LMSH1: braccio destro e sinistro alternati; versione "up" quando l'arco deve salire
        arcoNumero++;
        bool destra = (arcoNumero & 1) != 0;
        const char* nome = arcoRisalita > ARCO_RISALITA ? (destra ? "ws_up_r1" : "ws_up_l1") : (destra ? "ws_right1" : "ws_left1");
        AvviaAnim(ped, nome, ARCO_DURATA);
    }

    void Stacca(CPed* ped, bool protetto) {
        attaccato = false;
        cadutaProtetta = protetto;
        ped->bApplyGravity = true;
        ped->m_vecMoveSpeed = velocita / TICK;
        FermaAnim(ped);
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);
        fineProtezione = CTimer::m_snTimeInMilliseconds + PROTEZIONE;
    }


    // --- pareti -----------------------------------------------------------------------------------------
    bool SwimPronto() {
        int idx = CAnimManager::GetAnimationBlockIndex("swim");
        if (idx < 0) return false;
        if (!CAnimManager::ms_aAnimBlocks[idx].bLoaded) { CStreaming::RequestModel(ID_BASE_IFP + idx, 0x8); return false; }
        if (!rifSwim) { CAnimManager::AddAnimBlockRef(idx); rifSwim = true; }
        return true;
    }

    bool AnimPareteVisibile(CPed* ped) {
        if (!animParete || !ped->m_pRwClump) return false;
        for (CAnimBlendAssociation* x = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump); x; x = RpAnimBlendGetNextAssociation(x))
            if (x == animParete) return true;
        return false;
    }

    void AnimParete(CPed* ped, bool accesa, float velocitaAnim) {
        if (!accesa) {
            if (AnimPareteVisibile(ped)) { animParete->m_nFlags |= 0x4; animParete->m_fBlendDelta = -8.0f; }
            animParete = nullptr;
            return;
        }
        if (!SwimPronto() || !ped->m_pRwClump) return;
        if (!AnimPareteVisibile(ped)) {
            animParete = CAnimManager::AddAnimation(ped->m_pRwClump, ANIM_PARETE_GRUPPO, ANIM_PARETE);
            if (!animParete) return;
            animParete->m_nFlags |= ANIMATION_STARTED | ANIMATION_LOOPED | ANIMATION_PARTIAL;
            animParete->m_nFlags &= ~(ANIMATION_MOVEMENT | ANIMATION_TRANLSATE_X | ANIMATION_TRANLSATE_Y | 0x4);
            animParete->m_fBlendAmount = 0.0f;
            animParete->m_fBlendDelta = 8.0f;
        }
        animParete->m_fSpeed = velocitaAnim; // ferma quando Spider-Man è fermo sul muro
    }

    // raggio orizzontale dal petto: muro quasi verticale entro "dist"?
    bool TrovaParete(CPed* ped, const CVector& dir, float dist, CColPoint& cp) {
        CVector da = ped->GetPosition() + CVector(0.0f, 0.0f, 0.3f);
        CVector a = da + dir * dist;
        CEntity* e = nullptr;
        CEntity* ign = CWorld::pIgnoreEntity; CWorld::pIgnoreEntity = ped;
        bool preso = CWorld::ProcessLineOfSight(da, a, cp, e, true, false, false, true, false, false, false, false);
        CWorld::pIgnoreEntity = ign;
        return preso && std::fabs(cp.m_vecNormal.z) < 0.5f;
    }

    // orienta il pedone a pancia contro il muro (la nuotata di SA diventa un'arrampicata)
    void OrientaSuParete(CPed* ped, const CVector& avanti) {
        CMatrix& m = ped->GetMatrix();
        CVector at = normaleParete;
        CVector fw = avanti - at * (avanti.x * at.x + avanti.y * at.y + avanti.z * at.z);
        if (fw.MagnitudeSqr() < 1e-4f) fw = CVector(0.0f, 0.0f, 1.0f) - at * at.z;
        fw.Normalize();
        CVector right = CVector::Cross(fw, at);
        m.right = right; m.up = fw; m.at = at;
        ped->UpdateRwMatrix();
        ped->UpdateRwFrame();
    }

    void EntraParete(CPed* ped, const CColPoint& cp) {
        inParete = true;
        normaleParete = cp.m_vecNormal; normaleParete.z = 0.0f; normaleParete.Normalize();
        ped->bApplyGravity = false;
        // sul muro il movimento è "a binario" (posizione impostata ogni frame): con le collisioni del pedone
        // attive restava incastrato negli spigoli e nei davanzali (prova di Dario del 9/11)
        ped->bUsesCollision = false;
        ped->SetPosition(CVector(cp.m_vecPoint.x, cp.m_vecPoint.y, ped->GetPosition().z) + normaleParete * DIST_PARETE);
        ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
        Proteggi(ped, true);
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);
        SwimPronto();
    }

    void EsciParete(CPed* ped, const CVector& spinta) {
        inParete = false;
        fineParete = CTimer::m_snTimeInMilliseconds + PAUSA_PARETE;
        AnimParete(ped, false, 0.0f);
        ped->bApplyGravity = true;
        ped->bUsesCollision = true;
        // di nuovo dritto
        float h = std::atan2(normaleParete.x, -normaleParete.y); // guarda verso il muro
        CMatrix& m = ped->GetMatrix();
        m.right = CVector(std::cos(h), std::sin(h), 0.0f);
        m.up = CVector(-std::sin(h), std::cos(h), 0.0f);
        m.at = CVector(0.0f, 0.0f, 1.0f);
        ped->m_fHeadingCurrent = h; ped->m_fHeadingGoal = h;
        ped->UpdateRwMatrix(); ped->UpdateRwFrame();
        ped->m_vecMoveSpeed = spinta / TICK;
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);
        cadutaProtetta = true;
        fineProtezione = CTimer::m_snTimeInMilliseconds + PROTEZIONE;
    }

    // cerca il muro dietro alla pancia di Spider-Man a varie altezze (davanzali, finestre, cornici)
    bool MuroQui(CPed* ped, CColPoint& cp) {
        const float quote[] = { 0.0f, 0.4f, -0.3f, 0.8f };
        CVector base = ped->GetPosition();
        for (float q : quote) {
            CVector da = base + CVector(0.0f, 0.0f, q) + normaleParete * 0.3f;
            CVector a = da - normaleParete * 1.8f;
            CEntity* e = nullptr;
            CEntity* ign = CWorld::pIgnoreEntity; CWorld::pIgnoreEntity = ped;
            bool preso = CWorld::ProcessLineOfSight(da, a, cp, e, true, false, false, true, false, false, false, false);
            CWorld::pIgnoreEntity = ign;
            if (preso && std::fabs(cp.m_vecNormal.z) < 0.6f) { cp.m_vecPoint.z -= q; return true; }
        }
        return false;
    }

    void AggiornaParete(CPed* ped, CPad* pad) {
        const float dt = std::max(0.001f, CTimer::ms_fTimeStep / TICK);
        if (pad->JumpJustDown()) { EsciParete(ped, normaleParete * 7.0f + CVector(0.0f, 0.0f, 6.0f)); return; }
        if (pad->DuckJustDown()) { EsciParete(ped, normaleParete * 1.5f); return; }

        CVector su(0.0f, 0.0f, 1.0f);
        CVector destra = CVector::Cross(su, normaleParete);   // destra di chi guarda il muro
        float ingSu = std::clamp(-pad->GetPedWalkUpDown() / 128.0f, -1.0f, 1.0f);
        float ingLato = std::clamp(pad->GetPedWalkLeftRight() / 128.0f, -1.0f, 1.0f);
        float vel = pad->GetSprint() ? VEL_PARETE_SCATTO : VEL_PARETE;
        CVector pos = ped->GetPosition();
        CVector prossima = pos + (su * ingSu + destra * ingLato) * (vel * dt);

        // il muro c'è ancora alla nuova posizione?
        CMatrix& m = ped->GetMatrix();
        m.pos = prossima;
        CColPoint cp;
        if (MuroQui(ped, cp)) {
            CVector n = cp.m_vecNormal; n.z = 0.0f;
            if (n.MagnitudeSqr() > 0.01f) { n.Normalize(); normaleParete = n; }
            // distanza costante dal muro
            CVector sulMuro = cp.m_vecPoint + normaleParete * DIST_PARETE;
            prossima.x = sulMuro.x; prossima.y = sulMuro.y;
        } else if (ingSu > 0.1f) {
            m.pos = pos;
            // finito il muro salendo: si cerca il tetto appena oltre il bordo, non più in alto di 2 m
            CVector sopra = pos + CVector(0.0f, 0.0f, 2.2f) - normaleParete * 0.9f;
            CColPoint tetto; CEntity* e = nullptr;
            if (CWorld::ProcessVerticalLine(sopra, pos.z - 0.8f, tetto, e, true, false, false, true, false, false, nullptr)
                && tetto.m_vecPoint.z > pos.z - 0.8f) {
                ped->SetPosition(tetto.m_vecPoint + CVector(0.0f, 0.0f, 1.0f));
                EsciParete(ped, CVector(0.0f, 0.0f, 0.0f));
                return;
            }
            EsciParete(ped, normaleParete * -1.5f + CVector(0.0f, 0.0f, 4.0f)); // salto oltre il bordo
            return;
        } else {
            m.pos = pos;
            EsciParete(ped, normaleParete * 1.0f);
            return;
        }
        // a terra scendendo: torna a camminare
        if (ingSu < -0.1f && DistanzaSuolo(ped) < 1.1f) { m.pos = pos; EsciParete(ped, normaleParete * 1.0f); return; }

        ped->SetPosition(prossima);
        ped->bApplyGravity = false;
        ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
        float intensita = std::min(1.0f, std::sqrt(ingSu * ingSu + ingLato * ingLato));
        CVector direzione = intensita > 0.1f ? (su * ingSu + destra * ingLato) : su;
        ultimaDirezione = direzione;
        OrientaSuParete(ped, direzione);
        AnimParete(ped, true, intensita > 0.1f ? (pad->GetSprint() ? 1.6f : 1.0f) : 0.0f);
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);
    }

    void Aggiorna() {
        CPlayerPed* ped = FindPlayerPed();
        if (!ped) return;
        CPad* pad = CPad::GetPad(0);
        if (!pad) return;
        const unsigned int ora = CTimer::m_snTimeInMilliseconds;
        bool eSpidey = ped->m_nModelIndex == IdSpidey();

        if (!eSpidey || ped->m_fHealth <= 0.0f || ped->bInVehicle) {
            if (inParete) EsciParete(ped, CVector(0.0f, 0.0f, 0.0f));
            if (attaccato) Stacca(ped, false);
            if (protezioneMessa && ora > fineProtezione) Proteggi(ped, false);
            return;
        }

        if (inParete) { AggiornaParete(ped, pad); return; }

        // attaccarsi a un muro: camminando (o in volo/dondolo) contro una parete quasi verticale
        {
            float ingAv = -pad->GetPedWalkUpDown() / 128.0f;
            CVector dirPed = ped->GetMatrix().up; dirPed.z = 0.0f;
            if (ingAv > 0.5f && dirPed.MagnitudeSqr() > 0.01f && ora > fineParete) {
                dirPed.Normalize();
                CColPoint cp;
                if (TrovaParete(ped, dirPed, 0.9f, cp)) {
                    if (attaccato) { attaccato = false; FermaAnim(ped); }
                    EntraParete(ped, cp);
                    return;
                }
            }
        }

        if (!attaccato) {
            // dopo un lancio si resta protetti dai danni da caduta fino a terra (+ un attimo)
            if (cadutaProtetta) {
                if (ped->bIsStanding) { cadutaProtetta = false; fineProtezione = ora + PROTEZIONE; }
            } else if (protezioneMessa && ora > fineProtezione) {
                Proteggi(ped, false);
            }
            if ((ped->bIsInTheAir || !ped->bIsStanding) && pad->JumpJustDown()) {
                CVector punto;
                if (CercaAncora(ped, punto)) Attacca(ped, punto);
            }
            return;
        }

        // --- arco del dondolo ---
        const float dt = std::max(0.001f, CTimer::ms_fTimeStep / TICK);
        if (pad->DuckJustDown()) {
            if (ultimoAccovaccia && ora - ultimoAccovaccia <= DOPPIO_TOCCO) { ultimoAccovaccia = 0; Stacca(ped, true); return; }
            ultimoAccovaccia = ora;
        }
        // nuovo SALTO a metà arco: ragnatela successiva subito
        if (pad->JumpJustDown() && arcoT > 0.25f) {
            velocita = ped->m_vecMoveSpeed * TICK;
            CVector punto; CercaAncora(ped, punto); Attacca(ped, punto);
        }

        float ingLato = std::clamp(pad->GetPedWalkLeftRight() / 128.0f, -1.0f, 1.0f);
        float giro = -ingLato * CURVA * dt;
        arcoDir = CVector(arcoDir.x * std::cos(giro) - arcoDir.y * std::sin(giro),
                          arcoDir.x * std::sin(giro) + arcoDir.y * std::cos(giro), 0.0f);

        arcoT += dt;
        float u = std::min(1.0f, arcoT / ARCO_DURATA);
        const float PI_F = 3.14159265f;
        float vOrizz = ARCO_LUNGHEZZA / ARCO_DURATA;
        float vz = (-arcoDiscesa * PI_F * std::cos(PI_F * u) + 2.0f * arcoRisalita * u) / ARCO_DURATA;
        velocita = arcoDir * vOrizz + CVector(0.0f, 0.0f, vz);

        // fine arco. Come nel hub di LMSH1 (abilità MarvelWebSwing, animazioni webswing_hub/_up/_fallswing):
        // tenendo AVANTI si continua a dondolare da soli, un arco dopo l'altro, mantenendo una quota di
        // crociera; lasciando AVANTI si viene lanciati in avanti e in alto (poi SALTO in aria = nuova ragnatela)
        if (u >= 1.0f) {
            if (-pad->GetPedWalkUpDown() / 128.0f > 0.3f) {
                velocita = ped->m_vecMoveSpeed * TICK;
                CVector punto; CercaAncora(ped, punto); Attacca(ped, punto);
            } else {
                Stacca(ped, true);
            }
            return;
        }

        // muro davanti: ci si attacca se si spinge avanti, altrimenti ci si ferma contro
        CColPoint cp;
        if (TrovaParete(ped, arcoDir, 1.4f, cp)) {
            if (-pad->GetPedWalkUpDown() / 128.0f > 0.3f) { attaccato = false; FermaAnim(ped); EntraParete(ped, cp); return; }
            velocita = CVector(0.0f, 0.0f, 0.0f); Stacca(ped, true); return;
        }
        // vicino a terra mentre scende: atterra
        if (vz < 0.0f && DistanzaSuolo(ped) < 1.0f) { Stacca(ped, true); return; }

        ped->bApplyGravity = false;
        ped->m_vecMoveSpeed = velocita / TICK;
        float angolo = std::atan2(-arcoDir.x, arcoDir.y);
        ped->m_fHeadingCurrent = angolo; ped->m_fHeadingGoal = angolo; ped->SetHeading(angolo);
        AvviaAnim(ped);
        if (ped->m_pIntelligence) ped->m_pIntelligence->FlushImmediately(true);

        RwV3d mano = { 0.0f, 0.0f, 0.0f };
        ped->GetBonePosition(mano, BONE_MANO_DESTRA, true);
        filoDa = CVector(mano.x, mano.y, mano.z);
        filoA = ancora;
        disegnaFilo = true;
    }

    void Disegna() {
        if (!disegnaFilo) return;
        disegnaFilo = false;
        if (!mg::IniziaStrisce(false)) return;
        mg::Striscia(filoDa, filoA, 0.05f, 235, 235, 235, 255);
        mg::FineStrisce();
    }
} moduloRagnatela;

} // namespace
