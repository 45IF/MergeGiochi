"""MergeGiochi - lettura/scrittura IFP ANP3 (GTA San Andreas). Scritto da Claude.
Formato ANP3: 'ANP3', u32 dimensione, nome[24], u32 nAnim; per anim: nome[24], u32 nOggetti, u32 dimDati, u32 1;
per oggetto: nome[24], u32 tipo (3 = solo rotazione, 4 = rotazione+traslazione), u32 nFrame, u32 idOsso;
frame compressi int16: quat x,y,z,w *4096, tempo *60 [, traslazione x,y,z *1024]."""
import struct

def leggi(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'ANP3', d[:4]
    nome = d[8:32].split(b'\0')[0].decode()
    n = struct.unpack_from('<I', d, 32)[0]
    p = 36
    anims = []
    for _ in range(n):
        an = d[p:p+24].split(b'\0')[0].decode(); no, dim, _u = struct.unpack_from('<III', d, p+24); p += 36
        ogg = []
        for _ in range(no):
            on = d[p:p+24].split(b'\0')[0].decode(); tipo, nf, bid = struct.unpack_from('<III', d, p+24); p += 36
            frames = []
            for _ in range(nf):
                if tipo == 2 or tipo == 4:
                    qx, qy, qz, qw, t, tx, ty, tz = struct.unpack_from('<8h', d, p); p += 16
                    frames.append(((qx/4096, qy/4096, qz/4096, qw/4096), t/60, (tx/1024, ty/1024, tz/1024)))
                else:
                    qx, qy, qz, qw, t = struct.unpack_from('<5h', d, p); p += 10
                    frames.append(((qx/4096, qy/4096, qz/4096, qw/4096), t/60, None))
            ogg.append(dict(nome=on, tipo=tipo, osso=bid, frames=frames))
        anims.append(dict(nome=an, oggetti=ogg))
    return nome, anims

def _c16(v, k):
    return max(-32768, min(32767, int(round(v * k))))

def scrivi(path, nome_blocco, anims):
    """anims: [{nome, oggetti:[{nome, osso, frames:[(quat xyzw, tempo s, trasl o None)]}]}]"""
    corpo = b''
    for a in anims:
        dati = b''
        for o in a['oggetti']:
            con_t = any(f[2] is not None for f in o['frames'])
            tipo = 4 if con_t else 3
            dati += o['nome'].encode()[:23].ljust(24, b'\0') + struct.pack('<III', tipo, len(o['frames']), o['osso'])
            for q, t, tr in o['frames']:
                dati += struct.pack('<5h', _c16(q[0], 4096), _c16(q[1], 4096), _c16(q[2], 4096), _c16(q[3], 4096), _c16(t, 60))
                if con_t:
                    tr = tr or (0.0, 0.0, 0.0)
                    dati += struct.pack('<3h', _c16(tr[0], 1024), _c16(tr[1], 1024), _c16(tr[2], 1024))
        corpo += a['nome'].encode()[:23].ljust(24, b'\0') + struct.pack('<III', len(a['oggetti']), len(dati), 1) + dati
    testa = nome_blocco.encode()[:23].ljust(24, b'\0') + struct.pack('<I', len(anims))
    tutto = testa + corpo
    open(path, 'wb').write(b'ANP3' + struct.pack('<I', len(tutto)) + tutto)
