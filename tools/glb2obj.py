# Tripo GLB -> OBJ + MTL (+ 텍스처 JPG 추출). 사용: python -I glb2obj.py <assets 폴더> <ID>...
# 메시 1개·삼각형·노드 변환 없음(Tripo 출력)만 처리한다. vt는 OpenGL 기준(아래가 0)으로 v를 뒤집어 쓴다.
import json, struct, sys, os

def read_glb(path):
    f = open(path, 'rb').read()
    clen = struct.unpack_from('<I', f, 12)[0]
    j = json.loads(f[20:20 + clen])
    return f, j, 20 + clen + 8

def accessor(f, j, base, i):
    a = j['accessors'][i]; v = j['bufferViews'][a['bufferView']]
    n = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3}[a['type']]
    fmt = {5126: 'f', 5125: 'I', 5123: 'H'}[a['componentType']]
    stride = v.get('byteStride', n * struct.calcsize(fmt))
    off = base + v.get('byteOffset', 0) + a.get('byteOffset', 0)
    return [struct.unpack_from('<' + fmt * n, f, off + k * stride) for k in range(a['count'])]

def convert(assets, id):
    f, j, base = read_glb(f'{assets}/glb/{id}.glb')
    assert len(j['meshes']) == 1 and len(j['meshes'][0]['primitives']) == 1, 'mesh 1개만 지원'
    assert not any(k in j['nodes'][0] for k in ('matrix', 'translation', 'rotation', 'scale')), '노드 변환 있음'
    p = j['meshes'][0]['primitives'][0]
    P = accessor(f, j, base, p['attributes']['POSITION'])
    N = accessor(f, j, base, p['attributes']['NORMAL'])
    T = accessor(f, j, base, p['attributes']['TEXCOORD_0'])
    I = [x[0] for x in accessor(f, j, base, p['indices'])]

    im = j['images'][0]; v = j['bufferViews'][im['bufferView']]; o = base + v.get('byteOffset', 0)
    ext = {'image/jpeg': 'jpg', 'image/png': 'png'}[im['mimeType']]
    open(f'{assets}/textures/{id}.{ext}', 'wb').write(f[o:o + v['byteLength']])

    with open(f'{assets}/obj/{id}.mtl', 'w', newline='\n') as m:
        m.write(f'newmtl {id}\nKa 1 1 1\nKd 1 1 1\nKs 0 0 0\nmap_Kd ../textures/{id}.png\n')
    with open(f'{assets}/obj/{id}.obj', 'w', newline='\n') as o_:
        o_.write(f'# {id}: from glb/{id}.glb (Tripo). +Y up, front +Z, base at y=0, height 1\n')
        o_.write(f'# vertices {len(P)}, triangles {len(I)//3}\nmtllib {id}.mtl\no {id}\n')
        o_.writelines(f'v {x:.6f} {y:.6f} {z:.6f}\n' for x, y, z in P)
        o_.writelines(f'vt {u:.6f} {1 - w:.6f}\n' for u, w in T)
        o_.writelines(f'vn {x:.6f} {y:.6f} {z:.6f}\n' for x, y, z in N)
        o_.write(f'usemtl {id}\n')
        for t in range(0, len(I), 3):
            a, b, c = (k + 1 for k in I[t:t + 3])
            o_.write(f'f {a}/{a}/{a} {b}/{b}/{b} {c}/{c}/{c}\n')
    return ext, len(P), len(I) // 3

if __name__ == '__main__':
    for id in sys.argv[2:]:
        print(id, *convert(sys.argv[1], id))
