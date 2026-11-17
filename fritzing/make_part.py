#!/usr/bin/env python3
"""NU54V-DK Fritzing 파츠(.fzpz)와 스케치(.fzz) 생성기.

핀 이름은 회로도 시트 9(Pad Layout)의 P2/P4 커넥터 기준.
P2: 이미지 왼쪽 열, 위→아래 = 1→30.  P4: 이미지 오른쪽 열, 위→아래 = 30→1.
"""
import os, sys, zipfile
from xml.sax.saxutils import escape

OUT = os.path.dirname(os.path.abspath(__file__))
MODULE_ID = "NU54V-DK_nucode_v3"
BASE = "NU54V-DK"

P2 = ["GND"]*8 + ["P1.07","P1.06","P1.05","P1.04"] + ["GND"]*3 + \
     ["P1.08","P2.04","GND","P2.05","P2.06","P2.07","P2.08","P2.09","P2.10",
      "P0.00","P0.01","SWDCLK","SWDIO","VDD_MOD","GND"]
P4 = ["GND","GND","MOD_RST","P0.02","P0.03","P0.04","P1.09","P1.10","P1.11",
      "P1.12","P1.13","P1.14","GND","P1.00","P1.01","P1.02","P1.03","GND",
      "P2.00","P2.01","P2.02","P2.03","GND","GND","VDD_MOD","VDD_3V3_SYS",
      "VBAT","GND","VEXT","VBUS"]
assert len(P2) == 30 and len(P4) == 30
SHORT = {"VDD_MOD": "VMOD", "VDD_3V3_SYS": "3V3", "MOD_RST": "RST"}

# 커넥터 목록: (index, 헤더, 핀번호, 신호, 화면 행(0=맨 위), 열)
pins = []
for n in range(1, 31):
    pins.append((len(pins), "P2", n, P2[n-1], n-1, "L"))
for n in range(1, 31):
    pins.append((len(pins), "P4", n, P4[n-1], 30-n, "R"))

def lab(sig): return SHORT.get(sig, sig)

# ---------- breadboard ----------
K = 387.0  # 이미지 px / inch (핀 피치 38.7 px = 0.1 in)
W, H = 551, 1239
XL, XR, Y0, DY = 100.0, 100.0 + 0.9*K, 85.0, 38.7

def breadboard():
    # 보드 외곽(x 80~473, y 8~1230)만 viewBox로 잘라서 핀 이름을 보드 안쪽에 둔다
    BX, BY, BW, BH = 80, 8, 393, 1222
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{BW/K:.4f}in" height="{BH/K:.4f}in" viewBox="{BX} {BY} {BW} {BH}">',
         '<g id="breadboard">',
         f'<rect x="{BX}" y="{BY}" width="{BW}" height="{BH}" rx="8" fill="#101214"/>',
         '<rect x="218" y="8" width="115" height="105" rx="6" fill="#c9ccd1"/>',
         '<rect x="198" y="975" width="160" height="225" fill="#d9d9d9"/>',
         '<text x="278" y="1092" font-family="Arial" font-size="18" font-weight="bold" fill="#222" text-anchor="middle">NCRB54N01VC</text>',
         '<text x="276" y="600" font-family="Arial" font-size="24" font-weight="bold" fill="#fff" text-anchor="middle">NU54V-DK</text>']
    for (i, hdr, n, sig, row, col) in pins:
        x = XL if col == "L" else XR
        y = Y0 + DY*row
        if col == "L":
            bx, nx, nanc = x + 18, x + 18 + 64 + 6, "start"
        else:
            bx, nx, nanc = x - 18 - 64, x - 18 - 64 - 6, "end"
        s.append(f'<rect x="{bx:.1f}" y="{y-14:.1f}" width="64" height="28" fill="#5d6b78"/>')
        s.append(f'<text x="{bx+32:.1f}" y="{y+6:.1f}" font-family="Arial" font-size="17" fill="#fff" text-anchor="middle">{escape(lab(sig))}</text>')
        s.append(f'<text x="{nx:.1f}" y="{y+4:.1f}" font-family="Arial" font-size="12" fill="#9aa5b1" text-anchor="{nanc}">{n}</text>')
        s.append(f'<circle id="connector{i}pin" cx="{x:.1f}" cy="{y:.1f}" r="14" fill="#f2cd00"/>')
        s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="6" fill="#101214"/>')
    s.append('</g></svg>')
    return "\n".join(s)

