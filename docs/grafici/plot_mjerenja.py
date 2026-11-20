"""
plot_mjerenja.py - grafici za izvjestaj (Lab5, modifikacija 5)
  sl_alias_vs_f0.png : teorijske frekvencije harmonika/aliasa prema f_a = |k f0 - m fs|
  mj_*.png           : vrijednosti IZMJERENE na ploci ADSP-21489 (19.11.2026.),
                       prepisane iz docs/logovi/log_konzola_jedinstveno.txt
Pokretanje (iz foldera docs/grafici):  python3 plot_mjerenja.py
"""
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = "./"
plt.rcParams.update({"font.size": 10, "figure.dpi": 150})
C1, C2 = "#3b6ea8", "#d9822b"

# --- 0) teorija: harmonici i aliasi u zavisnosti od f0 ------------------
def alias(f, fs):
    f = np.mod(f, fs)
    return np.where(f > fs / 2, fs - f, f)

f0 = np.linspace(1000, 8000, 800)
fig, ax = plt.subplots(1, 2, figsize=(10, 4), sharey=True)
for a, fs in zip(ax, (48000, 96000)):
    for k in range(3, 16, 2):
        fa = alias(k * f0, fs)
        is_alias = k * f0 > fs / 2
        a.plot(f0 / 1e3, np.where(is_alias, fa, np.nan) / 1e3, "r", lw=0.8)
        a.plot(f0 / 1e3, np.where(~is_alias, fa, np.nan) / 1e3, "b", lw=0.8)
    a.plot(f0 / 1e3, f0 / 1e3, "k", lw=1.5)
    a.set_title(f"fs = {fs/1e3:.0f} kHz"); a.set_xlabel("f0 [kHz]")
    a.set_ylim(0, 20); a.grid(alpha=.3)
ax[0].set_ylabel("frekvencija na izlazu [kHz] (do 20 kHz)")
fig.suptitle("Harmonici k=3..15 (plavo = pravi harmonik, crveno = alias)")
fig.tight_layout(); fig.savefig(OUT + "sl_alias_vs_f0.png"); plt.close(fig)

# --- 1) obrada po rezimu, OS=1 / OS=2, fs = 48 kHz, G = 8 ---------------
modes = ["BYPASS", "HARD", "POLY", "TANH"]
os1 = [49, 63, 69, 153]
os2 = [238, 266, 278, 446]          # HARD OS=2 izmjeren na G=2 (cijena ne zavisi od G)
x = np.arange(len(modes)); w = 0.38
fig, ax = plt.subplots(figsize=(7, 3.8))
b1 = ax.bar(x - w/2, os1, w, label="bez nadodmjeravanja (OS=1)", color=C1)
b2 = ax.bar(x + w/2, os2, w, label="2× nadodmjeravanje (OS=2)", color=C2)
for b in list(b1) + list(b2):
    ax.text(b.get_x() + b.get_width()/2, b.get_height() + 5, f"{b.get_height():.0f}",
            ha="center", va="bottom", fontsize=9)
ax.set_xticks(x, modes); ax.set_ylabel("ciklusa po odmjerku")
ax.set_title("Izmjerena cijena obrade u prekidu (fs = 48 kHz, G = 8)")
ax.legend(frameon=False); ax.set_ylim(0, 520); ax.grid(axis="y", alpha=.3)
for s in ("top", "right"): ax.spines[s].set_visible(False)
fig.tight_layout(); fig.savefig(OUT + "mj_obrada_os.png"); plt.close(fig)

