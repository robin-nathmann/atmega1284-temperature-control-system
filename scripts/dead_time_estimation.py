import re


def berechne_totzeiten(dateipfad, ignoriere_erste_n=20):
    daten = []
    muster = re.compile(r"Count:(\d+)\s+Ist:(\d+).*?\s+([\d\.]+)%")

    # 1. Daten einlesen und parsen
    with open(dateipfad, 'r', encoding='utf-8') as file:
        for zeile in file:
            match = muster.search(zeile)
            if match:
                count = int(match.group(1))
                ist_temp = int(match.group(2))
                pwm = float(match.group(3))
                daten.append({"count": count, "ist": ist_temp, "pwm": pwm})

    if not daten:
        print("Keine gültigen Messdaten gefunden. Bitte das Dateiformat prüfen.")
        return

    # 2. Daten in Phasen unterteilen
    phasen = []
    aktuelle_phase = [daten[0]]

    for i in range(1, len(daten)):
        if daten[i]["pwm"] == aktuelle_phase[-1]["pwm"]:
            aktuelle_phase.append(daten[i])
        else:
            phasen.append(aktuelle_phase)
            aktuelle_phase = [daten[i]]

    # --- NEU: Listen für die Extrema und deren Counts ---
    # Listen für 100% -> 0% (Heizung AUS, Nachheizen)
    dt_100_auf_0 = []
    max_temperaturen = []
    max_counts = []

    # Listen für 0% -> 100% (Heizung EIN, Nachkühlen)
    dt_0_auf_100 = []
    min_temperaturen = []
    min_counts = []

    # 3. Phasen auswerten
    for i in range(1, len(phasen)):
        vorherige_phase = phasen[i - 1]
        aktuelle_phase = phasen[i]

        pwm_vorher = vorherige_phase[-1]["pwm"]
        pwm_aktuell = aktuelle_phase[0]["pwm"]
        start_count = aktuelle_phase[0]["count"]

        # Fall 1: Heizung schaltet ab (100% -> 0%)
        if pwm_vorher >= 50.0 and pwm_aktuell < 50.0:
            max_ist = max(punkt["ist"] for punkt in aktuelle_phase)

            extremum_count = start_count
            for punkt in aktuelle_phase:
                if punkt["ist"] == max_ist:
                    extremum_count = punkt["count"]

            # Werte in die Listen speichern
            dt_100_auf_0.append(extremum_count - start_count)
            max_temperaturen.append(max_ist)
            max_counts.append(extremum_count)

        # Fall 2: Heizung schaltet ein (0% -> 100%)
        elif pwm_vorher < 50.0 and pwm_aktuell >= 50.0:
            min_ist = min(punkt["ist"] for punkt in aktuelle_phase)

            extremum_count = start_count
            for punkt in aktuelle_phase:
                if punkt["ist"] == min_ist:
                    extremum_count = punkt["count"]

            # Werte in die Listen speichern
            dt_0_auf_100.append(extremum_count - start_count)
            min_temperaturen.append(min_ist)
            min_counts.append(extremum_count)

    # 4. Erste Erebnisse abschneiden
    eingeschwungen_100_0 = dt_100_auf_0[ignoriere_erste_n:] if len(dt_100_auf_0) > ignoriere_erste_n else dt_100_auf_0
    eingeschwungen_0_100 = dt_0_auf_100[ignoriere_erste_n:] if len(dt_0_auf_100) > ignoriere_erste_n else dt_0_auf_100

    max_totzeit_100_0 = max(eingeschwungen_100_0) if eingeschwungen_100_0 else 0
    max_totzeit_0_100 = max(eingeschwungen_0_100) if eingeschwungen_0_100 else 0

    # 5. Ergebnisse ausgeben
    print("=== Ergebnisse der Totzeit-Analyse ===")
    print(f"Gefundene Schaltvorgänge gesamt: {len(dt_100_auf_0) + len(dt_0_auf_100)}")
    print(f"Ignorierte Einschwingvorgänge  : {ignoriere_erste_n} pro Flanke")
    print("-" * 60)

    print("100% -> 0% (Heizung AUS, System heizt nach):")
    print(f"Totzeiten [s]     : {dt_100_auf_0}")
    print(f"Max. Temperaturen : {max_temperaturen}")
    print(f"Count der Maxima  : {max_counts}")
    if dt_100_auf_0:
        print(f"Ø Totzeit (gesamt): {sum(dt_100_auf_0) / len(dt_100_auf_0):.2f} s")
    print(f"MAX Totzeit (eingeschwungen): {max_totzeit_100_0} s\n")

    print("0% -> 100% (Heizung EIN, System kühlt noch ab):")
    print(f"Totzeiten [s]     : {dt_0_auf_100}")
    print(f"Min. Temperaturen : {min_temperaturen}")
    print(f"Count der Minima  : {min_counts}")
    if dt_0_auf_100:
        print(f"Ø Totzeit (gesamt): {sum(dt_0_auf_100) / len(dt_0_auf_100):.2f} s")
    print(f"MAX Totzeit (eingeschwungen): {max_totzeit_0_100} s")


# Aufruf
if __name__ == '__main__':
    berechne_totzeiten('../data/dead_time_target70.log', ignoriere_erste_n=0)