def icon():
    return ('<svg xmlns="http://www.w3.org/2000/svg" width="0.3in" height="0.93in" viewBox="0 0 30 93">\n'
            '<g id="icon"><rect x="1" y="1" width="28" height="91" rx="2" fill="#101214"/>'
            '<rect x="8" y="1" width="14" height="9" fill="#c9ccd1"/>'
            '<g fill="#f2cd00">' + "".join(f'<circle cx="5" cy="{14+r*2.7:.1f}" r="1"/><circle cx="25" cy="{14+r*2.7:.1f}" r="1"/>' for r in range(28)) +
            '</g></g></svg>')

# ---------- schematic (1 unit = 0.01 in) ----------
def schematic():
    SW, SH = 170, 360
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{SW/100:.2f}in" height="{SH/100:.2f}in" viewBox="0 0 {SW} {SH}">',
         '<g id="schematic">',
         '<rect x="30" y="15" width="110" height="330" fill="#fffff0" stroke="#000" stroke-width="1"/>',
         '<text x="85" y="10" font-family="Arial" font-size="8" font-weight="bold" text-anchor="middle">NU54V-DK</text>',
         '<text x="35" y="26" font-family="Arial" font-size="5" fill="#555">P2</text>',
         '<text x="135" y="26" font-family="Arial" font-size="5" fill="#555" text-anchor="end">P4</text>']
    for (i, hdr, n, sig, row, col) in pins:
        # 스키매틱은 번호 순서(위→아래)로 배치: P2 왼쪽 1→30, P4 오른쪽 1→30
        y = 30 + 10*(n-1)
        if col == "L":
            x1, x2, tx, anc = 10, 30, 33, "start"
        else:
            x1, x2, tx, anc = 160, 140, 137, "end"
        s.append(f'<line id="connector{i}pin" x1="{x1}" y1="{y}" x2="{x2}" y2="{y}" stroke="#000" stroke-width="1"/>')
        s.append(f'<rect id="connector{i}terminal" x="{x1-0.5 if col=="L" else x1-0.5}" y="{y-0.5}" width="1" height="1" fill="none"/>')
        s.append(f'<text x="{tx}" y="{y+2.5}" font-family="Arial" font-size="7" text-anchor="{anc}">{escape(lab(sig))}</text>')
        nx = 28 if col == "L" else 142
        s.append(f'<text x="{(x1+x2)/2:.0f}" y="{y-2}" font-family="Arial" font-size="4" fill="#888" text-anchor="middle">{n}</text>')
    s.append('</g></svg>')
    return "\n".join(s)

# ---------- pcb (1 unit = 0.01 in) ----------
def pcb():
    PW, PH = 110, 330
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{PW/100:.2f}in" height="{PH/100:.2f}in" viewBox="0 0 {PW} {PH}">',
         '<g id="silkscreen">',
         '<rect x="1" y="1" width="108" height="328" fill="none" stroke="#fff" stroke-width="1.2" id="silkscreen-outline"/>',
         '<text x="55" y="165" font-family="Arial" font-size="8" fill="#fff" text-anchor="middle" transform="rotate(-90 55 165)">NU54V-DK</text>',
         '<text x="10" y="9" font-family="Arial" font-size="5" fill="#fff" text-anchor="middle">P2</text>',
         '<text x="100" y="9" font-family="Arial" font-size="5" fill="#fff" text-anchor="middle">P4</text>',
         '</g>',
         '<g id="copper1"><g id="copper0">']
    for (i, hdr, n, sig, row, col) in pins:
        x = 10 if col == "L" else 100
        y = 15 + 10*row
        s.append(f'<circle id="connector{i}pin" cx="{x}" cy="{y}" r="3.6" fill="none" stroke="#F7BD13" stroke-width="1.8"/>')
    s.append('</g></g></svg>')
    return "\n".join(s)

