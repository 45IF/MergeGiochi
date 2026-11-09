// MergeGiochi — Modulo Abilità speciali
// Scritto da Claude. plugin-sdk, GTA SA 1.0 US.
//
// Un unico tasto per l'abilità speciale di ogni personaggio (richiesta di Dario del 9/11):
//   - tasto X tenuto premuto -> abilità del personaggio attuale
//
// Iron Man (modello "ironman"): raggio repulsore dalla mano destra verso il centro della camera.
// "Fonde" ciò che colpisce, tranne il suolo e l'acqua:
//   - pedoni: prendono fuoco subito;
//   - veicoli: dopo un attimo di raggio esplodono;
//   - oggetti: si rompono; se non si rompono, dopo un attimo si fondono (spariscono con fuoco);
//   - edifici e arredi: dopo un po' di raggio si fondono (spariscono, con il loro LOD), ma NON il suolo:
//     una superficie colpita con la normale rivolta in su (pavimento, strada, terreno, tetto piano) è esclusa.
//   - l'acqua in SA non ha collisione, quindi il raggio la attraversa senza effetti.
// Effetti grafici solo originali di SA: il raggio è una striscia luminosa disegnata in 3D con la texture
// originale "shad_exp" (gradiente radiale di SA), in due strati (alone azzurro + nucleo bianco), più due
// coronas originali alla mano e al punto d'impatto; fuoco con CFireManager.
// Quando avremo gli effetti del repulsore di LMSH1 convertiti, si cambia solo DisegnaRaggio().

#include "plugin.h"
#include "common.h"
#include "CPed.h"
#include "CPlayerPed.h"
#include "CCamera.h"
#include "CTimer.h"
#include "CWorld.h"
#include "CColPoint.h"
#include "CModelInfo.h"
#include "CVehicle.h"
#include "CObject.h"
#include "CFireManager.h"
#include "CWeapon.h"
#include "CPools.h"
#include "CStreaming.h"
#include "CAnimManager.h"
#include "CAnimBlendAssociation.h"
#include "CShadows.h"
#include "../comune/Grafica.h"
#include "CCoronas.h"
#include "CCamera.h"
#include <cmath>
#include <cstring>

using namespace plugin;

namespace {

constexpr int TASTO_ABILITA = 'X';
constexpr int BONE_MANO_DESTRA = 24;   // " R Hand" nello scheletro dei pedoni (25 è il dito)
constexpr float PORTATA = 150.0f;           // m
constexpr float SOGLIA_SUOLO = 0.7f;        // normale.z oltre cui la superficie è "suolo"
constexpr float TEMPO_VEICOLO = 1.2f;       // s di raggio per far esplodere un veicolo
constexpr float TEMPO_OGGETTO = 0.8f;       // s per fondere un oggetto
constexpr float TEMPO_EDIFICIO = 1.5f;      // s per fondere un edificio/arredo
constexpr unsigned int INTERVALLO_FUOCO = 250; // ms fra un fuoco e l'altro nel punto colpito

// CFireManager::StartFire(CVector punto, ...) a 0x539F00 riceve il CVector PER VALORE (12 byte sullo stack).
// Con MinGW il wrapper del plugin-sdk lo passava per riferimento (CVector non è banalmente copiabile per GCC):
// stack sbilanciato e crash a EIP=0x3F4CCCCD (= 0.8f) il 9/11. Qui lo chiamiamo passando x, y, z come float.
CFire* FuocoInPunto(const CVector& p, float dimensione, CEntity* creatore, unsigned int durata) {
    using Fn = CFire* (__thiscall*)(CFireManager*, float, float, float, float, unsigned char, CEntity*, unsigned int, signed char, unsigned char);
    return reinterpret_cast<Fn>(0x539F00)(&gFireManager, p.x, p.y, p.z, dimensione, 1, creatore, durata, 0, 1);
}

class ModuloAbilita {
public:
    ModuloAbilita() {
        const auto v = GetGameVersion();
        if (v != GAME_10US_COMPACT && v != GAME_10US_HOODLUM)
            return;
        Events::gameProcessEvent += [this] { Aggiorna(); };
        Events::renderEffectsEvent += [this] { Disegna(); };
    }

private:
    int idIronMan = -2;          // -2 = non ancora cercato, -1 = non trovato
    CEntity* bersaglio = nullptr;
    float calore = 0.0f;         // secondi di raggio accumulati sul bersaglio attuale
    unsigned int prossimoFuoco = 0;

