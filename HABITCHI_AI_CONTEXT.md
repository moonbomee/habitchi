# HABITCHI — Project Context for AI Assistants

> This file gives an AI assistant everything it needs to help with this project without re-asking.
> Write all answers to the team **in Korean**. Treat this file as the single source of truth. **[결정됨]** = settled; don't re-propose alternatives unless asked. **[미정]** = open.
> Last updated: 2026-11-04 (concept line added). Matches the final deck **`~/habitchi/마녀와야수들_수정본_v2.key`** (Keynote, 11 slides).

---

## 0. TL;DR
- **What:** "해빗치 (Habitchi)", a desk-top physical Tamagotchi on an **Arduino Mega 2560**.
  - Subtitle: "생활습관을 함께 돌보는 피지컬 다마고치". Core message: **"내가 건강해야 펫도 건강하다"**.
  - **Concept line (slide 4):** "오래 앉아 있는 만큼, 그 시간을 책임져 주는 책상 위 다마고치". Adults sit about 9 h/day (slide 2). Habitchi does not measure sitting; it looks after the habits formed during that desk time (sleep environment, coding).
  - The pet's health reflects habits that are **actually measured**: the user's **sleep environment** and **coding habits**. Commits are the pet's food. Nothing relies on the user pressing "I did it".
- **Arduino vs PC:** the **Arduino makes every decision**. The PC only sends facts: commit message quality, test result, keyboard activity. The sleep-environment features work with the PC off.
- **Team:** "마녀와 야수들", **3 members** (문보미, 김도훈, 조채원). Other teams have 4.
- **Course:** 부산대 정보컴퓨터공학부 「어드벤처디자인」 003분반 (Prof. 안성용, TA 김의현). Freshman level.
- **Constraints:**
  - No soldering: jumper wires + hot glue only.
  - One devicemart order totaling **₩58,976**.
  - About 4 weeks of build time.
- **Grading:** the submitted deck is the contract. The final evaluation scores **what is implemented + implementation difficulty**. Extra unlisted work earns credit, but missing a listed **필수** feature hurts. So 필수 = only things we are sure we can build; everything uncertain is marked **(확장)**.

## 1. Course context
- **Project weight:** 190 of 1000 points.
  - Other items: attendance 100, LAB/Quiz/HW 410, final exam 300.
  - Missing the project = F.
- **Lab topics so far:**

  | Week | Topic |
  |---|---|
  | 1 | Microcontrollers |
  | 2 | Arduino basics |
  | 3 | Digital I/O |
  | 4 | Analog I/O |
  | 6 | Speaker & sensors (`tone()`, `pitches.h`, LM35, CdS) |
  | 7 | 7-segment & 16×2 Text LCD |
  | 8 | Motors (Servo, DC + L293D) |
  | 9 | Serial |

- **Lab pin conventions (Mega):**
  - A0 = 54, A1 = 55, A2 = 56, A3 = 57.
  - Speaker on A3; LM35 on A1; CdS on A2.
- **Dates:**

  | When | What |
  |---|---|
  | 2026-11-02 | Proposal presentation (done) |
  | Week of 11-03 | Revised deck submitted as final |
  | Weeks 12–15 | Build |
  | **Week 16 (≈12/21)** | Final evaluation |
  | 12/11 (Fri) | Final exam (TA notice) |

## 2. Professor feedback that shaped the plan [결정됨]
1. **Measure, don't tap.** A desk-type device must use more actually measured data, not buttons or tags; otherwise make it portable. Pick one, even with fewer features.
   - We kept **desk-type + real measurement**. No portable unit, no Bluetooth.
2. **Use sensors creatively,** not just for their intended purpose. His example: a smart bin measured fullness with a fan and air pressure instead of a distance sensor.
   - Our 확장 features: a blue LED as a light sensor, foil strips as a water-level sensor, a piezo as a vibration sensor.
3. **Plan a demo scenario that actually works,** especially for temperature. See §8.
4. **"The Arduino has no role"** — feedback given to another team.
   - In Habitchi, all judgment runs on the Arduino; the PC only reports facts.

## 3. Features [결정됨] — slide 6, four rows
"(확장)" marks the part of a row that is not promised.

