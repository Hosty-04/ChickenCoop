#!/usr/bin/env python3
"""Vývojové diagramy kurníku (styl podle původních Flowcharts/*.svg).

Použití: python3 flowcharts.py <výstupní adresář>
Pro každý diagram vznikne <jméno>.svg (černé) a <jméno>_black/_white.png (průhledné pozadí, 96 dpi).
"""
import math, os, sys
import cairosvg

SW = 1.0              # tloušťka čáry (mm)
FS = 4.23333          # velikost písma (16 px)
LH = 5.3              # řádkování
BW, BH = 44.0, 22.2   # obdélník
DW, DH = 66.0, 44.0   # kosočtverec
TW, TH, TR = 26.0, 11.9, 3.7  # Start / Konec
GAP = 16.4            # mezera mezi prvky
ARM, ANG = 6.75, 30   # šipka


class Diagram:
    def __init__(self, name, title):
        self.name, self.title = name, title
        self.shapes, self.texts, self.lines = [], [], []
        self.max_x = self.max_y = 0.0

    def _ext(self, x, y):
        self.max_x, self.max_y = max(self.max_x, x), max(self.max_y, y)

    def _text(self, cx, cy, s):
        rows = s.split("\n")
        for i, r in enumerate(rows):
            y = cy + 1.5 + (i - (len(rows) - 1) / 2) * LH
            self.texts.append(f'<text x="{cx:.3f}" y="{y:.3f}">{r}</text>')

    # --- tvary (vrací slovník s hranami pro snadné napojení čar) ---
    def box(self, cx, cy, s):
        self.shapes.append(f'<rect x="{cx-BW/2:.3f}" y="{cy-BH/2:.3f}" width="{BW}" height="{BH}"/>')
        self._text(cx, cy, s); self._ext(cx + BW/2, cy + BH/2)
        return dict(x=cx, y=cy, t=cy - BH/2, b=cy + BH/2, l=cx - BW/2, r=cx + BW/2)

    def term(self, cx, cy, s):
        self.shapes.append(f'<rect x="{cx-TW/2:.3f}" y="{cy-TH/2:.3f}" width="{TW}" height="{TH}" rx="{TR}" ry="{TR}"/>')
        self._text(cx, cy, s); self._ext(cx + TW/2, cy + TH/2)
        return dict(x=cx, y=cy, t=cy - TH/2, b=cy + TH/2, l=cx - TW/2, r=cx + TW/2)

    def diamond(self, cx, cy, s):
        p = [(cx, cy - DH/2), (cx + DW/2, cy), (cx, cy + DH/2), (cx - DW/2, cy)]
        self.shapes.append('<polygon points="' + " ".join(f"{x:.3f},{y:.3f}" for x, y in p) + '"/>')
        self._text(cx, cy, s); self._ext(cx + DW/2, cy + DH/2)
        return dict(x=cx, y=cy, t=cy - DH/2, b=cy + DH/2, l=cx - DW/2, r=cx + DW/2)

    # --- čáry ---
    def path(self, pts, arrow=True):
        """Lomená čára; s arrow končí otevřenou šipkou, jejíž vnější hrot leží přesně na posledním bodě."""
        pts = list(pts)
        for x, y in pts:
            self._ext(x + SW/2, y + SW/2)
        if arrow:
            (x0, y0), (x1, y1) = pts[-2], pts[-1]
            L = math.hypot(x1 - x0, y1 - y0); ux, uy = (x1 - x0) / L, (y1 - y0) / L
            back = (SW / 2) / math.sin(math.radians(ANG))
            tx, ty = x1 - ux * back, y1 - uy * back
            pts[-1] = (tx, ty)
            arms = []
            for s in (+1, -1):
                a = s * math.radians(ANG)
                dx, dy = -(ux * math.cos(a) - uy * math.sin(a)), -(ux * math.sin(a) + uy * math.cos(a))
                arms.append((tx + dx * ARM, ty + dy * ARM))
            self.lines.append(f'<path d="M {arms[0][0]:.3f},{arms[0][1]:.3f} L {tx:.3f},{ty:.3f} L {arms[1][0]:.3f},{arms[1][1]:.3f}"/>')
        self.lines.append('<path d="M ' + " L ".join(f"{x:.3f},{y:.3f}" for x, y in pts) + '"/>')

    # --- popisky větví ANO / NE ---
    def lab_right(self, d, s):   # výstup doprava: nad čarou vpravo od hrotu
        self.texts.append(f'<text x="{d["r"]+1.6:.3f}" y="{d["y"]-3.4:.3f}" text-anchor="start">{s}</text>')

    def lab_left(self, d, s):    # výstup doleva: nad čarou vlevo od hrotu
        self.texts.append(f'<text x="{d["l"]-1.6:.3f}" y="{d["y"]-3.4:.3f}" text-anchor="end">{s}</text>')

    def lab_down(self, d, s):    # výstup dolů: vpravo od čáry pod hrotem
        self.texts.append(f'<text x="{d["x"]+9.5:.3f}" y="{d["b"]+4.6:.3f}" text-anchor="start">{s}</text>')

    def lab_up(self, d, s):      # výstup nahoru: vpravo od čáry nad hrotem
        self.texts.append(f'<text x="{d["x"]+9.5:.3f}" y="{d["t"]-2.4:.3f}" text-anchor="start">{s}</text>')

    # --- výstup ---
    def svg(self, color):
        W, H = self.max_x + 2.0, self.max_y + 2.0
        return f'''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg" width="{W:.3f}mm" height="{H:.3f}mm" viewBox="0 0 {W:.3f} {H:.3f}" version="1.1">
  <title>{self.title}</title>
  <g fill="none" stroke="{color}" stroke-width="{SW}" stroke-linecap="butt" stroke-linejoin="miter">
{chr(10).join(self.lines)}
{chr(10).join(self.shapes)}
  </g>
  <g fill="{color}" stroke="none" font-family="'Open Sans', sans-serif" font-size="{FS}" text-anchor="middle">
{chr(10).join(self.texts)}
  </g>
</svg>
''', W, H

    def save(self, out):
        for variant, color in (("black", "#000000"), ("white", "#ffffff")):
            s, W, H = self.svg(color)
            base = f"{out}/{self.name}_{variant}"
            if variant == "black":
                open(f"{out}/{self.name}.svg", "w", encoding="utf-8").write(s)
            cairosvg.svg2png(bytestring=s.encode(), write_to=base + ".png",
                             output_width=round(W / 25.4 * 96), output_height=round(H / 25.4 * 96))
        print(f"{self.name}: {W:.1f} x {H:.1f} mm -> {round(W/25.4*96)} x {round(H/25.4*96)} px")


