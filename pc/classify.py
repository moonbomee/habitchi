import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CONFIG_PATH = os.path.join(HERE, "config.json")


def load_config():
    with open(CONFIG_PATH, encoding="utf-8") as f:
        return json.load(f)


def has_letter(text):
    for ch in text:
        if "a" <= ch.lower() <= "z":
            return True
        if "가" <= ch <= "힣":
            return True
    return False


def strip_tail(text):
    return re.sub(r"[\W\d_]+$", "", text)


def classify(message, config=None):
    if config is None:
        config = load_config()
    text = message.strip()
    if len(text) < config["min_message_length"]:
        return "JUNK"
    if not has_letter(text):
        return "JUNK"
    junk_words = [word.lower() for word in config["junk_words"]]
    lowered = text.lower()
    if lowered in junk_words or strip_tail(lowered) in junk_words:
        return "JUNK"
    return "GOOD"


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("사용법: python3 classify.py \"커밋 메시지\"")
        sys.exit(1)
    print(classify(" ".join(sys.argv[1:])))