| Habit | Measured by | Pet reaction | Tier |
|---|---|---|---|
| **수면 환경** | lights-off/on time (CdS + RTC) → real sleep duration; night noise (sound sensor); room temperature (LM35). (확장) screen glow after lights-off via a **blue LED used as a light sensor** | sleeps and recovers when the environment is right on time; wake alarm; poor sleep if noisy; **cold** if sleeping in a cold room. (확장) poor sleep if a screen was used before sleep | 필수·확장 |
| **코딩 습관** | coding detected by the PC + room brightness (CdS) → dark-room coding; **continuous coding time** (PC keyboard); commits/keyboard after bedtime (PC). (확장) phone holder (**FSR pressure + piezo vibration**); water bottle level (**foil capacitance**) | sore eyes in the dark; **"스트레칭하자!" after 50 min**; sleepy and less EXP when pulling an all-nighter. (확장) less EXP if the phone is lifted without a notification; thirsty if no drinking while coding | 필수·확장 |
| **식사 = 커밋** | the PC classifies the commit message; the Arduino judges time and interval. (확장) the test/build result after a commit (PC, via git hook) | proper commit = healthy food; careless = junk; after bedtime = late-night snack. (확장) broken build = spoiled food → **stomachache** (pained sound and motion, small EXP) | 필수·확장 |
| 운동 | heart rate before vs after exercise (pulse sensor) | stamina ↑ only if HR really rose | 확장 |

### 3.1 Detailed rules
- **Continuous coding:**
  - Counts how long PC keyboard input continues. Over **50 min** → stretch reminder.
  - **No keyboard for ≥5 min** = a break, and the counter resets.
  - Ignoring the reminder and coding on drains stamina.
- **Phone holder (확장):**
  - FSR module = phone present. A second kit piezo (with 1 MΩ in parallel) = notification vibration.
  - After a vibration, lifting the phone is allowed for **5 min**.
  - Lifting it without a notification → EXP gain reduced.
- **Water (확장):**
  - Foil strips on the walls of a bottle holder, read with the **CapacitiveSensor library**: send pin → 1 MΩ → receive pin + foil.
  - **Disable auto-calibration** with `set_CS_AutocaL_Millis(0xFFFFFFFF)`, or slow level changes get erased.
  - Measure only when the reading is stable (the bottle is on the stand, no hand). Detect drinking as "lifted and put back with a lower level".
  - Thirsty if no drinking during a continuous coding session.
- **Screen glow (확장):**
  - Reverse-bias the LED, switch the cathode to INPUT, and time the discharge: more light → faster.
  - An LED responds only to light at or below its own wavelength, so a blue LED reacts to blue screen light. Use a clear-lens LED.
  - **Promise only "screen glow after lights-off".** White LED lamps also emit blue, so no lamp-vs-screen discrimination.
- **Fallbacks (internal only, not in the deck):** screen glow → plain CdS; water → drop it.

### 3.2 Commit = food

| Type | Rule | Decided by | Effect |
|---|---|---|---|
| 건강식 | proper message | PC (text only) | fullness+, EXP+ |
| 정크푸드 | "fix", "asdf", "ㅁㄴㅇㄹ" | PC (text only) | fullness+, raises obesity risk |
| 야식 | after bedtime | Arduino (RTC vs bedtime) | indigestion, sleep score ↓, less EXP |
| 과식 | many commits in a short time | Arduino (interval + fullness) | diminishing effect, "배불러…" |
| 상한 음식 (확장) | tests/build failed | PC runs the tests via a git hook (command set in config) and sends only the result | **stomachache**: pained sound/motion, small EXP; fullness still rises |

- **No starving:** fullness decays slowly; a rest-day setting exists; decay pauses if the PC has been disconnected for a long time.

### 3.3 Removed — don't re-propose
| Removed | Why / replaced by |
|---|---|
| Sitting/posture and screen distance (HC-SR04) | can't tell a chair back from a person; too many cases → replaced by continuous coding time |
| Eye fatigue (screen-side CdS) | too many light sensors |
| Servo | the pet lives on the screen |
| RFID food cards | tapping a card isn't eating → commit = food |
| Exercise timer, BMI input, supplement reminder, meal windows | self-reported input |
| Weather/fine dust | no Arduino role; it appears only under 향후 발전 on slide 9 |
| Ventilation/CO2 | a real CO2 sensor costs ₩27k–47k; the cheap MQ-135 isn't CO2 |

## 4. Pet model (slide 7)
**Stats (0–100):**