# --- 2) tanhf u zavisnosti od G (obrada - BYPASS) -----------------------
G = [1, 1.5, 2, 8, 48]
tanh_obr = [112.0, 133.5, 140.3, 153.0, 104.3]
net = [t - 49 for t in tanh_obr]
fig, ax = plt.subplots(figsize=(7, 3.6))
ax.plot(G, net, "o-", color=C1, label="tanh u prekidu (obrada − BYPASS)")
ax.plot([1, 8, 64], [67.9, 103.7, 55.5], "s--", color=C2, label="tanhf benchmark")
ax.plot(G, [14] * len(G), "-", color="#888", lw=1, label="tvrdo odsijecanje (14, ne zavisi od G)")
ax.plot(G, [20] * len(G), ":", color="#888", lw=1, label="polinom (20, ne zavisi od G)")
ax.set_xscale("log"); ax.set_xticks([1, 2, 8, 48, 64]); ax.set_xticklabels(["1", "2", "8", "48", "64"])
ax.set_xlabel("pojačanje G"); ax.set_ylabel("ciklusa po odmjerku")
ax.set_title("Cijena nelinearnosti u zavisnosti od pojačanja G (fs = 48 kHz)")
ax.legend(frameon=False, fontsize=8); ax.grid(alpha=.3); ax.set_ylim(0, 130)
for s in ("top", "right"): ax.spines[s].set_visible(False)
fig.tight_layout(); fig.savefig(OUT + "mj_tanh_G.png"); plt.close(fig)

# --- 3) opterecenje (cijeli prekid / budzet), 48 vs 96 kHz -------------
# korigovano: cijeli prekid = obrada + a, a = 242 ciklusa (fiksna cijena prekida,
# iz stabilnih prozora: BYPASS 291.0 - 49 = 242, POLY 311.2 - 69 = 242)
A = 242
cases = ["HARD\nOS=1", "HARD\nOS=2", "TANH\nOS=2"]
l48 = [100 * (o + A) / 8333 for o in (63, 266, 446)]
l96 = [100 * (o + A) / 4166 for o in (63, 268, 447.8)]
x = np.arange(len(cases))
fig, ax = plt.subplots(figsize=(7, 3.6))
b1 = ax.bar(x - w/2, l48, w, label="fs = 48 kHz (budžet 8333)", color=C1)
b2 = ax.bar(x + w/2, l96, w, label="fs = 96 kHz (budžet 4166)", color=C2)
for b in list(b1) + list(b2):
    ax.text(b.get_x() + b.get_width()/2, b.get_height() + .2, f"{b.get_height():.2f} %",
            ha="center", va="bottom", fontsize=9)
ax.set_xticks(x, cases); ax.set_ylabel("opterećenje procesora [%]")
ax.set_title("Opterećenje: (obrada + fiksna cijena prekida 242) / budžet")
ax.legend(frameon=False); ax.set_ylim(0, 19); ax.grid(axis="y", alpha=.3)
for s in ("top", "right"): ax.spines[s].set_visible(False)
fig.tight_layout(); fig.savefig(OUT + "mj_opterecenje.png"); plt.close(fig)

# --- 4) predvidjeno vs izmjereno (benchmark) ----------------------------
blocks = ["tvrdo", "polinom", "tanhf\nG=1", "tanhf\nG=64", "interp.", "decim.", "OS+tvrdo", "OS+tanhf"]
lo = [8, 10, 60, 20, 30, 30, 80, 200]
hi = [15, 20, 120, 60, 45, 45, 120, 320]
me = [12.0, 17.0, 67.9, 55.5, 89.0, 82.0, 192.0, 372.3]
x = np.arange(len(blocks))
fig, ax = plt.subplots(figsize=(8, 3.8))
ax.bar(x, np.array(hi) - np.array(lo), 0.5, bottom=lo, color="#c9d6e8", label="predviđeni opseg")
ok = [l <= m <= h for l, m, h in zip(lo, me, hi)]
ax.scatter(x[ok], np.array(me)[ok], color=C1, zorder=3, label="izmjereno – u opsegu")
ax.scatter(x[~np.array(ok)], np.array(me)[~np.array(ok)], color="#c0392b", marker="D", zorder=3,
           label="izmjereno – van opsega")
for i, m in enumerate(me):
    ax.text(i + .28, m, f"{m:.0f}", va="center", fontsize=8)
ax.set_xticks(x, blocks, fontsize=8); ax.set_ylabel("ciklusa po odmjerku"); ax.set_yscale("log")
ax.set_title("Predviđanje i benchmark mjerenje po blokovima")
ax.legend(frameon=False, fontsize=8, loc="upper left"); ax.grid(axis="y", alpha=.3, which="both")
for s in ("top", "right"): ax.spines[s].set_visible(False)
fig.tight_layout(); fig.savefig(OUT + "mj_predvidjeno_izmjereno.png"); plt.close(fig)
print("ok")