class Column:
    """Pomocník pro hlavní svislý sloupec: každý další prvek pod předchozím, spojený šipkou."""
    def __init__(self, d, x, top=2.0):
        self.d, self.x = d, x
        self.last = d.term(x, top + TH/2, "Start")

    def _below(self, h, extra):
        return self.last["b"] + GAP + extra + h/2

    def box(self, s, extra=0.0, link=True):
        cy = self._below(BH, extra)
        if link: self.d.path([(self.x, self.last["b"]), (self.x, cy - BH/2)])
        self.last = self.d.box(self.x, cy, s); return self.last

    def diamond(self, s, extra=0.0, link=True):
        cy = self._below(DH, extra)
        if link: self.d.path([(self.x, self.last["b"]), (self.x, cy - DH/2)])
        self.last = self.d.diamond(self.x, cy, s); return self.last

    def end(self, extra=0.0):
        cy = self._below(TH, extra)
        self.d.path([(self.x, self.last["b"]), (self.x, cy - TH/2)])
        self.last = self.d.term(self.x, cy, "Konec"); return self.last


def row_below(shape, h):
    """Střed prvku výšky h umístěného pod tvarem shape s běžnou mezerou."""
    return shape["b"] + GAP + h/2


# =============================================================================
# 1) Detekce snesených vajec
# =============================================================================
def egg():
    d = Diagram("egg_detection_flowchart", "Algoritmus detekce snesených vajec")
    C1, C2, C3, BUS = 37.0, 117.0, 197.0, 246.0
    c = Column(d, C1)
    c.box("Probuď vše\npotřebné")
    c.box("Počkej na\nustálení váhy")
    c.box("Odeber vzorky")
    c.box("Vypočti medián")
    c.box("Vyber vzorky\nnejblíže mediánu")
    c.box("Vypočti hmotnost\na odchylku")

    # slepice v hnízdě
    hen = c.diamond("Je v hnízdě\nslepice?"); d.lab_down(hen, "NE")
    drop1 = d.box(C2, hen["y"], "Zahoď měření")
    d.path([(hen["r"], hen["y"]), (drop1["l"], hen["y"])]); d.lab_right(hen, "ANO")
    brood = d.diamond(C3, hen["y"], "Sedí tam slepice\nopakovaně?")
    d.path([(drop1["r"], hen["y"]), (brood["l"], hen["y"])])
    occ = d.box(C3, row_below(brood, BH), "Označ kvočnu")
    d.path([(C3, brood["b"]), (C3, occ["t"])]); d.lab_down(brood, "ANO")
    d.lab_right(brood, "NE")
    d.path([(occ["r"], occ["y"]), (BUS, occ["y"])])

    # stabilita (níž, aby vodorovná větev podešla box „obsazené")
    dev = c.diamond("Je měření\nstabilní?", extra=occ["b"] + 11.0 - (c.last["b"] + GAP))
    d.lab_down(dev, "ANO")
    drop2 = d.box(C2, dev["y"], "Zahoď měření")
    d.path([(dev["r"], dev["y"]), (drop2["l"], dev["y"])]); d.lab_right(dev, "NE")
    d.path([(drop2["r"], dev["y"]), (BUS, dev["y"])])

    # počítání vajec – větev NE se vrací do sloupce dost vysoko nad další šipkou
    egg_ = c.diamond("Přibylo nebo\nubylo vejce?"); d.lab_down(egg_, "ANO")
    add = c.box("Přičti nebo\nodečti vejce")
    y_merge = add["b"] + 7.0
    d.path([(egg_["r"], egg_["y"]), (C2, egg_["y"]), (C2, y_merge), (C1, y_merge)]); d.lab_right(egg_, "NE")

    C2d = C2 + 12.0   # sloupec pro „Označ košík…“, kontrolu driftu a „Aktualizuj nulu…“
    full = c.diamond("Je košík plný?", extra=10.0); d.lab_down(full, "NE")
    fb = d.box(C2d, full["y"], "Označ košík\njako plný")
    d.path([(full["r"], full["y"]), (fb["l"], full["y"])]); d.lab_right(full, "ANO")
    d.path([(fb["r"], full["y"]), (BUS, full["y"])])

    # prázdné hnízdo: vynulování počtu a kontrola driftu
    empty = c.diamond("Je hnízdo\nprázdné?"); d.lab_down(empty, "NE")
    clr = d.box(C2d, empty["y"], "Vynuluj počet\nvajec")
    d.path([(empty["r"], empty["y"]), (clr["l"], empty["y"])]); d.lab_right(empty, "ANO")
    drift = d.diamond(C2d, row_below(clr, DH), "Proběhla dnes\nkontrola driftu?")
    d.path([(C2d, clr["b"]), (C2d, drift["t"])])
    d.path([(drift["r"], drift["y"]), (BUS, drift["y"])]); d.lab_right(drift, "ANO")
    zero = d.box(C2d, row_below(drift, BH), "Zkontroluj drift\na oprav nulu")
    d.path([(C2d, drift["b"]), (C2d, zero["t"])]); d.lab_down(drift, "NE")
    d.path([(zero["r"], zero["y"]), (BUS, zero["y"])])

    send = c.box("Odešli počet\nvajec", extra=zero["b"] + GAP - c.last["b"])
    d.path([(brood["r"], brood["y"]), (BUS, brood["y"]), (BUS, send["y"]), (send["r"], send["y"])])
    c.box("Uspi vše\npoužívané")
    c.end()
    return d