| Stat | Field | Up / recover | Down |
|---|---|---|---|
| 포만감 | `fullness` | commits | slow decay |
| 수면 | `rest` | sleeping on time in a quiet, warm room | late sleep, all-nighters, noise |
| 체력 | `stamina` | good sleep, exercise (확장) | all-nighters, colds, coding without breaks |
| **경험치** | `exp` | accumulates with commits | gain reduced by late-night commits, stomachache, lifting the phone without a notification |

- `exp` replaced the old "행복" stat. It does not decay.

**Stages:** 알 → 아기 → 성장기 → evolve (thresholds are proposals, tune in testing):
- **비만형** if the junk + late-night commit ratio is > 0.5.
- else **건강형** if sleep, coding habits and commits are balanced (e.g. average sleep score ≥ 70 and no long colds).
- else **허약형** (sleep debt, colds).

**Sickness and death [결정됨]:**
- **Sick** comes from exactly four causes:

  | Cause | When |
  |---|---|
  | 감기 | sleeping in a cold room on consecutive nights |
  | 수면 부족 | `rest` stays at 0 |
  | 굶주림 | `fullness` stays at 0 |
  | 과로 | `stamina` stays at 0 |

- These are only momentary reactions, not sickness: stomachache, indigestion, sleepy, sore eyes, thirsty.
- **Death:** sick for **1 day** without recovering (about 24 min in demo mode).
  - The timer pauses while the power is off or on a rest day.
  - After death: a tombstone screen, then **restart from an egg**. Keep the **generation count** and the previous pet's lifespan in EEPROM ("3대째 해빗치").
  - Provide a developer flag that disables death during testing.
- Slide 7 box: "아프면 — 감기·수면 부족·굶주림·과로로 아픈 채 1일이 지나면 죽음 → 알로 다시 시작 (세대 기록)".

**Moods (display):** NORMAL, HUNGRY, SLEEPY, SLEEPING, STUDYING (phone down), SORE_EYES, STRETCH, STOMACHACHE, THIRSTY, SICK, DEAD.

## 5. Hardware

### 5.1 Already owned (course kit) — do NOT buy
- Arduino Mega 2560 + USB cable, breadboard, a few jumper wires.
- LEDs, 10 kΩ resistors, tact buttons.
- **Two passive piezos:** one for sound, one as the phone-vibration sensor.
- **LM35.**
- **One CdS** (room light — the only CdS in the project).

### 5.2 Shopping list — devicemart, one order [결정됨]
Prices include VAT, checked 2026-11-03/04. Product URL: `https://www.devicemart.co.kr/goods/view?no=<no>`.

| Item [code] | Use | Qty | ₩ | no |
|---|---|---|---|---|
| OLED 0.96" SSD1306 I2C [VLT-OL022] | pet face | 1 | 7,040 | 15960989 |
| DS3231 RTC [SZH-EK047] (ZS-042) | real time | 1 | 5,280 | 1280312 |
| CR2032 | RTC backup | 1 | 990 | 2931 |
| Sound sensor, LM386, analog [SEN050302] | night noise (relative loudness); overseas, 3–4 days | 1 | 5,170 | 1289314 |
| FSR pressure sensor module [KS0309] | phone holder (확장); overseas, ~1 week, **non-returnable** | 1 | 9,020 | 15526390 |
| Pulse sensor kit [SZH-HWS008] | exercise check (확장) | 1 | 4,950 | 12319052 |
| Blue LED 5 mm, clear [5BB4SC00] | screen glow (확장) | 2 | 440 | 189 |
| 1/4W 1 MΩ resistor [105J] (min. order 20) | foil sensor, piezo load (확장) | 20 | 506 | 816 |
| Jumper cables M/M, M/F ×2, F/F [CH254] | wiring | 4 | 3,740 | 1321196 / 1321195 / 1321192 |
| U-shaped jumper wire kit [OR0014] | breadboard wires | 1 | 3,520 | 1329627 |
| M3 supports + screws + nuts | mount the Mega | 10 each | 1,100 | 3160 / 12677 / 34319 |
| Mini breadboard 170 | inside the case | 1 | 880 | 1329504 |
| Cable ties 100 mm | cable management | 1 | 550 | 15290068 |
| Plastic case 230×150×83 [BB161] | enclosure | 1 | 13,090 | 10914016 |

- **Total:** parts ₩56,276 + shipping ₩2,700 = **₩58,976**.
  - Sensors subtotal (slide 10): ₩33,396.
  - Wiring and assembly (slide 11): ₩22,880.
