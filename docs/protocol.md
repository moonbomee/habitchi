# PC ↔ 아두이노 통신 형식

- USB 시리얼, 115200 baud.
- 한 줄에 메시지 하나이고, 줄 끝은 `\n`, 값은 쉼표로 구분한다.
- 포트를 열면 Mega가 리셋되므로 PC는 연 뒤 2초 기다리고 보낸다.

## PC → 아두이노 (사실만 보낸다)
| 메시지 | 언제 |
|---|---|
| `EVT,PING` | 10초마다. 아두이노는 30초 동안 아무 메시지가 없으면 PC 연결이 끊긴 것으로 본다 |
| `EVT,TYPING` | 키보드를 누르고 있는 동안 최대 1초에 한 번 |
| `EVT,COMMIT,GOOD` | 메시지를 제대로 쓴 커밋 |
| `EVT,COMMIT,JUNK` | 성의 없는 메시지의 커밋 |
| `EVT,COMMIT,BROKEN` | 테스트·빌드가 실패한 커밋 (메시지와 상관없이 BROKEN이 우선) |
| `EVT,TIME,<unix초>` | 연결할 때 한 번. 아두이노 RTC 시각 맞추기 |
| `DEMO,STAGE,<0~5>` | 시연용. 펫 단계를 바로 바꾼다 (`Stage` 순서: 0 Egg, 1 Baby, 2 Child, 3 Healthy, 4 Weak, 5 Obese) |
| `DEMO,SICK,<1~4>` | 시연용. 아프게 만든다 (1 감기, 2 수면 부족, 3 굶주림, 4 과로 — `LOG,SICK` 값과 같다) |
| `DEMO,DIE` | 시연용. 펫이 죽는다 |
| `DEMO,EVOLVE` | 시연용. 지금까지의 기록으로 어른 진화 판정을 바로 한다 |

- `DEMO,...`는 브리지 터미널에 사람이 직접 입력한 시연 명령이다. 아두이노는 **시연 모드(`GameClock::demoMode()`)일 때만** 받아서 `Ev::DemoStage`(값 = 단계), `Ev::DemoSick`(값 = 원인), `Ev::DemoDie`, `Ev::DemoEvolve` 이벤트로 넣고, 시연 모드가 아니면 무시한다.

## 아두이노 → PC
| 메시지 | 뜻 |
|---|---|
| `LOG,<unix초>,<종류>,<값>` | 기록. 종류: `SLEEP_START`, `SLEEP_END`, `SLEEP_SCORE`, `NOISE`, `TEMP`, `PHONE`, `WATER`, `HEART`, `COMMIT`, `STRETCH`, `SICK`, `DEATH` 등 |
| `STATE,<stage>,<mood>,<fullness>,<rest>,<stamina>,<exp>` | 60초마다 펫 상태 |

- `<unix초>`는 아두이노의 게임 시각(`GameClock::now()`)이다.
- **시각 기준:** PC는 `EVT,TIME`에 **한국 시각 기준 unix초**(실제 unix초 + 9시간)를 보낸다. 아두이노는 시간대 계산 없이 이 값을 그대로 RTC에 넣고 `hour()`를 구한다. `LOG` 시각도 같은 기준이다.

### LOG 종류와 값 (펫 모듈이 보낸다)
| 종류 | 값 | 언제 |
|---|---|---|
| `SLEEP_START` | 0 | 잠든 것으로 판단했을 때 |
| `SLEEP_END` | 잔 시간(분) | 깨어난 것으로 판단했을 때 |
| `SLEEP_SCORE` | 0~100 | 아침에 수면 점수를 냈을 때 |
| `NOISE` | 소음 크기(0~100) | 잠든 동안 큰 소리가 날 때마다 한 줄 |
| `TEMP` | 방 온도 ×10 (예: 215 = 21.5℃) | 30분(게임 시각)마다 |
| `COMMIT` | 0 = 건강식, 1 = 정크, 2 = 빌드 깨짐 | 커밋 이벤트를 처리할 때 |
| `PHONE` | 1 = 올려 둠, 0 = 들어 올림 | 폰 두기 모드에서 상태가 바뀔 때 |
| `WATER` | 마신 양(%) | 물을 마셨다고 판단했을 때 |
| `HEART` | 심박(bpm) | 운동 전후 측정이 끝났을 때 |
| `STRETCH` | 쉬지 않고 코딩한 분 | 스트레칭 알림을 띄울 때 |
| `SICK` | 1 = 감기, 2 = 수면 부족, 3 = 굶주림, 4 = 과로 | 아픔이 시작될 때 |
| `DEATH` | 죽은 펫의 세대 수 | 죽었을 때 |
- PC는 `LOG`와 `STATE`를 CSV로 저장해서 대시보드를 그린다.
