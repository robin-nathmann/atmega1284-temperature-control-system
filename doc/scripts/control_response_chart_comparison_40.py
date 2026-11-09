import os
import re
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

# ================= KONFIGURATION =================
# Kontrastreiche Farben (Blau, Orange, Grün, Violett, Rot)
DATEIEN = [
    {
        "pfad": r"..\..\data\control_response_2point_target40.log",
        "label": "Zweipunktregler",
        "farbe": "#1f77b4",  # Kräftiges Blau
    },
    {
        "pfad": r"..\..\data\control_response_pi_compensation_tw10_target40.log",
        "label": "PI-Kompensationsregler (TW=10)",
        "farbe": "#ff7f0e",  # Leuchtendes Orange
    },
    {
        "pfad": r"..\..\data\control_response_pi_compensation_tw25_target40.log",
        "label": "PI-Kompensationsregler (TW=25)",
        "farbe": "#2ca02c",  # Kräftiges Grün
    },
    {
        "pfad": r"..\..\data\control_response_pi_compensation_tw100_target40.log",
        "label": "PI-Kompensationsregler (TW=100)",
        "farbe": "#9467bd",  # Violett
    },
    {
        "pfad": r"..\..\data\control_response_pi_zn_target40.log",
        "label": "PI-Regler nach Ziegler-Nichols",
        "farbe": "#d62728",  # Signalrot
    },
]

MAX_ZEIT = 3000                   # Maximale Zeit in Sekunden oder None für alle Werte

Y_MIN_TEMP = 20.0                 # Untere Grenze der Temperaturachse in °C
Y_MAX_TEMP = 42.0                 # Obere Grenze der Temperaturachse in °C

X_HAUPTSCHRITT = 500              # Beschriftete Hauptlinien auf der x-Achse (in Sekunden)
X_NEBENSCHRITT = 100              # Feine Zwischenlinien auf der x-Achse (in Sekunden)

# Einstellungen für den Detailausschnitt (Inset Plot)
INSET_X_MIN = 100                 # Startzeit des Zoom-Fensters in Sekunden
INSET_X_MAX = 2000                # Endzeit des Zoom-Fensters in Sekunden
INSET_Y_MIN = 37.5                # Untere Temperaturgrenze im Zoom-Fenster in °C
INSET_Y_MAX = 41.5                # Obere Temperaturgrenze im Zoom-Fenster in °C
# =================================================


def lese_logdatei(pfad):
    """Liest eine Logdatei ab dem ersten Zeitpunkt ein, an dem die Heizleistung ungleich 0% ist."""
    counts = []
    ist_temperaturen = []
    soll_temperaturen = []

    start_gefunden = False

    muster = re.compile(
        r"Count:(?P<count>\d+)\s+"
        r"Ist:(?P<ist>\d+)\s+"
        r"Soll:(?P<soll>\d+)\s+"
        r"(?P<prozent>[\d\.]+)%"
    )

    if not os.path.exists(pfad):
        print(f"Warnung: Datei '{pfad}' wurde nicht gefunden und wird übersprungen.")
        return None

    with open(pfad, "r", encoding="utf-8") as datei:
        for zeile in datei:
            treffer = muster.search(zeile)
            if not treffer:
                continue

            count = int(treffer.group("count"))
            ist_temp = float(treffer.group("ist")) / 100.0
            soll_temp = float(treffer.group("soll")) / 100.0
            prozent = float(treffer.group("prozent"))

            if not start_gefunden:
                if prozent != 0.0:
                    start_gefunden = True
                else:
                    continue

            counts.append(count)
            ist_temperaturen.append(ist_temp)
            soll_temperaturen.append(soll_temp)

    if not counts:
        print(f"Warnung: Keine relevanten Heizdaten in '{pfad}' gefunden.")
        return None

    return {
        "counts": counts,
        "ist": ist_temperaturen,
        "soll": soll_temperaturen,
    }


def gleiche_starttemperatur_an(datensaetze, max_zeit=None):
    """
    Ermittelt die höchste Starttemperatur aller Dateien und verschiebt alle anderen
    Messreihen zeitlich so, dass sie bei dieser Temperatur mit t = 0 starten.
    """
    if not datensaetze:
        return []

    ziel_start_temp = max(d["daten"]["ist"][0] for d in datensaetze)
    print(f"Abgleich auf gemeinsame Starttemperatur: {ziel_start_temp:.2f} °C")

    angeglichene_daten = []

    for eintrag in datensaetze:
        counts = eintrag["daten"]["counts"]
        ist = eintrag["daten"]["ist"]
        soll = eintrag["daten"]["soll"]

        start_idx = 0
        for i, temp in enumerate(ist):
            if temp >= ziel_start_temp:
                if i > 0 and abs(ist[i - 1] - ziel_start_temp) < abs(temp - ziel_start_temp):
                    start_idx = i - 1
                else:
                    start_idx = i
                break

        basis_count = counts[start_idx]
        zeiten_neu = []
        ist_neu = []
        soll_neu = []

        for c, temp_i, temp_s in zip(counts[start_idx:], ist[start_idx:], soll[start_idx:]):
            zeit_t = c - basis_count
            if max_zeit is not None and zeit_t > max_zeit:
                break
            zeiten_neu.append(zeit_t)
            ist_neu.append(temp_i)
            soll_neu.append(temp_s)

        if zeiten_neu:
            angeglichene_daten.append({
                "label": eintrag["label"],
                "farbe": eintrag.get("farbe"),
                "zeiten": zeiten_neu,
                "ist": ist_neu,
                "soll": soll_neu,
                "start_temp_original": ist[0],
                "start_temp_angeglichen": ist_neu[0],
            })

    return angeglichene_daten