- **Check on arrival:**
  - The sound sensor's pot: `analogRead` must follow sound.
  - The FSR: a clear difference between phone on and phone off.
  - The blue LED, foil and piezo can be tested early with kit parts.

### 5.3 Pin map (proposal)

| Part | Pin (Mega) | Notes |
|---|---|---|
| OLED (0x3C) + DS3231 (0x68; AT24C32 at 0x57) | SDA 20, SCL 21 | RTC on 3.3V (the ZS-042 charging circuit is meant for a rechargeable cell) |
| Sound sensor AO | A0 | 5V |
| LM35 | A1 | °C = mV / 10 |
| CdS room | A2 | 10 kΩ divider |
| Piezo (sound) | A3 | `tone()` |
| FSR module | A4 | 5V |
| Piezo (vibration sensor) | A5 | 1 MΩ in parallel to bleed charge |
| Pulse sensor | A6 | 5V |
| Buttons A/B/C | 24, 25, 26 | `INPUT_PULLUP` |
| Blue LED sensor | anode D30, cathode D31 | series resistor |
| Foil (CapacitiveSensor) | send D32 → 1 MΩ → receive D33 + foil | — |
| PC | USB Serial, 115200 | — |

- Wire colors: red = 5V, black = GND, orange = 3.3V.

### 5.4 Enclosure (no solder) [결정됨]
- **Case:** BB161, internal size ≈ 222×142×75 mm. Mega on M3 supports; back row Mega + half breadboard; front strip mini breadboard, RTC, piezo.
- **Lid:** OLED window, 3 buttons, room CdS.
- **Outside the case:**
  - LM35 and the sound sensor, so the case doesn't distort temperature or muffle sound.
  - Phone holder (FSR under the pad + vibration piezo).
  - Bottle holder with foil.
  - Blue LED facing the bed.
- **Fixing:** hot-glue the jumper connectors, cable ties, strain relief on USB.

## 6. Software design

### 6.1 Principles [결정됨]
- No `delay()`; `millis()` scheduling.
- Each module has `setup<Module>()` / `update<Module>()`. `loop()` calls the updates → `updatePet()` → `render()`.
- **All judgments on the Arduino.**
- Write the state/transition table first.
- Smooth noisy readings (sound, light, pressure) with a moving average; use hysteresis thresholds.

### 6.2 Libraries
- **Arduino:** `Wire`, `EEPROM`, Adafruit SSD1306 + GFX (or U8g2 for Korean), RTClib, **CapacitiveSensor**, `pitches.h`.
- **PC (Python 3):**
  - `pyserial`
  - `pynput` (store only the time of the last key press, never the keys; macOS needs the Accessibility permission)
  - `git log` via `subprocess`, or the GitHub API
  - a git `post-commit` hook that runs the configured test command (확장)
  - `matplotlib` (dashboard)

### 6.3 Serial protocol (115200 baud, ASCII lines ending in `\n`)
- **Arduino → PC:**
  - `LOG,<t>,SLEEP,<start|end>,<v>`
  - `LOG,<t>,NOISE,<count>`
  - `LOG,<t>,TEMP,<c>`
  - `LOG,<t>,PHONE,<up|down>`
  - `STATE,<stage>,<mood>,<fullness>,<rest>,<stamina>,<exp>` — every 60 s
- **PC → Arduino:**
  - `EVT,COMMIT,GOOD|JUNK|BROKEN`
  - `EVT,TYPING` (rate-limited, e.g. once per minute while typing)
  - `EVT,TIME,<unix>` (optional)
- The PC never sends judgments (late-night snack, all-nighter, stretch due).
- Opening the port resets the Mega; the PC waits ~2 s. State survives in EEPROM and time in the RTC.

### 6.4 EEPROM
- **Struct:** `{magic, version, stage, fullness, rest, stamina, exp, sickSince, generation, lastLifespanDays, sleepScoreAvg, junkCount, lateCount, commitCount, coldNights, bedtime, wakeTime, restDay}`.
- **When to save:** every ~10 game minutes and on stage, sickness, death or settings changes.
- Use `EEPROM.put`; never write in every loop.

## 7. Demo mode [internal plan — not in the deck]
- **Virtual clock:** `gameTime = RTC + offset`, so the RTC keeps real time.
  - Buttons: long-press A → time-set; B = +1 h; C = +10 min.
  - Optional jumps: 22:59, 02:00, 06:59, 14:00.
