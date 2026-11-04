# PC 연결 (PcLink + `pc/`)

해빗치의 PC 쪽은 **사실만** 아두이노에 보내요. "키보드를 눌렀다", "커밋 메시지가 성의 없었다", "테스트가 실패했다" 같은 것만 보내고, 야식·밤샘·스트레칭 같은 판단은 모두 아두이노의 펫 모듈이 해요. 통신 형식은 `docs/protocol.md`를 따라요.

## 파일

| 파일 | 하는 일 |
|---|---|
| `firmware/habitchi/PcLink.cpp` | 아두이노 쪽. 시리얼로 PC 메시지를 받고, 기록(`LOG`)과 펫 상태(`STATE`)를 보내요 |
| `pc/habitchi_pc.py` | 브리지. 코딩하는 동안 터미널에 켜 두는 프로그램이에요 |
| `pc/classify.py` | 커밋 메시지를 `GOOD`(건강식) / `JUNK`(정크푸드)로 나눠요 |
| `pc/hooks/post-commit` | git 훅. 커밋할 때마다 메시지와 테스트 결과를 대기열 파일에 적어요 |
| `pc/install_hook.py` | 훅을 저장소에 설치해요 |
| `pc/dashboard.py` | 최근 7일 기록으로 그래프 4개를 그려요 |
| `pc/config.json` | 설정값 (아래 표) |
| `pc/requirements.txt` | 필요한 파이썬 패키지 |
| `pc/data/` | 브리지가 만드는 기록 폴더 (`log.csv`, `dashboard.png`) |

## 1. 설치

macOS에 기본으로 있는 파이썬 3.9로 돼요. `habitchi` 폴더에서 한 번만 실행해요.

```bash
cd ~/habitchi
python3 -m pip install --user -r pc/requirements.txt
```

설치되는 패키지는 세 개예요.
- `pyserial`: 아두이노와 USB 시리얼 통신
- `pynput`: 키보드를 누른 **시각** 알아내기
- `matplotlib`: 대시보드 그래프

## 2. 키보드 권한 (손쉬운 사용)

macOS는 다른 앱에서 누른 키를 프로그램이 알 수 있게 하려면 허락이 필요해요.

1. **시스템 설정 → 개인정보 보호 및 보안 → 손쉬운 사용**으로 가요.
2. 브리지를 실행하는 **터미널 앱**(터미널, iTerm, VS Code 등)을 켜요. 목록에 없으면 `+`로 추가해요.
3. 터미널을 완전히 껐다가 다시 켜요.

macOS 버전에 따라 **입력 모니터링** 항목에도 같은 앱을 켜야 할 수 있어요.

**개인정보:** 브리지는 **마지막으로 키를 누른 시각 하나만** 기억해요. 어떤 키를 눌렀는지는 저장하지도, 보내지도 않아요. 아두이노로 가는 것은 `EVT,TYPING` 한 줄뿐이에요.

## 3. 브리지 실행

아두이노 Mega를 USB로 꽂고 실행해요.

```bash
cd ~/habitchi
python3 pc/habitchi_pc.py
```

포트를 직접 고르고 싶으면 `--port`를 써요.

```bash
python3 pc/habitchi_pc.py --port /dev/cu.usbmodem14101
```

브리지가 하는 일이에요.
- `/dev/cu.usbmodem*`, `/dev/cu.usbserial*` 중에서 포트를 찾아 115200 baud로 열어요.
- 포트를 열면 Mega가 리셋되므로 **2초 기다린 뒤** `EVT,TIME,<시각>`을 보내요.
- 10초마다 `EVT,PING`을 보내요. 아두이노는 30초 동안 아무 메시지도 못 받으면 PC가 끊긴 것으로 봐요.
- 키를 누르고 있는 동안 **최대 1초에 한 번** `EVT,TYPING`을 보내요.
- 커밋 대기열 파일(`~/.habitchi/commits.jsonl`)에 새 줄이 생기면 `EVT,COMMIT,GOOD|JUNK|BROKEN`을 보내요. 테스트가 실패한 커밋은 메시지와 상관없이 `BROKEN`이에요.
- 아두이노가 보낸 `LOG`, `STATE` 줄을 `pc/data/log.csv`에 PC 시각과 함께 적어요.
- 터미널에 사건마다 짧은 줄을 찍어요 (시연할 때 확인용).
- USB를 뽑으면 2초마다 다시 찾아서 자동으로 다시 연결해요.
- `Ctrl+C`로 끝내요.