def erstelle_diagramm(
    datensaetze, max_zeit=None,
    y_min_temp=18.0, y_max_temp=75.0,
    x_hauptschritt=500, x_nebenschritt=100
):
    if not datensaetze:
        print("Keine Daten zum Plotten vorhanden.")
        return

    fig, ax1 = plt.subplots(figsize=(12, 7))

    # Detailausschnitt (Inset Plot) im freien mittleren Bereich erstellen
    # Format: [x_links, y_unten, breite, hoehe] in relativen Achsenkoordinaten (0 bis 1)
    axins = ax1.inset_axes([0.1, 0.15, 0.85, 0.55])
    axins.set_facecolor("white")

    # Vorhandene Soll-Temperaturen als durchgezogene Linie plotten
    soll_werte = sorted({round(d["soll"][0], 1) for d in datensaetze if d["soll"]})
    for soll_wert in soll_werte:
        ax1.axhline(
            y=soll_wert,
            color="#333333",
            linestyle="--",
            linewidth=1.4,
            zorder=2,
            label=f"Soll-Temperatur ({soll_wert:.0f} °C)"
        )
        axins.axhline(
            y=soll_wert,
            color="#333333",
            linestyle="--",
            linewidth=1.2,
            zorder=2
        )

    # Alle Ist-Kurven in Hauptplot und Inset-Plot zeichnen
    max_gefundene_zeit = 0
    alle_ist_werte = []

    for i, d in enumerate(datensaetze):
        # Etwas dünnere Linien (1.5), damit sich überlappende Schwingungen nicht zu Kreisen formen
        ax1.plot(
            d["zeiten"],
            d["ist"],
            color=d["farbe"],
            linestyle="-",
            linewidth=1.5,
            zorder=3 + i,
            label=d["label"]
        )
        axins.plot(
            d["zeiten"],
            d["ist"],
            color=d["farbe"],
            linestyle="-",
            linewidth=1.5,
            zorder=3 + i
        )
        if d["zeiten"]:
            max_gefundene_zeit = max(max_gefundene_zeit, max(d["zeiten"]))
        alle_ist_werte.extend(d["ist"])

    ax1.set_xlabel("Zeit t in Sekunden (s)")
    ax1.set_ylabel("Temperatur in °C")
    ax1.tick_params(axis="both", which="both")

    # y-Achse Hauptdiagramm skalieren
    y1_min = min(y_min_temp, min(alle_ist_werte) - 2.0)
    y1_max = max(y_max_temp, max(alle_ist_werte) + 2.0)
    ax1.set_ylim(y1_min, y1_max)

    # Gitternetz Hauptdiagramm
    ax1.yaxis.set_major_locator(MultipleLocator(5))
    ax1.yaxis.set_minor_locator(MultipleLocator(1))
    ax1.xaxis.set_major_locator(MultipleLocator(x_hauptschritt))
    ax1.xaxis.set_minor_locator(MultipleLocator(x_nebenschritt))
    ax1.grid(True, which="major", axis="both", linestyle="-", linewidth=0.8, alpha=0.6)
    ax1.grid(True, which="minor", axis="both", linestyle="-", linewidth=0.3, alpha=0.3)

    if max_zeit is not None:
        ax1.set_xlim(0, max_zeit)
    else:
        ax1.set_xlim(0, max_gefundene_zeit)

    # Inset-Plot Achsen und Gitternetz konfigurieren
    axins.set_xlim(INSET_X_MIN, INSET_X_MAX)
    axins.set_ylim(INSET_Y_MIN, INSET_Y_MAX)
    axins.xaxis.set_major_locator(MultipleLocator(500))
    axins.xaxis.set_minor_locator(MultipleLocator(100))
    axins.yaxis.set_major_locator(MultipleLocator(1.0))
    axins.yaxis.set_minor_locator(MultipleLocator(0.5))
    axins.grid(True, which="major", axis="both", linestyle="-", linewidth=0.7, alpha=0.6)
    axins.grid(True, which="minor", axis="both", linestyle="-", linewidth=0.3, alpha=0.3)
    axins.tick_params(labelsize=9)

    # Rahmen und Verbindungslinien des Zoom-Bereichs (ebenfalls durchgezogen)
    ax1.indicate_inset_zoom(axins, edgecolor="#555555", linestyle="-", linewidth=1.0, alpha=0.8)

    # Legende unten rechts platzieren, damit sie nicht mit dem Inset-Plot kollidiert
    ax1.legend(loc="lower right", framealpha=0.95, ncol=3)
    plt.title("Vergleich des Regelverhaltens verschiedener Regler bei Soll-Temperatur von 40 Grad Celsius")
    plt.tight_layout()
    plt.savefig(r"..\figures\control_response_chart_comparison_40.png", dpi=300)
    plt.show()


if __name__ == "__main__":
    geladene_daten = []
    for eintrag in DATEIEN:
        rohdaten = lese_logdatei(eintrag["pfad"])
        if rohdaten is not None:
            geladene_daten.append({
                "label": eintrag["label"],
                "farbe": eintrag.get("farbe"),
                "daten": rohdaten,
            })

    angeglichene_daten = gleiche_starttemperatur_an(geladene_daten, max_zeit=MAX_ZEIT)

    erstelle_diagramm(
        angeglichene_daten,
        max_zeit=MAX_ZEIT,
        y_min_temp=Y_MIN_TEMP,
        y_max_temp=Y_MAX_TEMP,
        x_hauptschritt=X_HAUPTSCHRITT,
        x_nebenschritt=X_NEBENSCHRITT
    )