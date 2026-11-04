# 해빗치 코드 구조와 작업 규칙

## 폴더
| 폴더 | 내용 |
|---|---|
| `firmware/habitchi/` | 아두이노 스케치. Arduino IDE에서 `habitchi.ino`를 열면 같은 폴더의 `.h/.cpp`가 모두 함께 컴파일됨 |
| `pc/` | PC 프로그램 (파이썬): 시리얼 연결, 키보드 감지, 커밋 훅, 대시보드 |
| `docs/` | 모듈별 설명서, 배선 안내, 통신 형식, 시연 순서 |

## 모듈과 담당 (파일 소유권)
한 파일은 한 패키지만 고친다. 다른 패키지의 파일을 고쳐야 하면 직접 고치지 말고 조율자(Claude)에게 요청한다.

| 패키지 | 파일 | 하는 일 |
|---|---|---|
| 공용 (조율자) | `habitchi.ino`, `Pins.h`, `Timing.h`, `Facts.h/.cpp`, `Events.h/.cpp`, `PetView.h`, 모든 패키지의 공개 헤더(`GameClock.h`, `Input.h`, `Sensors.h`, `Sound.h`, `Display.h`, `PcLink.h`, `Pet.h`) | 핀 번호, 시간 설정, 모듈 사이에 주고받는 데이터 형식 |
| 센서 | `Sensors.cpp`, `Sensor*.h/.cpp`, `SensorsConfig.h` | 조도, 소리, 온도, 파란 LED, 폰 거치대(압력 + 피에조), 물병(호일), 심박 → `facts`와 이벤트로 전달 |
| 시계·입력 | `GameClock.cpp`, `Input.cpp`, `ClockConfig.h` | RTC, 시연 모드 가상 시계, 버튼 |
| 펫 | `Pet.cpp`, `Pet*.h/.cpp`, `Storage.h/.cpp`, `PetConfig.h` | 상태 수치, 규칙, 진화, 아픔·죽음, EEPROM 저장 |
| 화면·소리 | `Display.cpp`, `Display*.h/.cpp`, `Sprites.h`, `Sound.cpp` | OLED 표정·화면, 피에조 효과음 |
| PC 연결 | `PcLink.cpp`, `pc/` 전체 | 시리얼 통신, 키보드 감지, 커밋 분류·테스트 결과, 대시보드 |

## 모듈끼리 주고받는 방법
- **`facts` (Facts.h):** 센서와 PC 연결이 "지금 상태"를 적어 두는 곳. 펫이 읽기만 한다.
- **이벤트 (Events.h):** 한 번 일어난 일(커밋, 큰 소리, 폰 진동, 물 마심, 운동 시작·끝)을 넣는 큐. **꺼내는 쪽은 펫 하나뿐**이다.
- **`Pet::view()` (PetView.h):** 펫이 화면에 보여 줄 상태. 화면 모듈이 읽는다.
- **`GameClock`:** 모든 모듈이 "지금 몇 시인지"는 여기서만 얻는다. 시연 모드면 가상 시각이 나온다.
- **`Sound::busy()`:** 펫 소리가 나는 동안 소리 센서가 그 소리를 소음으로 세지 않게 하는 데 쓴다.
- **판단은 펫 모듈이 한다.** 센서와 PC는 사실만 전달한다 (예: "어둡다", "커밋이 JUNK였다"). "야식이다", "밤샘이다" 같은 판단은 펫 모듈만 한다.

## 코드 규칙
1. **주석을 쓰지 않는다.** 설명은 `docs/`의 설명서에 쓴다.
2. **`delay()` 금지.** `millis()`로 시간을 재고, 오래 걸리는 측정(파란 LED 방전, 호일 정전용량)도 한 번에 끝내지 말고 여러 `loop()`에 나눠서 한다.
3. **시리얼 출력은 `PcLink`만** 한다. 다른 모듈은 `PcLink::log()`를 부른다.
4. **핀 번호는 `Pins.h`에서만** 가져온다. 숫자를 코드에 직접 쓰지 않는다.
5. **설정값(기준값, 시간)은 패키지별 `*Config.h`에** 모아 두고, 일반 모드와 시연 모드 값이 다르면 `Timing`과 `pick()`을 쓴다.
6. **타이머 충돌 주의:** `tone()`이 Timer2를 쓰므로 Timer2를 쓰는 라이브러리(PulseSensor Playground의 인터럽트 모드 등)는 쓰지 않는다.
7. **구현하기로 한 기능만 만든다.** 기능표(`HABITCHI_AI_CONTEXT.md` §3)에 없는 기능은 넣지 않는다.

## 컴파일 확인
```bash
"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" compile --fqbn arduino:avr:mega ~/habitchi/firmware/habitchi
```
