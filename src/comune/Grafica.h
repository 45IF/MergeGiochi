// MergeGiochi — piccole utilità grafiche condivise (solo asset originali di SA).
// Scritto da Claude.
#pragma once
#include "plugin.h"
#include "CCamera.h"
#include "CShadows.h"
#include <cmath>

namespace mg {

// Striscia rivolta verso la camera fra da e a, larga 2*meta, con la texture originale "shad_exp" di SA
// (gradiente radiale: al centro piena, ai bordi sfumata). Gli stati di render vanno impostati prima.
inline void Striscia(const CVector& da, const CVector& a, float meta, RwUInt8 r, RwUInt8 g, RwUInt8 b, RwUInt8 alfa) {
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

// Cilindro di n lati fra da e a (raggio r): da lontano o visto di punta resta un "tubo" pieno, a differenza
// della striscia che vista di punta collassa. u = 0.5 per tutti i vertici: prende il centro pieno della texture.
inline void Cilindro(const CVector& da, const CVector& a, float r, RwUInt8 cr, RwUInt8 cg, RwUInt8 cb, RwUInt8 alfa, int n = 8) {
    CVector asse = a - da;
    if (asse.MagnitudeSqr() < 1e-6f) return;
    CVector ax = asse; ax.Normalize();
    CVector aiuto = std::fabs(ax.z) < 0.9f ? CVector(0.0f, 0.0f, 1.0f) : CVector(1.0f, 0.0f, 0.0f);
    CVector u1 = CVector::Cross(ax, aiuto); u1.Normalize();
    CVector u2 = CVector::Cross(ax, u1);
    static RwIm3DVertex v[2 * 16];
    static RwImVertexIndex idx[6 * 16];
    if (n > 16) n = 16;
    for (int i = 0; i < n; i++) {
        float t = 6.2831853f * i / n;
        CVector off = (u1 * std::cos(t) + u2 * std::sin(t)) * r;
        CVector p0 = da + off, p1 = a + off;
        RwIm3DVertexSetPos(&v[2 * i], p0.x, p0.y, p0.z);
        RwIm3DVertexSetPos(&v[2 * i + 1], p1.x, p1.y, p1.z);
        for (int k = 0; k < 2; k++) {
            RwIm3DVertexSetRGBA(&v[2 * i + k], cr, cg, cb, alfa);
            RwIm3DVertexSetU(&v[2 * i + k], 0.5f);
            RwIm3DVertexSetV(&v[2 * i + k], 0.5f);
        }
        int j = (i + 1) % n;
        RwImVertexIndex q[6] = { (RwImVertexIndex)(2 * i), (RwImVertexIndex)(2 * i + 1), (RwImVertexIndex)(2 * j),
                                 (RwImVertexIndex)(2 * i + 1), (RwImVertexIndex)(2 * j + 1), (RwImVertexIndex)(2 * j) };
        for (int k = 0; k < 6; k++) idx[6 * i + k] = q[k];
    }
    if (RwIm3DTransform(v, 2 * n, nullptr, rwIM3D_VERTEXUV | rwIM3D_VERTEXRGBA)) {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, idx, 6 * n);
        RwIm3DEnd();
    }
}

// additivo = true per raggi luminosi, false per oggetti opachi-sfumati (es. filo di ragnatela)
inline bool IniziaStrisce(bool additivo) {
    RwTexture* tex = gpShadowExplosionTex;
    if (!tex) return false;
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)(additivo ? rwBLENDONE : rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)RwTextureGetRaster(tex));
    return true;
}

inline void FineStrisce() {
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)nullptr);
}

} // namespace mg
