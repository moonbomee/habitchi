import argparse
import csv
import glob
import json
import os
import queue
import sys
import threading
import time

try:
    import serial
    from pynput import keyboard
except ImportError:
    print("필요한 패키지가 없어요. 먼저 설치해 주세요:")
    print("  python3 -m pip install --user -r pc/requirements.txt")
    sys.exit(1)

from classify import classify, load_config

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(HERE, "data")
LOG_PATH = os.path.join(DATA_DIR, "log.csv")
PORT_PATTERNS = ["/dev/cu.usbmodem*", "/dev/cu.usbserial*"]
RETRY_S = 2
TYPING_NOTE_GAP_S = 10
DEMO_STAGES = ["egg", "baby", "child", "healthy", "weak", "obese"]
DEMO_SICK = ["cold", "sleep", "hunger", "overwork"]
DEMO_HELP = ("명령: stage egg|baby|child|healthy|weak|obese / sick cold|sleep|hunger|overwork"
             " / evolve / die  (아두이노가 시연 모드일 때만 반응해요)")


class KeyActivity:
    def __init__(self):
        self.last_press = 0.0

    def on_press(self, key):
        self.last_press = time.time()


def say(text):
    print(time.strftime("[%H:%M:%S]"), text, flush=True)


def local_unix_now():
    return int(time.time()) + time.localtime().tm_gmtoff


def find_port(wanted):
    if wanted:
        if os.path.exists(wanted):
            return wanted
        return None
    for pattern in PORT_PATTERNS:
        found = sorted(glob.glob(pattern))
        if found:
            return found[0]
    return None


def open_board(port, config):
    board = serial.Serial(port, config["baud"], timeout=0.05)
    time.sleep(config["reset_wait_s"])
    board.reset_input_buffer()
    return board


def send(board, text):
    board.write((text + "\n").encode("ascii"))


def save_line(fields):
    os.makedirs(DATA_DIR, exist_ok=True)
    with open(LOG_PATH, "a", newline="", encoding="utf-8") as f:
        csv.writer(f).writerow([time.strftime("%Y-%m-%d %H:%M:%S")] + fields)


def handle_board_line(text):
    fields = text.split(",")
    if fields[0] == "LOG" and len(fields) == 4:
        save_line(fields)
        say("기록  " + fields[2] + " = " + fields[3])
    elif fields[0] == "STATE" and len(fields) == 7:
        save_line(fields)
        say("펫 상태  단계=" + fields[1] + " 기분=" + fields[2] + " 포만=" + fields[3]
            + " 휴식=" + fields[4] + " 체력=" + fields[5] + " 경험치=" + fields[6])


def read_board(board, buffer):
    data = board.read(board.in_waiting or 1)
    if not data:
        return buffer
    buffer += data
    while b"\n" in buffer:
        raw, buffer = buffer.split(b"\n", 1)
        text = raw.decode("ascii", errors="replace").strip()
        if text:
            handle_board_line(text)
    return buffer


class CommitQueue:
    def __init__(self, path):
        self.path = os.path.expanduser(path)
        self.offset = self.size()

    def size(self):
        try:
            return os.path.getsize(self.path)
        except OSError:
            return 0

    def new_commits(self):
        size = self.size()
        if size < self.offset:
            self.offset = 0
        if size == self.offset:
            return []
        with open(self.path, "rb") as f:
            f.seek(self.offset)
            chunk = f.read(size - self.offset)
        end = chunk.rfind(b"\n")
        if end < 0:
            return []
        self.offset += end + 1
        commits = []
        for raw in chunk[:end].split(b"\n"):
            text = raw.decode("utf-8", errors="replace").strip()
            if not text:
                continue
            try:
                commits.append(json.loads(text))
            except ValueError:
                say("커밋 기록 한 줄을 읽지 못해서 건너뛰어요: " + text[:60])
        return commits


def demo_message(text):
    words = text.lower().split()
    if words == ["die"]:
        return "DEMO,DIE"
    if words == ["evolve"]:
        return "DEMO,EVOLVE"
    if len(words) == 2 and words[0] == "stage" and words[1] in DEMO_STAGES:
        return "DEMO,STAGE," + str(DEMO_STAGES.index(words[1]))
    if len(words) == 2 and words[0] == "sick" and words[1] in DEMO_SICK:
        return "DEMO,SICK," + str(DEMO_SICK.index(words[1]) + 1)
    return None