# ---------- fzp ----------
def fzp():
    x = ['<?xml version="1.0" encoding="UTF-8"?>',
         f'<module fritzingVersion="0.9.10" moduleId="{MODULE_ID}">',
         '<version>3</version>',
         '<author>NU54 project</author>',
         '<title>NU54V-DK (nRF54L15)</title>',
         '<label>U</label>',
         '<date>2026-11-17</date>',
         '<tags><tag>NU54V-DK</tag><tag>nRF54L15</tag><tag>Nucode</tag><tag>Zephyr</tag></tags>',
         '<properties><property name="family">NU54V-DK</property><property name="type">NU54V-DK</property>'
         '<property name="pins">60</property></properties>',
         '<description>Nucode NU54V-DK 개발 보드 (P2/P4 30핀 헤더 2개). 핀 이름은 회로도 시트 9 기준.</description>',
         '<views>',
         f'<iconView><layers image="icon/{BASE}_icon.svg"><layer layerId="icon"/></layers></iconView>',
         f'<breadboardView><layers image="breadboard/{BASE}_breadboard.svg"><layer layerId="breadboard"/></layers></breadboardView>',
         f'<schematicView><layers image="schematic/{BASE}_schematic.svg"><layer layerId="schematic"/></layers></schematicView>',
         f'<pcbView><layers image="pcb/{BASE}_pcb.svg"><layer layerId="copper0"/><layer layerId="copper1"/><layer layerId="silkscreen"/></layers></pcbView>',
         '</views>',
         '<connectors>']
    for (i, hdr, n, sig, row, col) in pins:
        x.append(f'<connector id="connector{i}" name="{escape(lab(sig))}" type="male">'
                 f'<description>{hdr} pin {n} ({escape(sig)})</description><views>'
                 f'<breadboardView><p layer="breadboard" svgId="connector{i}pin"/></breadboardView>'
                 f'<schematicView><p layer="schematic" svgId="connector{i}pin" terminalId="connector{i}terminal"/></schematicView>'
                 f'<pcbView><p layer="copper0" svgId="connector{i}pin"/><p layer="copper1" svgId="connector{i}pin"/></pcbView>'
                 '</views></connector>')
    x.append('</connectors><buses>')
    groups = {}
    for (i, hdr, n, sig, row, col) in pins:
        if sig in ("GND", "VDD_MOD"):
            groups.setdefault(sig, []).append(i)
    for name, ids in groups.items():
        x.append(f'<bus id="bus_{name}">' + "".join(f'<nodeMember connectorId="connector{i}"/>' for i in ids) + '</bus>')
    x.append('</buses></module>')
    return "\n".join(x)

# ---------- sketch (.fz) ----------
def sketch():
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<module fritzingVersion="0.9.10">
<views>
<view name="breadboardView" backgroundColor="#ffffff" gridSize="0.1in" showGrid="1" alignToGrid="1" viewFromBelow="0"/>
<view name="schematicView" backgroundColor="#ffffff" gridSize="0.295276in" showGrid="1" alignToGrid="1" viewFromBelow="0"/>
<view name="pcbView" backgroundColor="#333333" gridSize="0.05in" showGrid="1" alignToGrid="1" viewFromBelow="0"/>
</views>
<instances>
<instance moduleIdRef="{MODULE_ID}" modelIndex="5000" path=":/{MODULE_ID}.fzp">
<title>U1</title>
<views>
<breadboardView layer="breadboard"><geometry z="2.5" x="100" y="40"/></breadboardView>
<schematicView layer="schematic"><geometry z="5.5" x="100" y="40"/></schematicView>
<pcbView layer="copper0"><geometry z="1.5" x="100" y="40"/></pcbView>
</views>
</instance>
</instances>
</module>
'''

def bin_xml():
    return f'''<?xml version="1.0" encoding="UTF-8"?>
<module fritzingVersion="1.0.8" icon="Mine.png" >
    <title>NU54V-DK</title>
    <instances>
        <instance moduleIdRef="{MODULE_ID}" modelIndex="5000" path="part.{MODULE_ID}.fzp">
            <title>NU54V-DK (nRF54L15)</title>
            <views>
                <iconView layer="icon"><geometry z="0" x="0" y="0"/></iconView>
            </views>
        </instance>
    </instances>
</module>
'''

def write_zip(path, with_sketch, with_bin=False):
    files = {
        f"part.{MODULE_ID}.fzp": fzp(),
        f"svg.breadboard.{BASE}_breadboard.svg": breadboard(),
        f"svg.schematic.{BASE}_schematic.svg": schematic(),
        f"svg.pcb.{BASE}_pcb.svg": pcb(),
        f"svg.icon.{BASE}_icon.svg": icon(),
    }
    if with_sketch:
        files[f"{BASE}.fz"] = sketch()
    if with_bin:
        files[f"{BASE}.fzb"] = bin_xml()
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        for k, v in files.items():
            z.writestr(k, v)
    return files

if __name__ == "__main__":
    f = write_zip(os.path.join(OUT, f"{BASE}.fzpz"), False)
    write_zip(os.path.join(OUT, f"{BASE}.fzz"), True)
    write_zip(os.path.join(OUT, f"{BASE}.fzbz"), False, True)
    open(os.path.join(OUT, f"{BASE}.fzb"), "w").write(bin_xml())
    # 미리보기용 SVG
    pv = os.path.join(OUT, "preview"); os.makedirs(pv, exist_ok=True)
    for k, v in f.items():
        if k.endswith(".svg"):
            open(os.path.join(pv, k.split(".", 2)[1] + ".svg"), "w").write(v)
    print("ok")