터미널 출력 예시예요.

```
[14:03:20] 연결 중: /dev/cu.usbmodem14101 (Mega가 리셋돼서 2초 기다려요)
[14:03:22] 연결됐어요. 시각을 보냈어요.
[14:03:30] 타이핑 감지
[14:04:02] 커밋 → JUNK  (demo) "fix" 테스트=none
[14:04:22] 펫 상태  단계=1 기분=0 포만=40 휴식=70 체력=80 경험치=12
```

`STATE`의 단계·기분 숫자는 `PetView.h`의 `Stage`, `Mood` 순서(0부터)예요.

### 시연 명령 (진화·아픔·죽음 보여 주기)
발표 중에 진화·아픔·죽음을 바로 보여 주려고, 브리지 터미널에 아래 명령을 치고 Enter를 누르면 아두이노로 보내요. 아두이노가 **시연 모드일 때만** 반응하고, 시연 모드가 아니면 무시해요.

| 입력 | 보내는 메시지 | 뜻 |
|---|---|---|
| `stage egg` / `baby` / `child` / `healthy` / `weak` / `obese` | `DEMO,STAGE,0` ~ `5` | 펫 단계를 바로 바꿔요 |
| `sick cold` / `sleep` / `hunger` / `overwork` | `DEMO,SICK,1` ~ `4` | 감기 / 수면 부족 / 굶주림 / 과로로 아프게 해요 |
| `evolve` | `DEMO,EVOLVE` | 지금까지의 기록으로 성장기 → 어른(건강형·허약형·비만형) 진화 판정을 바로 해요 (죽은 상태만 빼고 어느 단계에서든) |
| `die` | `DEMO,DIE` | 펫이 죽어요 |

- 대소문자는 상관없어요. 모르는 명령을 치면 사용법 한 줄을 보여 줘요.
- 아두이노가 연결돼 있지 않으면 보내지 않고 알려 줘요.
- 터미널에 키를 치는 것도 키보드 입력이라서 `EVT,TYPING`이 같이 가요.

### 보내는 시각에 대해
`EVT,TIME`에는 **한국 시각 기준의 unix초**(실제 unix초 + 9시간)를 보내요. 아두이노는 시간대를 모르기 때문에, 이렇게 보내야 `GameClock::hour()`가 바로 한국 시각을 돌려줘요. 대시보드도 같은 기준으로 `LOG`의 시각을 읽어요.

### 커밋 대기열에 대해
- 브리지를 켤 때 대기열에 이미 있던 커밋은 **보내지 않아요** (예전 커밋이 한꺼번에 밥이 되지 않게).
- 브리지는 켜져 있는데 아두이노만 빠져 있는 동안의 커밋은 기다렸다가 다시 연결되면 보내요.

### `pc/data/log.csv` 형식
제목 줄 없이 한 줄에 하나씩 적어요. 첫 칸은 PC 시각이고, 나머지는 아두이노가 보낸 줄 그대로예요.

```
2026-11-04 23:10:05,LOG,1793833805,SLEEP_START,0
2026-11-04 23:11:05,STATE,1,4,50,60,70,120
```

## 4. 커밋 메시지 분류 (`classify.py`)

다음 중 하나면 `JUNK`(정크푸드), 아니면 `GOOD`(건강식)이에요.
1. 앞뒤 공백을 지운 메시지가 **6글자보다 짧아요** (`min_message_length`).
2. 영어 글자나 **완성된 한글**(가~힣)이 하나도 없어요. 자음·모음만 있는 `ㅋㅋㅋㅋㅋㅋ`도 여기에 걸려요.
3. 메시지가 **성의 없는 단어 하나뿐**이에요 (`junk_words`). 대소문자는 구분하지 않고, 뒤에 붙은 숫자·문장부호·공백은 무시해요. 그래서 `fix2`, `update!!`, `Updated 3`도 JUNK예요.

`fix login bug`처럼 단어 뒤에 설명이 있으면 GOOD이에요. 메시지 하나를 직접 확인해 볼 수도 있어요.

```bash
python3 pc/classify.py "fix login bug"
```

## 5. git 훅 설치

커밋을 펫의 밥으로 쓰고 싶은 저장소마다 한 번씩 설치해요.

```bash
python3 pc/install_hook.py ~/my-project
```

