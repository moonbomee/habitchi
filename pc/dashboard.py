import csv
import os
import sys
from datetime import date, datetime, timedelta

try:
    import matplotlib.pyplot as plt
except ImportError:
    print("matplotlib이 없어요. 먼저 설치해 주세요:")
    print("  python3 -m pip install --user -r pc/requirements.txt")
    sys.exit(1)

from classify import load_config

HERE = os.path.dirname(os.path.abspath(__file__))
LOG_PATH = os.path.join(HERE, "data", "log.csv")
PNG_PATH = os.path.join(HERE, "data", "dashboard.png")
DAYS = 7
MAX_SLEEP_HOURS = 16


def game_time(seconds):
    return datetime(1970, 1, 1) + timedelta(seconds=seconds)


def night_of(moment):
    return (moment - timedelta(hours=12)).date()


def read_logs():
    logs = []
    if not os.path.exists(LOG_PATH):
        return logs
    with open(LOG_PATH, newline="", encoding="utf-8") as f:
        for row in csv.reader(f):
            if len(row) != 5 or row[1] != "LOG":
                continue
            try:
                moment = game_time(int(row[2]))
                value = float(row[4])
            except ValueError:
                continue
            logs.append((moment, row[3], value))
    logs.sort(key=lambda item: item[0])
    return logs


def sleep_hours(logs):
    hours = {}
    start = None
    for moment, kind, value in logs:
        if kind == "SLEEP_START":
            start = moment
        elif kind == "SLEEP_END":
            if value > 0:
                length = value / 60
            elif start is not None:
                length = (moment - start).total_seconds() / 3600
            else:
                length = 0
            if 0 < length <= MAX_SLEEP_HOURS:
                night = night_of(moment - timedelta(hours=length))
                hours[night] = hours.get(night, 0) + length
            start = None
    return hours


def sleep_scores(logs):
    scores = {}
    for moment, kind, value in logs:
        if kind == "SLEEP_SCORE":
            scores[night_of(moment)] = value
    return scores


def count_by(logs, wanted, key):
    counts = {}
    for moment, kind, value in logs:
        if kind == wanted:
            counts[key(moment)] = counts.get(key(moment), 0) + 1
    return counts


def average_by_day(logs, wanted, divisor):
    totals = {}
    for moment, kind, value in logs:
        if kind == wanted:
            day = moment.date()
            total, count = totals.get(day, (0.0, 0))
            totals[day] = (total + value / divisor, count + 1)
    return {day: total / count for day, (total, count) in totals.items()}


def draw(axis, title, dates, values, color):
    labels = [d.strftime("%m/%d") for d in dates]
    heights = [values.get(d, 0) for d in dates]
    axis.bar(labels, heights, color=color)
    axis.set_title(title)
    axis.tick_params(axis="x", labelsize=8)
    if not any(d in values for d in dates):
        axis.text(0.5, 0.5, "데이터 없음", transform=axis.transAxes, ha="center", va="center", color="gray")


def main():
    config = load_config()
    logs = read_logs()
    if not logs:
        print("기록이 아직 없어요: " + LOG_PATH)

    today = date.today()
    days = [today - timedelta(days=i) for i in range(DAYS - 1, -1, -1)]
    last_night = night_of(datetime.now())
    nights = [last_night - timedelta(days=i) for i in range(DAYS - 1, -1, -1)]

    hours = sleep_hours(logs)
    if hours:
        sleep_title, sleep_values = "밤마다 잠잔 시간 (시간)", hours
    else:
        sleep_title, sleep_values = "밤마다 수면 점수", sleep_scores(logs)

    plt.rcParams["font.family"] = ["AppleGothic", "DejaVu Sans"]
    plt.rcParams["axes.unicode_minus"] = False
    figure, axes = plt.subplots(2, 2, figsize=(10, 7))
    figure.suptitle("해빗치 주간 기록 (최근 7일)")

    draw(axes[0][0], sleep_title, nights, sleep_values, "#5470c6")
    draw(axes[0][1], "밤마다 큰 소음 횟수", nights, count_by(logs, "NOISE", night_of), "#ee6666")
    draw(axes[1][0], "하루 평균 온도 (°C)", days,
         average_by_day(logs, "TEMP", config["dashboard_temp_divisor"]), "#fac858")
    draw(axes[1][1], "하루 커밋 수", days, count_by(logs, "COMMIT", lambda m: m.date()), "#91cc75")

    figure.tight_layout()
    os.makedirs(os.path.dirname(PNG_PATH), exist_ok=True)
    figure.savefig(PNG_PATH)
    print("저장했어요: " + PNG_PATH)
    plt.show()


if __name__ == "__main__":
    main()