    int IdIronMan() {
        if (idIronMan == -2) {
            int id = -1;
            CModelInfo::GetModelInfo("ironman", &id);
            idIronMan = id;
        }
        return idIronMan;
    }

    // stato del raggio da disegnare (calcolato in Aggiorna, disegnato in Disegna)
    bool raggioAttivo = false;
    CVector raggioDa, raggioA;
    bool raggioImpatto = false;

    void DisegnaRaggio(CVector da, CVector a, bool impatto) {
        raggioAttivo = true; raggioDa = da; raggioA = a; raggioImpatto = impatto;
    }

    // striscia rivolta verso la camera fra da e a, larga 2*meta
    static void Striscia(const CVector& da, const CVector& a, float meta, RwUInt8 r, RwUInt8 g, RwUInt8 b, RwUInt8 alfa) {
        CVector dir = a - da;
        CVector centro = (da + a) * 0.5f;
        CVector versoCam = centro - TheCamera.GetPosition();
        CVector lato = CVector::Cross(dir, versoCam);
        if (lato.MagnitudeSqr() < 1e-6f) return;
        lato.Normalize();
        lato *= meta;
        static RwIm3DVertex v[4];
        static RwImVertexIndex indici[6] = { 0, 1, 2, 1, 3, 2 };
        const CVector p[4] = { da + lato, da - lato, a + lato, a - lato };
        const float u[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
        for (int i = 0; i < 4; i++) {
            RwIm3DVertexSetPos(&v[i], p[i].x, p[i].y, p[i].z);
            RwIm3DVertexSetRGBA(&v[i], r, g, b, alfa);
            RwIm3DVertexSetU(&v[i], u[i]);
            RwIm3DVertexSetV(&v[i], 0.5f);
        }
        if (RwIm3DTransform(v, 4, nullptr, rwIM3D_VERTEXUV | rwIM3D_VERTEXRGBA)) {
            RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indici, 6);
            RwIm3DEnd();
        }
    }