def read_typed_lines(lines):
    for text in sys.stdin:
        lines.put(text.strip())


def start_typed_reader():
    lines = queue.Queue()
    reader = threading.Thread(target=read_typed_lines, args=(lines,), daemon=True)
    reader.start()
    return lines


def handle_typed(lines, board):
    while not lines.empty():
        text = lines.get()
        if not text:
            continue
        message = demo_message(text)
        if message is None:
            say(DEMO_HELP)
        elif board is None:
            say("아두이노가 연결되지 않아서 보내지 못했어요: " + message)
        else:
            send(board, message)
            say("시연 명령 보냄 → " + message)


def commit_kind(commit, config):
    if commit.get("tests") == "fail":
        return "BROKEN"
    return classify(str(commit.get("message", "")), config)


def run(args):
    config = load_config()
    activity = KeyActivity()
    listener = keyboard.Listener(on_press=activity.on_press)
    listener.start()
    listener.wait()
    if getattr(listener, "IS_TRUSTED", True) is False:
        say("키보드 감지 권한이 없어요. 손쉬운 사용 권한을 확인해 주세요 (docs/pc.md).")

    commits = CommitQueue(config["commit_queue"])
    say("커밋 대기열: " + commits.path)
    say("기록 파일: " + LOG_PATH)
    typed = start_typed_reader()
    say(DEMO_HELP)

    board = None
    buffer = b""
    waiting_said = False
    next_try = 0.0
    last_ping = 0.0
    last_typing_sent = 0.0
    last_commit_check = 0.0

    try:
        while True:
            now = time.time()

            try:
                handle_typed(typed, board)
            except (serial.SerialException, OSError) as error:
                say("보내지 못했어요: " + str(error))

            if board is None:
                if now < next_try:
                    time.sleep(0.1)
                    continue
                next_try = now + RETRY_S
                port = find_port(args.port)
                if port is None:
                    if not waiting_said:
                        say("아두이노를 찾는 중이에요... (USB 연결을 확인해 주세요)")
                        waiting_said = True
                    continue
                try:
                    say("연결 중: " + port + " (Mega가 리셋돼서 " + str(config["reset_wait_s"]) + "초 기다려요)")
                    board = open_board(port, config)
                    send(board, "EVT,TIME," + str(local_unix_now()))
                    buffer = b""
                    last_ping = time.time()
                    waiting_said = False
                    say("연결됐어요. 시각을 보냈어요.")
                except (serial.SerialException, OSError) as error:
                    say("연결 실패: " + str(error))
                    board = None
                continue

            try:
                buffer = read_board(board, buffer)
                now = time.time()

                if now - last_ping >= config["ping_interval_s"]:
                    send(board, "EVT,PING")
                    last_ping = now

                pressed = activity.last_press
                if pressed > last_typing_sent and now - last_typing_sent >= config["typing_interval_s"]:
                    if now - last_typing_sent >= TYPING_NOTE_GAP_S:
                        say("타이핑 감지")
                    send(board, "EVT,TYPING")
                    last_typing_sent = now

                if now - last_commit_check >= 1:
                    last_commit_check = now
                    for commit in commits.new_commits():
                        kind = commit_kind(commit, config)
                        send(board, "EVT,COMMIT," + kind)
                        message = str(commit.get("message", "")).splitlines()
                        first = message[0][:40] if message else ""
                        say("커밋 → " + kind + "  (" + str(commit.get("repo", "?")) + ") \"" + first
                            + "\" 테스트=" + str(commit.get("tests", "none")))
            except (serial.SerialException, OSError) as error:
                say("연결이 끊겼어요: " + str(error))
                try:
                    board.close()
                except (serial.SerialException, OSError):
                    pass
                board = None
                next_try = time.time() + RETRY_S
    except KeyboardInterrupt:
        print()
        say("종료해요.")
    finally:
        listener.stop()
        if board is not None:
            try:
                board.close()
            except (serial.SerialException, OSError):
                pass


def main():
    parser = argparse.ArgumentParser(description="해빗치 PC 연결 프로그램")
    parser.add_argument("--port", help="시리얼 포트 (예: /dev/cu.usbmodem14101). 없으면 자동으로 찾아요.")
    run(parser.parse_args())


if __name__ == "__main__":
    main()
