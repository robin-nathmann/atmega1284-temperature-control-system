import re
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

# ================= KONFIGURATION =================
DATEIPFAD = r"..\..\data\control_response_pi_zn_target70.log"
ZEIGE_HEIZLEISTUNG = True         # True = 2. y-Achse rechts anzeigen, False = ausblenden
MAX_ZEIT = 3000                   # Maximale Zeit in Sekunden (z.B. 3000 bis 5000) oder None

TEMP_BEI_0_PROZENT = 20.0         # Temperaturhöhe (in °C), auf der 0% Heizleistung liegen soll
TEMP_BEI_100_PROZENT = 70.0       # Temperaturhöhe (in °C), auf der 100% Heizleistung liegen soll

X_HAUPTSCHRITT = 500              # Beschriftete Hauptlinien auf der x-Achse (in Sekunden)
X_NEBENSCHRITT = 100              # Feine Zwischenlinien auf der x-Achse (in Sekunden)

# --- INSET-PLOT KONFIGURATION ---
ZEIGE_INSET = True                # True = Zoom-Fenster anzeigen, False = ausblenden
INSET_AUTOMATISCH = False          # True = Zoom automatisch auf das Erreichen der Soll-Temperatur legen
INSET_X_BEREICH = (700, 2400)     # Wird genutzt, falls INSET_AUTOMATISCH = False
INSET_Y_BEREICH = (68.5, 70.5)    # Wird genutzt, falls INSET_AUTOMATISCH = False
INSET_POSITION = [0.15, 0.125, 0.8, 0.3]  # [x0, y0, Breite, Höhe] relativ (0.0 bis 1.0)
# =================================================


def lese_logdatei(pfad, max_zeit=None):
    zeiten = []
    ist_temperaturen = []
    soll_temperaturen = []
    heizleistungen = []

    start_count = None

    muster = re.compile(
        r"Count:(?P<count>\d+)\s+"
        r"Ist:(?P<ist>\d+)\s+"
        r"Soll:(?P<soll>\d+)\s+"
        r"(?P<prozent>[\d\.]+)%"
    )

    with open(pfad, "r", encoding="utf-8") as datei:
        for zeile in datei:
            treffer = muster.search(zeile)
            if not treffer:
                continue

            count = int(treffer.group("count"))
            ist_temp = float(treffer.group("ist")) / 100.0
            soll_temp = float(treffer.group("soll")) / 100.0
            prozent = float(treffer.group("prozent"))

            if start_count is None:
                if prozent != 0.0:
                    start_count = count
                else:
                    continue

            zeit_t = count - start_count

            if max_zeit is not None and zeit_t > max_zeit:
                break

            zeiten.append(zeit_t)
            ist_temperaturen.append(ist_temp)
            soll_temperaturen.append(soll_temp)
            heizleistungen.append(prozent)

    return zeiten, ist_temperaturen, soll_temperaturen, heizleistungen