# =============================================================================
# 2) MOSFET odpojovač – kontrola panelu a akumulátoru (podle battery.c)
# =============================================================================
def separator():
    d = Diagram("separator_flowchart", "Kontrola fotovoltaického panelu a akumulátoru")
    C1, C2, C3 = 37.0, 125.0, 210.0
    c = Column(d, C1)
    c.box("Probuď vše\npotřebné")
    c.box("Navzorkuj napětí\nakumulátoru")

    # panel se před měřením napětí naprázdno odpojí (jen je-li připojený)
    conn = c.diamond("Je panel\npřipojen?"); d.lab_down(conn, "NE")
    cut = d.box(C2, conn["y"], "Odpoj panel")
    d.path([(conn["r"], conn["y"]), (cut["l"], conn["y"])]); d.lab_right(conn, "ANO")
    wait = d.box(C3, conn["y"], "Počkej na\nustálení")
    d.path([(cut["r"], conn["y"]), (wait["l"], conn["y"])])
    y_m1 = conn["b"] + 9.5
    d.path([(C3, wait["b"]), (C3, y_m1), (C1, y_m1)])
    c.box("Navzorkuj napětí\npanelu", extra=y_m1 + 11.0 - (c.last["b"] + GAP))

    # kritický režim
    crit = c.diamond("Je akumulátor\nvybitý?"); d.lab_down(crit, "NE")
    on = d.box(C2, crit["y"], "Zapni kritický\nrežim")
    d.path([(crit["r"], crit["y"]), (on["l"], crit["y"])]); d.lab_right(crit, "ANO")
    off = c.box("Vypni kritický\nrežim")
    y_m2 = off["b"] + 7.0
    d.path([(C2, on["b"]), (C2, y_m2), (C1, y_m2)])

    # připojení panelu – rozhodování v řadě vedle sebe
    lim = c.diamond("Je akumulátor\nnabitý?", extra=10.0); d.lab_down(lim, "ANO")
    low = d.diamond(C2, lim["y"], "Je napětí panelu\nnižší než\nakumulátoru?")
    d.path([(lim["r"], lim["y"]), (low["l"], lim["y"])]); d.lab_right(lim, "NE")
    on_p = d.box(C3, lim["y"], "Připoj panel")
    d.path([(low["r"], low["y"]), (on_p["l"], low["y"])]); d.lab_right(low, "NE")

    send = c.box("Odešli data", extra=8.0)
    d.path([(C3, on_p["b"]), (C3, send["y"]), (send["r"], send["y"])])
    d.path([(C2, low["b"]), (C2, send["y"])]); d.lab_down(low, "ANO")
    c.box("Uspi vše\npoužívané")
    c.end()
    return d