- Game time ×60 with a "DEMO" badge. Leaving demo mode clears the offset.
- Death timer in demo mode ≈ 24 min. Use the dev flag to disable death while testing.

## 8. Demo scenario (draft)
1. **Bedtime:** jump to 22:59 and cover the room CdS → the pet falls asleep.
2. **Night:** clap or play sound → noise count rises, poor sleep.
3. **Cold room:** a cold can on the LM35 → sneezing, then a cold; warm it by hand → recovery. This makes the temperature demo possible on stage.
4. **Morning:** jump to 06:59 and uncover → wake alarm and sleep score.
5. **Coding:**
   - Type continuously → after 50 game minutes (~50 s) "스트레칭하자!"; stop typing → reset.
   - Commit "fix" → junk. Commit a proper message → healthy food. Commit after bedtime → late-night snack with less EXP. (확장) A failing test → stomachache.
6. **(확장) Phone:** put the phone on the holder; a notification vibration then a pickup → allowed; a pickup without a notification → less EXP.
7. **Sickness and death:** show a pet in the sick state recovering (keep death disabled during the demo unless it is shown on purpose).

## 9. Roles (proposal)
- **A · Pet core:** OLED faces, stats/evolution/sickness/death, sound, EEPROM, demo mode.
- **B · Sensors:** CdS + RTC sleep, sound, LM35. 확장: FSR + piezo phone holder, blue LED, foil, pulse sensor.
- **C · PC:** serial, commit classification, keyboard activity time, (확장) git-hook test results, dashboard.
- **A + C together:** the Arduino-side rules (late-night snack, overeating, all-nighter, continuous coding).

## 10. Schedule

| Week | Work |
|---|---|
| 10–11 | Each module standalone. Before delivery, practise with kit parts: LM35, CdS, LED, piezo |
| 12–13 | Integrate; state machine complete |
| 14 | PC link + 확장 features |
| 15 | Case, wiring, demo rehearsal |
| 16 | Final evaluation |

## 11. Files (`~/habitchi/`)

| File | What it is |
|---|---|
| **`마녀와야수들_수정본_v2.key`** | **Final deck** (Keynote, 11 slides) |
| `마녀와야수들_수정본.key` | Previous version (still has sitting/posture); superseded |
| `팀원공유_기획변경_v2.md` | Korean explanation for teammates (same content as this file) |
| `HABITCHI_AI_CONTEXT.md` | This file |

**To read the .key:** export it to pptx with AppleScript: `tell application "Keynote" to export (open f) to o as Microsoft PowerPoint`. For a faithful look, export `as slide images`.

**Deck structure (11 slides):**

| # | Slide |
|---|---|
| 1 | Cover |
| 2 | 배경: sleep 7h41m (OECD lowest); adults sit 9 h/day (질병관리청 2023). Closing band: "앉아 있는 그 시간을 함께 책임져 줄 존재가 필요하다" |
| 3 | Limits of existing products |
| 4 | Goals and features: subline "오래 앉아 있는 만큼, 그 시간을 책임져 주는 책상 위 다마고치"; feature 1 "조도·소리·온도·압력 센서와 PC로 수면 환경과 코딩 습관을 직접 측정"; target users "밤샘과 장시간 착석이 잦은 대학생 · 개발자" (kept on purpose — it matches the concept) |
| 5 | System diagram: inputs CdS · RTC · pressure+piezo (확장) · sound+LM35 · blue LED+foil (확장) · pulse (확장)+buttons |
| 6 | Habit table (§3) |
| 7 | Stats, evolution, sickness/death (§4) |
| 8 | Software and PC link |
| 9 | Uses and future work: AI food recognition, generative AI chat, weather/fine dust, medication reminder, Bluetooth app |
| 10–11 | Parts (§5.2) |

## 12. Open items [미정]
- Bedtime and wake defaults, decay rates, EXP amounts, evolution thresholds.
- Thresholds: the cold (18℃?), noise count, dark coding, FSR on/off, piezo vibration, blue-LED glow.
- The commit source (local `git log` of chosen repos, or the GitHub API) and the test command per repo.
- Korean text on the OLED (U8g2) vs icons.

## 13. Implementation (started 2026-11-04) [결정됨]
Code lives in this folder, tracked with a local git repo (no remote yet; the user will push to GitHub themselves). **Read `docs/ARCHITECTURE.md` before changing code.**