- 훅은 `<저장소>/.git/hooks/post-commit`에 복사되고 실행 권한이 붙어요.
- 그 자리에 **다른 post-commit 훅이 이미 있으면 덮어쓰지 않고** 알려 줘요. 이때는 기존 훅을 직접 확인해서 합쳐야 해요.
- 훅은 커밋할 때마다 `~/.habitchi/commits.jsonl`에 한 줄을 붙여요.

```json
{"time":1793770641,"repo":"my-project","message":"add login page","tests":"pass"}
```

- `rebase` 중에 만들어지는 커밋은 기록하지 않아요. (`commit --amend`는 새 커밋으로 기록돼요.)

### 테스트 명령 정하기 (`.habitchi-test`)
저장소 맨 위 폴더에 `.habitchi-test` 파일을 만들고 **첫 줄에 테스트 명령**을 써요.

```bash
echo "python3 -m pytest -q" > ~/my-project/.habitchi-test
```

- 명령이 성공(종료 코드 0)하면 `pass`, 실패하면 `fail`이에요.
- **30초**(훅 맨 위 `LIMIT`) 안에 끝나지 않으면 멈추고 `fail`로 봐요.
- 파일이 없으면 테스트를 돌리지 않고 `tests`는 `none`이에요.
- 테스트가 도는 동안 `git commit`이 기다려요. 그래서 테스트는 빨라야 해요.
- 테스트는 커밋된 내용이 아니라 **지금 폴더에 있는 파일**로 돌아요.

## 6. 시연용 저장소 만들기 ("상한 음식" 시연)

테스트를 일부러 실패시킬 수 있는 아주 작은 저장소예요. 테스트는 1초도 안 걸려요.

```bash
mkdir ~/habitchi-demo && cd ~/habitchi-demo
git init
echo "exit 0" > check.sh
echo "sh check.sh" > .habitchi-test
git add . && git commit -m "first commit"
python3 ~/habitchi/pc/install_hook.py ~/habitchi-demo
```

시연 순서예요 (브리지는 켜 둔 상태).

```bash
echo a >> note.txt && git add . && git commit -m "fix"
echo b >> note.txt && git add . && git commit -m "메모 파일에 내용 추가"
echo "exit 1" > check.sh && git add . && git commit -m "검사 스크립트 변경"
echo "exit 0" > check.sh && git add . && git commit -m "검사 스크립트 되돌리기"
```

1. `fix` → `JUNK` (정크푸드)
2. 제대로 된 메시지 → `GOOD` (건강식)
3. `check.sh`가 1로 끝나서 테스트 실패 → `BROKEN` (상한 음식, 배탈)
4. 다시 0으로 → `GOOD`

## 7. 대시보드

```bash
python3 pc/dashboard.py
```

`pc/data/log.csv`에서 최근 7일을 읽어 그래프 4개를 한 창에 그리고, `pc/data/dashboard.png`로도 저장해요.

| 그래프 | 어떻게 구하나요 |
|---|---|
| 밤마다 잠잔 시간 | `SLEEP_END`의 값(잔 시간, 분)을 시간으로 바꿔요. 값이 0이면 `SLEEP_START`부터 `SLEEP_END`까지 걸린 시간을 써요. 둘 다 없으면 대신 `SLEEP_SCORE`(수면 점수, 0~100)를 그려요 |
| 밤마다 큰 소음 횟수 | `NOISE` 기록 줄의 개수 (잠든 동안 큰 소리 한 번에 한 줄) |
| 하루 평균 온도 | `TEMP` 값의 하루 평균. 아두이노는 온도×10(215 = 21.5℃)을 보내므로 `dashboard_temp_divisor`(10)로 나눠요 |
| 하루 커밋 수 | `COMMIT` 기록 줄의 개수 (건강식·정크·빌드 깨짐을 모두 합쳐요) |

- "밤"은 정오부터 다음 날 정오까지예요. 11월 3일 밤 11시에 자서 11월 4일 아침에 깼다면 11월 3일 밤으로 쳐요.
- 시각은 아두이노의 게임 시각이에요. 시연 모드에서 시계를 건너뛴 기록도 그 시각으로 그려져요.
- 기록이 없는 칸은 비워 두고, 그래프에 데이터가 하나도 없으면 "데이터 없음"이라고 써요.

## 8. 설정 (`pc/config.json`)

