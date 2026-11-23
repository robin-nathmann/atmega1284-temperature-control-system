import re
import glob
import os
import statistics

LINE_RE = re.compile(r"Count:(\d+)\s+Ist:(-?\d+)\s+Soll:(-?\d+)\s+([\d.]+)%")
BAND = 0.1
MIN_HOLD = 300
WIN_LEN = 500


def load_log(path):
    t, temp, pwm, soll = [], [], [], None
    with open(path, encoding="utf-8", errors="ignore") as f:
        for line in f:
            m = LINE_RE.search(line)
            if m:
                t.append(int(m.group(1)))
                temp.append(int(m.group(2)) / 100.0)
                soll = int(m.group(3)) / 100.0
                pwm.append(float(m.group(4)))
    # Start ab erstem PWM > 0 %
    i0 = next((i for i, p in enumerate(pwm) if p > 0), 0)
    return t[i0:], temp[i0:], pwm[i0:], soll


def median3(vals):
    if len(vals) < 3:
        return vals[:]
    return (
        [vals[0]]
        + [statistics.median(vals[i - 1 : i + 2]) for i in range(1, len(vals) - 1)]
        + [vals[-1]]
    )


def eval_group(pattern):
    files = sorted(glob.glob(pattern))
    if not files:
        print(f"Keine Dateien für Muster: {pattern}")
        return

    loaded = [(f, *load_log(f)) for f in files]
    ref_temp = max(temp[0] for _, _, temp, _, _ in loaded)

    # 1) Alle Kurven auf die gemeinsame Starttemperatur normieren
    normalized = []
    for path, t_raw, temp_raw, pwm_raw, soll in loaded:
        idx0 = next((i for i, v in enumerate(temp_raw) if v >= ref_temp), 0)
        t0 = t_raw[idx0]
        t_norm = [ti - t0 for ti in t_raw[idx0:]]
        normalized.append((path, t_norm, temp_raw[idx0:], pwm_raw[idx0:], soll))

    # 2) Kürzeste normierte Messdauer der Gruppe als Basis bestimmen
    shortest = min(normalized, key=lambda item: item[1][-1])
    horizon = shortest[1][-1]
    limiting_file = os.path.basename(shortest[0])
    win_start = max(0, horizon - WIN_LEN)

    print(
        f"\n================ {pattern} ================\n"
        f"Ref-Start: {ref_temp:.2f} °C | Basis-Fenster: 0..{horizon} s "
        f"(begrenzt durch {limiting_file}) | Endfenster: {win_start}..{horizon} s"
    )

    # 3) Alle Dateien auf diesem gemeinsamen Zeitfenster auswerten
    for path, t_norm, temp_norm, pwm_norm, soll in normalized:
        idx = [i for i, ti in enumerate(t_norm) if ti <= horizon]
        t = [t_norm[i] for i in idx]
        temp_slice = [temp_norm[i] for i in idx]
        pwm_slice = [pwm_norm[i] for i in idx]

        temp = median3(temp_slice)  # unterdrückt 1-s-Einzelausreißer
        spikes = sum(1 for a, b in zip(temp_slice, temp) if abs(a - b) > 0.15)

        t_an = next((ti for ti, v in zip(t, temp) if v >= soll - BAND), None)
        overshoot = max(0.0, max(temp) - soll)

        last_out = None
        for i in range(len(t) - 1, -1, -1):
            if abs(temp[i] - soll) > BAND:
                last_out = i
                break

        if last_out is None:
            t_aus = t[0]
        elif last_out + 1 < len(t) and (t[-1] - t[last_out + 1]) >= MIN_HOLD:
            t_aus = t[last_out + 1]
        else:
            t_aus = None

        win_vals = [v for ti, v in zip(t, temp) if ti >= win_start]
        win_pwm = [p for ti, p in zip(t, pwm_slice) if ti >= win_start]
        mean_err = soll - statistics.mean(win_vals)
        v_pp = max(win_vals) - min(win_vals)
        mean_pwm = statistics.mean(win_pwm)

        print(f"{os.path.basename(path)} (norm. Gesamtdauer: {t_norm[-1]} s):")
        print(
            f"  t_an={t_an} s | Overshoot={overshoot:.2f} K | t_aus={t_aus} s | "
            f"e_mean={mean_err:+.2f} K | pp={v_pp:.2f} K | PWM={mean_pwm:.1f} % | "
            f"1s-Spikes(>0.15K)={spikes}"
        )


eval_group(r"..\..\data\control_response_*_target70.log")
eval_group(r"..\..\data\control_response_*_target40.log")