| Path | What |
|---|---|
| `firmware/habitchi/` | Arduino sketch (open `habitchi.ino`). Packages: sensors (`Sensor*.cpp`, `SensorsConfig.h`), clock/buttons (`GameClock.cpp`, `Input.cpp`, `ClockConfig.h`), pet brain (`Pet*.cpp`, `Storage.cpp`, `PetConfig.h`), display/sound (`Display*.cpp`, `Sprites.h`, `DisplayTexts.h`, `DisplayGlyphs.h`, `Sound.cpp`), PC link (`PcLink.cpp`). Shared contracts: `Pins.h`, `Timing.h`, `Facts.h`, `Events.h`, `PetView.h` + public headers |
| `firmware/tests/*/` | One standalone test sketch per part (light, noise, temp, blueled, phone, water, pulse, clock_buttons, oled_sound) |
| `pc/` | `habitchi_pc.py` bridge (serial, typing via pynput, commit queue, demo commands), `classify.py`, `hooks/post-commit` + `install_hook.py`, `dashboard.py`, `config.json` |
| `docs/` | `ARCHITECTURE.md`, `protocol.md`, `wiring.md` (full wiring + assembly order), `sensors.md`, `clock_input.md`, `display_sound.md`, `pet.md`, `pc.md` (all Korean) |

**Rules:**
- **No code comments**; explanations live in `docs/`.
- No `delay()`; only `PcLink` writes to Serial.
- Pins only in `Pins.h`; numbers live in each package's `*Config.h`.
- Only the pet package pops events and makes judgments.
- Timer2 is reserved for `tone()`.

**Toolchain:**
- Compile with the Arduino IDE's bundled CLI: `"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" compile --fqbn arduino:avr:mega firmware/habitchi`. Last build: flash ≈22 %, RAM ≈34 %.
- Arduino libraries: U8g2, RTClib (+ Adafruit BusIO), CapacitiveSensor.
- Python 3.9 user packages: pyserial, pynput, matplotlib.

**Decisions made during implementation:**
- **Buttons:**

  | Press | Action |
  |---|---|
  | A long | demo mode on/off |
  | A short | pet ↔ stats page |
  | B / C short (demo) | +60 / +10 game minutes |
  | B long | exercise start/stop |
  | C long | phone mode on/off |

- **Demo timings:** stretch after 30 s; a break = 10 s without typing; phone grace 5 s. Game time runs ×60.
- **Demo shortcuts** (bridge terminal, demo mode only): `stage <egg|baby|child|healthy|weak|obese>`, `sick <cold|sleep|hunger|overwork>`, `die`, `evolve` (runs the Child→adult judgement now).
- **Bedtime, sleep and the alarm:** 23:00 / 07:00. The wake alarm repeats until the room becomes bright (max 2 min). A night with no sleep scores 0.
- **Cold:** an ice pack while sleeping gives an immediate "감기 기운" reaction (sneeze) and a morning notice. Real cold sickness needs 2 consecutive cold nights.
- **Late-night commit:** an immediate bloated-belly look (Indigestion). The real Obese stage only comes at evolution.
- **EXP penalties stack:** each one halves (a midnight commit = ¼).
- **Exercise:** the result shows the heart-rate rise ("심박 +N"), switchable with `PetCfg::EXERCISE_SHOW_RISE` after accuracy tests. A failed measurement shows `Notice::MeasureAgain` ("다시 재 줘").
- **Phone mode** is saved in EEPROM.
- **Stage timers** count time spent in the current stage.
- **NoisyNight** fires on 1 loud noise.
- **Junk commit:** fewer than 6 characters, or a lazy word from `pc/config.json`.
- **`EVT,TIME`** carries Korean local time as unix seconds.
- **OLED text is Korean,** but the AVR U8g2 Korean font has only 478 syllables. Missing ones are hand-added in `DisplayGlyphs.h`; check this before changing texts. The DEMO badge stays English.
- **Sprites are placeholders;** the team will draw its own (see `docs/display_sound.md`).

**Known hardware limits:**
- A nearly empty bottle reads like "no bottle", so demo with a half-full one.
- Every sensor threshold is a guess until calibrated with the test sketches.
- pynput may misbehave with the Korean IME on recent macOS; test it first.
- The git hook blocks `git commit` while tests run (30 s limit).
- `PetCfg::DEATH_ENABLED` can be turned off for long rehearsals.