def erstelle_diagramm(
    zeiten, ist_temp, soll_temp, heizleistung,
    zeige_leistung=True, max_zeit=None,
    temp_bei_0_prozent=20.0, temp_bei_100_prozent=70.0,
    x_hauptschritt=500, x_nebenschritt=100,
    zeige_inset=True, inset_auto=True,
    inset_x=(800, 2000), inset_y=(68.0, 73.0),
    inset_pos=[0.48, 0.15, 0.45, 0.42]
):
    if not zeiten:
        print("Keine relevanten Daten gefunden (Heizleistung war nie ungleich 0%).")
        return

    fig, ax1 = plt.subplots(figsize=(12, 7))

    linie_soll = ax1.plot(
        zeiten, soll_temp, color="tab:red", linestyle=":", linewidth=2, zorder=3, label="Soll-Temperatur (°C)"
    )
    linie_ist = ax1.plot(
        zeiten, ist_temp, color="tab:red", linewidth=2, zorder=2, label="Ist-Temperatur (°C)"
    )

    ax1.set_xlabel("Zeit t in Sekunden (s)")
    ax1.set_ylabel("Temperatur in °C", color="tab:red")
    ax1.tick_params(axis="y", which="both", labelcolor="tab:red")
    ax1.tick_params(axis="x", which="both")

    y1_min, y1_max = ax1.get_ylim()
    y1_min = min(y1_min, temp_bei_0_prozent - 2.0)
    y1_max = max(y1_max, temp_bei_100_prozent + 2.0)
    ax1.set_ylim(y1_min, y1_max)

    ax1.yaxis.set_major_locator(MultipleLocator(5))
    ax1.yaxis.set_minor_locator(MultipleLocator(1))
    ax1.xaxis.set_major_locator(MultipleLocator(x_hauptschritt))
    ax1.xaxis.set_minor_locator(MultipleLocator(x_nebenschritt))

    ax1.grid(True, which="major", axis="both", linestyle="-", linewidth=0.8, alpha=0.6)
    ax1.grid(True, which="minor", axis="both", linestyle=":", linewidth=0.5, alpha=0.4)

    if max_zeit is not None:
        ax1.set_xlim(0, max_zeit)
    else:
        ax1.set_xlim(0, max(zeiten))

    linien = linie_ist + linie_soll

    if zeige_leistung:
        ax2 = ax1.twinx()
        linie_pwm = ax2.plot(
            zeiten, heizleistung, color="tab:blue", alpha=1, label="Heizleistung (%)"
        )
        ax2.set_ylabel("Heizleistung in %", color="tab:blue")
        ax2.tick_params(axis="y", which="both", labelcolor="tab:blue")

        temp_spanne = temp_bei_100_prozent - temp_bei_0_prozent
        y2_min = (y1_min - temp_bei_0_prozent) / temp_spanne * 100.0
        y2_max = (y1_max - temp_bei_0_prozent) / temp_spanne * 100.0
        ax2.set_ylim(y2_min, y2_max)

        ax2.set_yticks(range(0, 101, 10))
        ax2.yaxis.set_minor_locator(MultipleLocator(1))

        ax1.set_zorder(ax2.get_zorder() + 1)
        ax1.patch.set_visible(False)

        linien += linie_pwm

    # ================= INSET-PLOT (NUR TEMPERATUR-ZOOM) =================
    if zeige_inset:
        axins = ax1.inset_axes(inset_pos)
        axins.set_in_layout(False)    # Verhindert die tight_layout-Warnung
        axins.set_facecolor("white")  # Weißer Hintergrund überdeckt das Hauptgitter

        # Nur Soll- und Ist-Temperatur im Zoom darstellen
        axins.plot(zeiten, soll_temp, color="tab:red", linestyle=":", linewidth=1.8, zorder=3)
        axins.plot(zeiten, ist_temp, color="tab:red", linewidth=1.8, zorder=2)

        if inset_auto:
            ziel_soll = soll_temp[-1]
            # Ersten Zeitpunkt suchen, an dem die Ist-Temperatur bis auf 2 °C an den Sollwert herankommt
            nahe_soll_zeiten = [t_val for t_val, temp in zip(zeiten, ist_temp) if temp >= ziel_soll - 2.0]
            if nahe_soll_zeiten:
                x_start = max(0, nahe_soll_zeiten[0] - 100)
                x_ende = min(zeiten[-1], x_start + 1200)
            else:
                x_start, x_ende = inset_x

            max_ist = max(ist_temp)
            y_start = ziel_soll - 2.0
            y_ende = max(ziel_soll + 2.0, max_ist + 0.8)
        else:
            x_start, x_ende = inset_x
            y_start, y_ende = inset_y

        axins.set_xlim(x_start, x_ende)
        axins.set_ylim(y_start, y_ende)

        axins.yaxis.set_major_locator(MultipleLocator(1))
        axins.yaxis.set_minor_locator(MultipleLocator(0.5))
        axins.xaxis.set_major_locator(MultipleLocator(200))
        axins.xaxis.set_minor_locator(MultipleLocator(50))
        axins.grid(True, which="major", linestyle="-", linewidth=0.6, alpha=0.6)
        axins.grid(True, which="minor", linestyle=":", linewidth=0.4, alpha=0.4)
        axins.tick_params(axis="y", labelcolor="tab:red", labelsize=8)
        axins.tick_params(axis="x", labelsize=8)

        ax1.indicate_inset_zoom(axins, edgecolor="black", alpha=0.7, linewidth=1.2)
    # ====================================================================

    labels = [l.get_label() for l in linien]
    ax1.legend(linien, labels, loc="lower right", framealpha=0.95, ncol=3)

    # Titel explizit auf ax1 setzen statt plt.title()
    ax1.set_title(
        "Regelverhalten des PI-Reglers nach Ziegler-Nichols bei einer Soll-Temperatur von 70 Grad Celsius mit Heizleistung",
        pad=12
    )
    plt.tight_layout()
    plt.savefig(r"..\figures\control_response_chart_pi_zn_70.png", dpi=300)
    plt.show()


if __name__ == "__main__":
    t, ist, soll, pwm = lese_logdatei(DATEIPFAD, max_zeit=MAX_ZEIT)
    erstelle_diagramm(
        t, ist, soll, pwm,
        zeige_leistung=ZEIGE_HEIZLEISTUNG,
        max_zeit=MAX_ZEIT,
        temp_bei_0_prozent=TEMP_BEI_0_PROZENT,
        temp_bei_100_prozent=TEMP_BEI_100_PROZENT,
        x_hauptschritt=X_HAUPTSCHRITT,
        x_nebenschritt=X_NEBENSCHRITT,
        zeige_inset=ZEIGE_INSET,
        inset_auto=INSET_AUTOMATISCH,
        inset_x=INSET_X_BEREICH,
        inset_y=INSET_Y_BEREICH,
        inset_pos=INSET_POSITION
    )