| 키 | 기본값 | 뜻 |
|---|---|---|
| `baud` | 115200 | 시리얼 속도 (아두이노와 같아야 해요) |
| `reset_wait_s` | 2 | 포트를 연 뒤 기다리는 초 |
| `ping_interval_s` | 10 | `EVT,PING` 간격 |
| `typing_interval_s` | 1 | `EVT,TYPING` 최소 간격 |
| `commit_queue` | `~/.habitchi/commits.jsonl` | 훅이 적는 대기열 파일 (훅 안의 경로와 같아야 해요) |
| `min_message_length` | 6 | 이보다 짧은 메시지는 JUNK |
| `junk_words` | fix, update, wip, 수정, ㅁㄴㅇㄹ … | 이 단어 하나뿐이면 JUNK |
| `dashboard_temp_divisor` | 10 | 대시보드에서 `TEMP` 값을 나눌 수 (온도×10이라서) |

## 9. 아두이노 쪽 (`PcLink.cpp`)

- `update()`가 부를 때마다 시리얼에 쌓인 글자를 **최대 64개** 읽어서 줄 버퍼(48칸)에 모아요. 기다리지 않으니 `loop()`가 멈추지 않아요. 너무 긴 줄은 버려요.
- 한 줄이 완성되면 처리해요.
  - 어떤 줄이든 받으면 `facts.pcConnected = true`이고, `Cfg::PC_TIMEOUT_MS`(30초) 동안 아무것도 안 오면 `false`가 돼요.
  - `EVT,TYPING` → `facts.lastTypingMs = millis()`, `facts.typingSeen = true`
  - `EVT,COMMIT,GOOD|JUNK|BROKEN` → 이벤트 `CommitGood` / `CommitJunk` / `CommitBroken`을 넣어요. 먹을지 말지, 야식인지는 펫 모듈이 정해요.
  - `EVT,TIME,<초>` → `GameClock::syncFromPc(초)`
  - `DEMO,STAGE,<0~5>` / `DEMO,SICK,<1~4>` / `DEMO,DIE` / `DEMO,EVOLVE` → **시연 모드일 때만** `Ev::DemoStage`(값 = 단계) / `Ev::DemoSick`(값 = 원인) / `Ev::DemoDie` / `Ev::DemoEvolve`를 넣어요. 범위를 벗어난 숫자는 무시해요.
- `PcLink::log(종류, 값)`은 `LOG,<GameClock::now()>,<종류>,<값>`을 보내요. 다른 모듈은 `Serial`을 직접 쓰지 말고 이 함수를 불러요.
- 60초마다 `STATE,<stage>,<mood>,<fullness>,<rest>,<stamina>,<exp>`를 보내요 (enum은 숫자로).

## 10. 문제 해결

**"아두이노를 찾는 중이에요..."만 계속 나와요**
- USB 케이블이 **충전 전용**이 아닌지 확인해요 (데이터 선이어야 해요).
- `ls /dev/cu.*`로 포트가 보이는지 확인하고, 보이면 `--port`로 직접 넣어요.
- Arduino IDE의 **시리얼 모니터가 열려 있으면** 포트를 같이 쓸 수 없어요. 닫아요.
- 업로드 중에도 포트를 쓸 수 없어요. 업로드가 끝나면 브리지가 알아서 다시 연결해요.

**연결할 때마다 펫이 처음부터 시작하는 것처럼 보여요**
- 정상이에요. Mega는 포트가 열리면 **리셋**돼요. 그래서 브리지가 2초 기다린 뒤 시각을 보내요. 펫 상태는 EEPROM, 시각은 RTC에 남아 있어요.
- 처음 몇 줄이 깨져 보여도 무시해요.

**키보드를 쳐도 "타이핑 감지"가 안 나와요**
- 2번의 손쉬운 사용 권한을 다시 확인하고 터미널을 껐다 켜요.

**커밋해도 반응이 없어요**
- 그 저장소에 훅을 설치했는지 확인해요 (`python3 pc/install_hook.py <저장소>`).
- `cat ~/.habitchi/commits.jsonl`에 줄이 생기는지 봐요. 생기면 훅은 정상이고, 브리지가 켜져 있는지 확인해요.
- 브리지를 켜기 **전에** 한 커밋은 보내지 않아요.

**대시보드 글자가 네모로 나와요**
- 맥의 `AppleGothic` 글꼴을 써요. 다른 컴퓨터에서는 한글 글꼴 이름을 `dashboard.py`의 `font.family`에 넣어요.