# =============================================================================
# 3) Pohyb dvířek (podle README a door.c / motor.c) – bez křížení čar
# =============================================================================
def door():
    d = Diagram("door_flowchart", "Řízení dvířek kurníku")
    C1, C2, C3 = 60.0, 155.0, 235.0
    XL, XR, XD, XA = 15.0, 273.0, 285.0, 297.0   # svislé vodiče: smyčka vlevo, návrat, porucha, sběrnice
    c = Column(d, C1)
    c.box("Probuď vše\npotřebné")

    was = c.diamond("Jsou dvířka\nporouchaná?"); d.lab_down(was, "NE")
    cmd = d.diamond(C2, was["y"], "Máš příkaz\nod uživatele?")
    d.path([(was["r"], was["y"]), (cmd["l"], was["y"])]); d.lab_right(was, "ANO")
    ok = d.box(C2, row_below(cmd, BH), "Nastav dvířka\njako funkční")
    d.path([(C2, cmd["b"]), (C2, ok["t"])]); d.lab_down(cmd, "ANO")
    d.path([(ok["l"], ok["y"]), (C1, ok["y"])])
    d.lab_right(cmd, "NE")

    endpos = c.diamond("Jsou dvířka\nv krajní poloze?", extra=ok["b"] - c.last["b"]); d.lab_down(endpos, "ANO")
    d.lab_right(endpos, "NE")
    motor = c.box("Zapni motor")
    c.box("Počkej na\nrozběh motoru")
    pwm = c.box("Uprav střídu PWM")
    c.box("Navzorkuj napětí\na proud")
    cur = c.diamond("Je proud\npříliš vysoký?"); d.lab_down(cur, "NE")
    t25 = c.diamond("Běží motor\npříliš dlouho?"); d.lab_down(t25, "NE")
    done = c.diamond("Je úkon\ndokončen?"); d.lab_down(done, "ANO")
    # smyčka regulace vlevo
    d.path([(done["l"], done["y"]), (XL, done["y"]), (XL, pwm["y"]), (pwm["l"], pwm["y"])]); d.lab_left(done, "NE")
    stop_ok = c.box("Vypni motor")

    # překážka – pravý sloupec
    t150 = d.diamond(C2, cur["y"], "Je to jen\nkrátká špička?")
    d.path([(cur["r"], cur["y"]), (t150["l"], cur["y"])]); d.lab_right(cur, "ANO")
    d.path([(C2, t150["t"]), (C2, pwm["y"]), (pwm["r"], pwm["y"])]); d.lab_up(t150, "ANO")
    stop = d.box(C2, row_below(t150, BH), "Zastav motor")
    d.path([(C2, t150["b"]), (C2, stop["t"])]); d.lab_down(t150, "NE")
    wait = d.box(C2, row_below(stop, BH), "Počkej na\nzastavení")
    d.path([(C2, stop["b"]), (C2, wait["t"])])
    tries = d.diamond(C2, row_below(wait, DH), "Zbývají ještě\npokusy?")
    d.path([(C2, wait["b"]), (C2, tries["t"])])
    plan = d.box(C3, tries["y"], "Naplánuj\nnový pokus")
    d.path([(tries["r"], tries["y"]), (plan["l"], tries["y"])]); d.lab_right(tries, "ANO")
    rev = d.box(C3, row_below(plan, BH), "Obrať směr\ntahu")
    d.path([(C3, plan["b"]), (C3, rev["t"])])
    d.path([(rev["r"], rev["y"]), (XR, rev["y"]), (XR, motor["y"]), (motor["r"], motor["y"])])

    # porucha
    fault = d.box(C2, rev["y"] + BH + GAP, "Nastav dvířka\nna porouchaná")
    d.path([(C2, tries["b"]), (C2, fault["t"])]); d.lab_down(tries, "NE")
    y_j = fault["t"] - 18.0   # dost vysoko nad šipkou do boxu, aby se hroty nepřekrývaly
    XM = (t25["r"] + t150["l"]) / 2
    d.path([(t25["r"], t25["y"]), (XM, t25["y"]), (XM, y_j), (C2, y_j)]); d.lab_right(t25, "ANO")
    d.path([(endpos["r"], endpos["y"]), (XD, endpos["y"]), (XD, fault["y"]), (fault["r"], fault["y"])])

    send = c.box("Odešli data", extra=max(0.0, fault["b"] + GAP - c.last["b"]))
    d.path([(cmd["r"], cmd["y"]), (XA, cmd["y"]), (XA, send["y"]), (send["r"], send["y"])])
    d.path([(C2, fault["b"]), (C2, send["y"])])
    c.box("Uspi vše\npoužívané")
    c.end()
    return d


if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for f in (egg, separator, door):
        f().save(out)