    void Disegna() {
        DisegnaScudo();
        if (!raggioAttivo) return;
        raggioAttivo = false; // va ridisegnato solo se Aggiorna lo richiede di nuovo
        RwTexture* tex = gpShadowExplosionTex;
        if (!tex) return;

        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE); // additivo: il raggio "brilla"
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)RwTextureGetRaster(tex));

        // raggio "massiccio" come il repulsore di LMSH1: tubo azzurro largo + tubo chiaro + nucleo bianco,
        // più strisce rivolte alla camera; spessore che pulsa
        float t = CTimer::m_snTimeInMilliseconds * 0.025f;
        float tremolio = 1.0f + 0.18f * std::sin(t) + 0.08f * std::sin(t * 2.7f);
        mg::Cilindro(raggioDa, raggioA, 0.45f * tremolio, 30, 110, 255, 70, 10);   // alone esterno
        mg::Cilindro(raggioDa, raggioA, 0.25f * tremolio, 90, 190, 255, 140, 10);  // fascia
        mg::Cilindro(raggioDa, raggioA, 0.10f, 255, 255, 255, 255, 8);             // nucleo bianco
        Striscia(raggioDa, raggioA, 0.70f * tremolio, 40, 140, 255, 160);
        Striscia(raggioDa, raggioA, 0.30f * tremolio, 160, 220, 255, 255);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)nullptr);
    }

    void Bagliori() {
        // coronas originali (stella) alla mano e all'impatto; id fissi presi dall'indirizzo di questo oggetto
        unsigned int base = reinterpret_cast<unsigned int>(this);
        CCoronas::RegisterCorona(base + 1, nullptr, 120, 200, 255, 255, raggioDa, 1.2f, 200.0f, CORONATYPE_SHINYSTAR,
                                 FLARETYPE_NONE, false, false, 0, 0.0f, false, 0.1f, 0, 15.0f, false, false);
        // bagliori lungo tutto il raggio: sono sprite a schermo, quindi il raggio resta ben visibile anche
        // quando lo si guarda quasi di punta da dietro Iron Man
        CVector passo = (raggioA - raggioDa) * (1.0f / 24.0f);
        for (int i = 1; i < 24; i++)
            CCoronas::RegisterCorona(base + 10 + i, nullptr, 70, 160, 255, 200, raggioDa + passo * (float)i, 0.9f, 200.0f,
                                     CORONATYPE_SHINYSTAR, FLARETYPE_NONE, false, false, 0, 0.0f, false, 0.1f, 0, 30.0f, false, false);
        if (raggioImpatto)
            CCoronas::RegisterCorona(base + 2, nullptr, 160, 220, 255, 255, raggioA, 3.0f, 200.0f, CORONATYPE_SHINYSTAR,
                                     FLARETYPE_NONE, false, false, 0, 0.0f, false, 0.1f, 0, 15.0f, false, false);
    }

    void Fondi(CEntity* e, const CVector& punto, CPed* giocatore) {
        FuocoInPunto(punto, 1.0f, giocatore, 3000);
        e->bIsVisible = false;
        e->bUsesCollision = false;
        if (e->m_pLod) {
            e->m_pLod->bIsVisible = false;
            e->m_pLod->bUsesCollision = false;
        }
    }

    void Colpisci(CEntity* e, const CColPoint& punto, CPed* giocatore, float dt) {
        const CVector& p = punto.m_vecPoint;
        unsigned int ora = CTimer::m_snTimeInMilliseconds;
        bool fuoco = ora >= prossimoFuoco;

        if (e != bersaglio) { bersaglio = e; calore = 0.0f; }
        calore += dt;

        switch (e->m_nType) {
        case ENTITY_TYPE_PED: {
            CPed* ped = static_cast<CPed*>(e);
            if (!ped->m_pFire)
                gFireManager.StartFire(ped, giocatore, 0.8f, 1, 7000, 0);
            break;
        }
        case ENTITY_TYPE_VEHICLE: {
            CVehicle* veicolo = static_cast<CVehicle*>(e);
            if (fuoco) FuocoInPunto(p, 0.8f, giocatore, 1500);
            if (calore >= TEMPO_VEICOLO && veicolo->m_fHealth > 0.0f) {
                veicolo->BlowUpCar(giocatore, false);
                calore = -100.0f; // una volta sola
            }
            break;
        }
        case ENTITY_TYPE_OBJECT: {
            CObject* oggetto = static_cast<CObject*>(e);
            CVector dir = p - giocatore->GetPosition();
            oggetto->ObjectDamage(2000.0f * dt, const_cast<CVector*>(&p), &dir, giocatore, WEAPONTYPE_FTHROWER);
            if (calore >= TEMPO_OGGETTO && oggetto->bIsVisible) Fondi(e, p, giocatore);
            else if (fuoco) FuocoInPunto(p, 0.6f, giocatore, 1200);
            break;
        }
        case ENTITY_TYPE_BUILDING:
        case ENTITY_TYPE_DUMMY:
            if (punto.m_vecNormal.z > SOGLIA_SUOLO) { calore = 0.0f; break; } // suolo: escluso
            if (fuoco) FuocoInPunto(p, 0.6f, giocatore, 1200);
            if (calore >= TEMPO_EDIFICIO) Fondi(e, p, giocatore);
            break;
        default:
            break;
        }
        if (fuoco) prossimoFuoco = ora + INTERVALLO_FUOCO;
    }


    // ---------------------------------------------------------------- colpi corpo a corpo (Wolverine, Hulk)
    static int IdModello(const char* nome) { int id = -1; CModelInfo::GetModelInfo(nome, &id); return id; }
    bool xPrima = false;
    unsigned int prontoDi = 0;   // ms: fine del tempo di ricarica

    // breve animazione originale di SA come parziale "una volta sola"
    static void Mossa(CPed* ped, int idAnim, float velocita) {
        if (!ped->m_pRwClump) return;
        CAnimBlendAssociation* a = CAnimManager::AddAnimation(ped->m_pRwClump, 0, idAnim);
        if (!a) return;
        a->m_nFlags |= ANIMATION_STARTED | ANIMATION_PARTIAL | 0x8; // 0x8: si rimuove da sola alla fine
        a->m_nFlags &= ~(ANIMATION_LOOPED | ANIMATION_MOVEMENT | ANIMATION_TRANLSATE_X | ANIMATION_TRANLSATE_Y);
        a->m_fBlendAmount = 1.0f; a->m_fBlendDelta = 0.0f; a->m_fSpeed = velocita; a->m_fCurrentTime = 0.0f;
    }

    // colpisce tutto ciò che sta davanti entro "raggio" (cono: prodotto scalare > minDot)
    // forza = m/s di spinta; danno per pedoni; dannoVeicolo/dannoOggetto
    void Colpo(CPed* ped, float raggio, float minDot, int danno, eWeaponType arma, float forza,
               float dannoVeicolo, float dannoOggetto) {
        CVector pos = ped->GetPosition();
        CVector avanti = ped->GetMatrix().up; avanti.z = 0.0f; avanti.Normalize();
        auto davanti = [&](CEntity* e, CVector& dir) {
            dir = e->GetPosition() - pos;
            float d = dir.Magnitude();
            if (d > raggio || d < 0.01f) return false;
            dir *= 1.0f / d;
            return d < 1.2f || (dir.x * avanti.x + dir.y * avanti.y) > minDot;
        };
        CVector dir;
        for (int i = 0; i < CPools::ms_pPedPool->m_nSize; i++) {
            CPed* p = CPools::ms_pPedPool->GetAt(i);
            if (!p || p == ped || p->m_fHealth <= 0.0f || !davanti(p, dir)) continue;
            CWeapon::GenerateDamageEvent(p, ped, arma, danno, (ePedPieceTypes)3, 0);
            if (forza > 0.0f) p->m_vecMoveSpeed += (dir + CVector(0.0f, 0.0f, 0.5f)) * (forza / 50.0f);
        }
        for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
            CVehicle* v = CPools::ms_pVehiclePool->GetAt(i);
            if (!v || !davanti(v, dir)) continue;
            v->m_fHealth -= dannoVeicolo;
            if (forza > 0.0f) {
                v->m_vecMoveSpeed += (dir + CVector(0.0f, 0.0f, 0.6f)) * (forza * 0.6f / 50.0f);
                v->m_vecTurnSpeed += CVector(0.0f, 0.0f, 0.05f);
            }
            if (v->m_fHealth <= 0.0f) v->BlowUpCar(ped, false);
        }
        for (int i = 0; i < CPools::ms_pObjectPool->m_nSize; i++) {
            CObject* o = CPools::ms_pObjectPool->GetAt(i);
            if (!o || !davanti(o, dir)) continue;
            CVector pp = o->GetPosition();
            o->ObjectDamage(dannoOggetto, &pp, &dir, ped, arma);
            if (forza > 0.0f) o->m_vecMoveSpeed += (dir + CVector(0.0f, 0.0f, 0.5f)) * (forza / 50.0f);
        }
    }

    void Wolverine(CPed* ped, bool premuto) {
        // artigli: colpo rapido a ripetizione (ricarica breve)
        unsigned int ora = CTimer::m_snTimeInMilliseconds;
        if (!premuto || ora < prontoDi) return;
        prontoDi = ora + 320;
        Mossa(ped, 103, 1.6f);   // Bike_ElbowR: fendente del braccio destro (animazione originale di SA)
        Colpo(ped, 2.8f, 0.35f, 45, WEAPONTYPE_KNIFE, 2.0f, 150.0f, 600.0f);
    }

    void Hulk(CPed* ped, bool premuto) {
        // pugno che scaraventa: persone, auto e oggetti davanti vengono lanciati via
        unsigned int ora = CTimer::m_snTimeInMilliseconds;
        if (!premuto || ora < prontoDi) return;
        prontoDi = ora + 900;
        Mossa(ped, 103, 1.2f);
        Colpo(ped, 7.0f, 0.2f, 80, WEAPONTYPE_UNARMED, 22.0f, 350.0f, 2000.0f);
        TheCamera.CamShake(0.35f, ped->GetPosition().x, ped->GetPosition().y, ped->GetPosition().z);
    }


    // ---------------------------------------------------------------- Captain America: lancio dello scudo
    // Lo scudo in volo è il modello "mgscudo" (originale di LMSH1, AMERICASHIELD_NXG, convertito con
    // tools/converti_oggetto.py e registrato nella sezione objs di peds.ide). NON è un oggetto del mondo
    // (CObject): senza collisione il gioco crashava a 0x59F8B4 creandolo (prova del 9/11). Lo si disegna
    // direttamente: si prende l'atomico del modello, gli si dà la matrice e lo si renderizza.
    // Mentre vola, lo scudo che Cap ha in mano (texture "capscudo") diventa trasparente.
    bool scudo = false;
    int faseScudo = 0;            // 0 in mano, 1 andata, 2 ritorno
    CVector posScudo, metaScudo;
    float giroScudo = 0.0f;
    CEntity* giaColpiti[16]; int nColpiti = 0;

    static constexpr int BONE_MANO_SINISTRA = 34;   // " L Hand"
    static constexpr float VEL_SCUDO = 45.0f;
    static constexpr float PORTATA_SCUDO = 80.0f;

    CVector ManoSinistra(CPed* ped) {
        RwV3d m = { 0.0f, 0.0f, 0.0f };
        ped->GetBonePosition(m, BONE_MANO_SINISTRA, true);
        return CVector(m.x, m.y, m.z);
    }

    RpAtomic* atomicoScudo = nullptr;   // copia privata dell'atomico del modello, con un suo frame
    RpAtomic* AtomicoScudo() {
        if (atomicoScudo) return atomicoScudo;
        int id = IdModello("mgscudo");
        if (id < 0) return nullptr;
        if (CStreaming::ms_aInfoForModel[id].m_nLoadState != LOADSTATE_LOADED) {
            CStreaming::RequestModel(id, GAME_REQUIRED | KEEP_IN_MEMORY);
            CStreaming::LoadAllRequestedModels(false);
        }
        CBaseModelInfo* mi = CModelInfo::ms_modelInfoPtrs[id];
        RpAtomic* base = mi ? reinterpret_cast<RpAtomic*>(mi->m_pRwObject) : nullptr;
        if (!base) return nullptr;
        // l'atomico del modello non ha un frame utilizzabile (lo scudo non si vedeva, prova del 9/11):
        // se ne fa una copia con un frame nuovo
        atomicoScudo = RpAtomicClone(base);
        if (atomicoScudo) RpAtomicSetFrame(atomicoScudo, RwFrameCreate());
        return atomicoScudo;
    }

    void DisegnaScudo() {
        if (!scudo) return;
        RpAtomic* a = AtomicoScudo();
        if (!a) return;
        RwFrame* f = RpAtomicGetFrame(a);
        if (!f) return;
        RwMatrix* m = RwFrameGetMatrix(f);
        // disco "a frisbee": la normale del disco (asse Y del modello) in verticale, che ruota
        m->right = { std::cos(giroScudo), std::sin(giroScudo), 0.0f };
        m->up    = { 0.0f, 0.0f, 1.0f };
        m->at    = { std::sin(giroScudo), -std::cos(giroScudo), 0.0f };
        m->pos   = { posScudo.x, posScudo.y, posScudo.z };
        RwMatrixUpdate(m);
        RwFrameUpdateObjects(f);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
        SetAmbientColours();
        ActivateDirectional();
        a->renderCallBack(a);
        DeActivateDirectional();
    }

    void Lancia(CPed* ped) {
        if (!AtomicoScudo()) return;
        scudo = true;
        // bersaglio: centro dello schermo (fino a 35 m)
        CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
        CVector fine = cam.m_vecSource + cam.m_vecFront * (PORTATA_SCUDO + 10.0f);
        CColPoint cp; CEntity* e = nullptr;
        CEntity* ign = CWorld::pIgnoreEntity; CWorld::pIgnoreEntity = ped;
        if (CWorld::ProcessLineOfSight(cam.m_vecSource, fine, cp, e, true, true, true, true, false, false, false, true)) fine = cp.m_vecPoint;
        CWorld::pIgnoreEntity = ign;
        posScudo = ManoSinistra(ped);
        CVector d = fine - posScudo;
        if (d.Magnitude() > PORTATA_SCUDO) { d.Normalize(); fine = posScudo + d * PORTATA_SCUDO; }
        metaScudo = fine;
        faseScudo = 1; nColpiti = 0;
        Mossa(ped, 103, 1.4f);
    }

    void ColpisciConScudo(CPed* ped) {
        auto gia = [&](CEntity* e) { for (int i = 0; i < nColpiti; i++) if (giaColpiti[i] == e) return true; return false; };
        auto segna = [&](CEntity* e) { if (nColpiti < 16) giaColpiti[nColpiti++] = e; };
        for (int i = 0; i < CPools::ms_pPedPool->m_nSize; i++) {
            CPed* p = CPools::ms_pPedPool->GetAt(i);
            if (!p || p == ped || p->m_fHealth <= 0.0f || gia(p)) continue;
            CVector d = p->GetPosition() - posScudo;
            if (std::fabs(d.z) < 1.2f && d.Magnitude2D() < 1.0f) {
                CWeapon::GenerateDamageEvent(p, ped, WEAPONTYPE_UNARMED, 40, (ePedPieceTypes)3, 0);
                CVector dir = metaScudo - posScudo; dir.Normalize();
                p->m_vecMoveSpeed += (dir + CVector(0.0f, 0.0f, 0.4f)) * (8.0f / 50.0f);
                segna(p);
            }
        }
        for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
            CVehicle* v = CPools::ms_pVehiclePool->GetAt(i);
            if (!v || gia(v)) continue;
            if ((v->GetPosition() - posScudo).Magnitude() < 2.5f) { v->m_fHealth -= 120.0f; segna(v); }
        }
    }

    void AggiornaScudo(CPed* ped) {
        if (faseScudo == 0 || !scudo) return;
        const float dt = CTimer::ms_fTimeStep / 50.0f;
        giroScudo += 25.0f * dt;
        CVector meta = faseScudo == 1 ? metaScudo : ManoSinistra(ped);
        CVector d = meta - posScudo;
        float dist = d.Magnitude();
        float passo = (faseScudo == 1 ? VEL_SCUDO : VEL_SCUDO * 1.15f) * dt;
        if (dist <= passo + 0.3f) {
            if (faseScudo == 1) { faseScudo = 2; nColpiti = 0; }
            else { scudo = false; faseScudo = 0; return; }
        } else {
            CVector prossima = posScudo + d * (passo / dist);
            // contro un muro all'andata: rimbalza indietro
            if (faseScudo == 1) {
                CColPoint cp; CEntity* e = nullptr;
                if (CWorld::ProcessLineOfSight(posScudo, prossima, cp, e, true, false, false, false, false, false, false, false)) {
                    prossima = cp.m_vecPoint; faseScudo = 2; nColpiti = 0;
                }
            }
            posScudo = prossima;
        }
        ColpisciConScudo(ped);
    }

    // trasparenza dello scudo in mano mentre l'altro vola
    static RpMaterial* AlfaMateriale(RpMaterial* m, void* dati) {
        RwTexture* t = m->texture;
        if (t && std::strcmp(t->name, "capscudo") == 0) m->color.alpha = *static_cast<RwUInt8*>(dati);
        return m;
    }
    static RpAtomic* AlfaAtomico(RpAtomic* a, void* dati) {
        RpGeometryForAllMaterials(a->geometry, AlfaMateriale, dati);
        return a;
    }
    void ScudoInMano(CPed* ped) {
        if (!ped->m_pRwClump) return;
        RwUInt8 alfa = faseScudo ? 0 : 255;
        RpClumpForAllAtomics(ped->m_pRwClump, AlfaAtomico, &alfa);
    }

    void Aggiorna() {
        CPlayerPed* ped = FindPlayerPed();
        if (!ped || ped->bInVehicle || ped->m_fHealth <= 0.0f) return;
        bool x = KeyPressed(TASTO_ABILITA);
        bool premuto = x && !xPrima;
        xPrima = x;
        static int idWolvie = -2, idHulk = -2;
        if (idWolvie == -2) { idWolvie = IdModello("wolvie"); idHulk = IdModello("hulk"); }
        if (ped->m_nModelIndex == idWolvie) { Wolverine(ped, x); return; }   // tenendo premuto: raffica di fendenti
        if (ped->m_nModelIndex == idHulk) { Hulk(ped, premuto); return; }
        static int idCap = -2;
        if (idCap == -2) idCap = IdModello("capam");
        if (faseScudo && (ped->m_nModelIndex != idCap)) { scudo = false; faseScudo = 0; }
        if (ped->m_nModelIndex == idCap) {
            if (premuto && faseScudo == 0) Lancia(ped);
            AggiornaScudo(ped);
            ScudoInMano(ped);
            return;
        }
        (void)premuto;
        if (!x) { bersaglio = nullptr; calore = 0.0f; return; }
        if (ped->m_nModelIndex != IdIronMan()) return;

        const float dt = CTimer::ms_fTimeStep / 50.0f;

        // mira: raggio dalla camera al centro dello schermo
        CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
        CVector origineCam = cam.m_vecSource;
        CVector fine = origineCam + cam.m_vecFront * PORTATA;
        CColPoint punto;
        CEntity* colpita = nullptr;
        CEntity* ignoraPrima = CWorld::pIgnoreEntity;
        CWorld::pIgnoreEntity = ped;
        bool preso = CWorld::ProcessLineOfSight(origineCam, fine, punto, colpita,
                                                true, true, true, true, true, false, false, true);
        CWorld::pIgnoreEntity = ignoraPrima;
        CVector bersaglioPunto = preso ? punto.m_vecPoint : fine;

        // il personaggio si gira verso il punto mirato
        CVector verso = bersaglioPunto - ped->GetPosition();
        if (verso.Magnitude2D() > 0.5f) {
            float angolo = std::atan2(-verso.x, verso.y);
            ped->m_fHeadingCurrent = angolo;
            ped->m_fHeadingGoal = angolo;
            ped->SetHeading(angolo);
        }

        RwV3d mano = { 0.0f, 0.0f, 0.0f };
        ped->GetBonePosition(mano, BONE_MANO_DESTRA, true);
        DisegnaRaggio(CVector(mano.x, mano.y, mano.z), bersaglioPunto, preso);
        Bagliori();

        if (preso && colpita && colpita != ped)
            Colpisci(colpita, punto, ped, dt);
    }
} moduloAbilita;

} // namespace
