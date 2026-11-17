#!/usr/bin/env python3
"""nucode_selfbalancing.fzz의 브레드보드 뷰에 docs/hardware.md §3 핀맵대로 와이어를 추가한다.

사용: python3 -I wire_sketch.py <원본.fzz> <출력.fzz> <q_dir>
q_dir: 각 부품 breadboard SVG에 `inkscape --query-all`을 돌려 만든 파일(커넥터 중심 좌표)
"""
import sys, zipfile, re
import xml.etree.ElementTree as ET

SRC, DST, QDIR, FZPZ = sys.argv[1:5]
K = 90 / 96  # inkscape px(96dpi) -> Fritzing scene px(90dpi)

def load_q(name):
    d = {}
    for ln in open(f"{QDIR}/{name}"):
        p = ln.strip().split(",")
        m = re.fullmatch(r"connector(\d+)pin", p[0])
        if m:
            x, y, w, h = map(float, p[1:5])
            d[int(m.group(1))] = ((x + w/2) * K, (y + h/2) * K)
    return d

Q = {
    "U1":  load_q("q_svg.breadboard.NU54V-DK_breadboard.svg.txt"),
    "MPU": load_q("q_MPU6050_GY521_9.svg.txt"),
    "TB":  load_q("q_svg.breadboard.adafruit_6609f8449e258ae7e31b5367b618ff28_2_breadboard_68.svg.txt"),
}

zin = zipfile.ZipFile(SRC)
fzname = [n for n in zin.namelist() if n.endswith(".fz")][0]
fztext = zin.read(fzname).decode("utf-8")
zp = zipfile.ZipFile(FZPZ)
new_id = [n for n in zp.namelist() if n.endswith(".fzp")][0][len("part."):-4]
fztext = re.sub(r"NU54V-DK_nucode_v\d+", new_id, fztext)
root = ET.fromstring(fztext.encode("utf-8"))
insts = root.find("instances")

def part(title):
    for i in insts:
        if i.findtext("title") == title:
            return i
    raise SystemExit("no part " + title)

P = {"U1": part("U1"), "MPU": part("부품2"), "TB": part("부품1")}

def scene_pos(key, cid):
    inst = P[key]
    g = inst.find("views/breadboardView/geometry")
    x0, y0 = float(g.get("x")), float(g.get("y"))
    lx, ly = Q[key][cid]
    t = g.find("transform")
    if t is not None:
        m = {k: float(v) for k, v in t.attrib.items()}
        lx, ly = lx*m["m11"] + ly*m["m21"] + m["m31"], lx*m["m12"] + ly*m["m22"] + m["m32"]
    return x0 + lx, y0 + ly

# P4 핀 n -> NU54V-DK connector 29+n, P2 핀 n -> n-1
def p4(n): return 29 + n

RED, BLACK, BLUE, ORANGE, GREEN, YELLOW = "#cc1717", "#404040", "#418dd9", "#ff7033", "#47cc79", "#ffe24d"
# (이름, 색, (부품, 커넥터), (부품, 커넥터))
WIRES = [
    ("PWMA P1.10",  ORANGE, ("U1", p4(8)),  ("TB", 17)),
    ("PWMB P1.14",  ORANGE, ("U1", p4(12)), ("TB", 11)),
    ("AIN1 P2.00",  BLUE,   ("U1", p4(19)), ("TB", 15)),
    ("AIN2 P2.01",  BLUE,   ("U1", p4(20)), ("TB", 16)),
    ("BIN1 P2.02",  BLUE,   ("U1", p4(21)), ("TB", 13)),
    ("BIN2 P2.03",  BLUE,   ("U1", p4(22)), ("TB", 12)),
    ("STBY 3V3",    RED,    ("U1", p4(26)), ("TB", 14)),
    ("TB VCC 3V3",  RED,    ("U1", p4(26)), ("TB", 9)),
    ("TB GND",      BLACK,  ("U1", p4(23)), ("TB", 7)),
    ("IMU VCC 3V3", RED,    ("U1", p4(26)), ("MPU", 0)),
    ("IMU GND",     BLACK,  ("U1", p4(24)), ("MPU", 1)),
    ("IMU SCL P4-16", GREEN, ("U1", p4(16)), ("MPU", 2)),
    ("IMU SDA P4-17", YELLOW, ("U1", p4(17)), ("MPU", 3)),
]

def bb_connectors(inst):
    return inst.find("views/breadboardView/connectors")

def add_part_side(key, cid, wire_idx, wire_cid):
    cons = bb_connectors(P[key])
    for c in cons:
        if c.get("connectorId") == f"connector{cid}":
            break
    else:
        c = ET.SubElement(cons, "connector", connectorId=f"connector{cid}", layer="breadboard")
        ET.SubElement(c, "geometry", x="0", y="0")
    cs = c.find("connects")
    if cs is None:
        cs = ET.SubElement(c, "connects")
    ET.SubElement(cs, "connect", connectorId=wire_cid, modelIndex=str(wire_idx), layer="breadboardWire")

base = 90001
for n, (name, color, a, b) in enumerate(WIRES):
    idx = base + n
    (xa, ya), (xb, yb) = scene_pos(*a), scene_pos(*b)
    w = ET.SubElement(insts, "instance", moduleIdRef="WireModuleID", modelIndex=str(idx),
                      path=":/resources/parts/core/wire.fzp")
    ET.SubElement(w, "title").text = f"Wire{n+1}"
    ET.SubElement(w, "property", name="comment", value=name) if False else None
    views = ET.SubElement(w, "views")
    bv = ET.SubElement(views, "breadboardView", layer="breadboardWire")
    ET.SubElement(bv, "geometry", z=str(3.5 + n*0.0001), x=f"{xa:.4f}", y=f"{ya:.4f}",
                  x1="0", y1="0", x2=f"{xb-xa:.4f}", y2=f"{yb-ya:.4f}", wireFlags="64")
    ET.SubElement(bv, "wireExtras", mils="22.2222", color=color, opacity="1", banded="0")
    cons = ET.SubElement(bv, "connectors")
    for end, (key, cid) in (("connector0", a), ("connector1", b)):
        c = ET.SubElement(cons, "connector", connectorId=end, layer="breadboardWire")
        ET.SubElement(c, "geometry", x="0", y="0")
        cs = ET.SubElement(c, "connects")
        ET.SubElement(cs, "connect", connectorId=f"connector{cid}", modelIndex=P[key].get("modelIndex"), layer="breadboard")
        add_part_side(key, cid, idx, end)

ET.indent(root, space="    ")
out = b'<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root)
seen = set()
with zipfile.ZipFile(DST, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr(fzname, out); seen.add(fzname)
    for n in zp.namelist():
        z.writestr(n, zp.read(n)); seen.add(n)
    for n in zin.namelist():
        if n not in seen and "NU54V-DK" not in n:
            z.writestr(n, zin.read(n)); seen.add(n)
for n, (name, color, a, b) in enumerate(WIRES):
    print(f"Wire{n+1}: {name}: {a} -> {b}")
