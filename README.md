# D07

## Cấu hình clock
- Nguồn HSE được bật và đưa vào PLL.
- PLL nhân 9; SYSCLK và HCLK là **72 MHz**.
- AHB chia 1; APB1 chia 2 (**36 MHz**); APB2 chia 1 (**72 MHz**).
- Flash latency là 2 wait states.# D07 – Hệ thống cảnh báo khoảng cách/đỗ xe

Firmware cho STM32F103C8T6, hướng tới đo khoảng cách bằng cảm biến siêu âm HC-SR04, làm mượt số đo, hiển thị trên LED 7 đoạn 4 số, báo mức bằng LED và cảnh báo bằng còi.


## 1. Phần cứng và công cụ

- MCU: **STM32F103C8T6**, package LQFP48; định nghĩa build `STM32F103xB`.
- Clock: HSE → PLL ×9, SYSCLK/HCLK = **72 MHz**; APB1 = 36 MHz, clock timer APB1 được nhân đôi thành 72 MHz; APB2 = 72 MHz.
- HAL: STM32CubeF1, cấu hình dự án ghi CubeMX 6.18.1 và STM32Cube FW_F1 V1.8.7.
- Project file: `D07.ioc`; Keil uVision: `MDK-ARM/D07.uvprojx`.

## 2. Sơ đồ khối và luồng dữ liệu

```text
                       ┌──────────────────────────────┐
                       │         Parking App          │
                       │   Parking_Init / Parking_Task │
                       └──────────────┬───────────────┘
                                      │ gọi theo chu kỳ
                                      ▼
┌─────────────┐   Echo capture   ┌─────────────┐  5 mẫu/validity  ┌──────────────┐
│ HC-SR04     │ ───────────────► │ HCSR04      │ ───────────────► │ Distance     │
│ Trig / Echo │                  │ echo_us,    │                  │ Filter       │
└─────────────┘                  │ status      │                  └──────┬───────┘
                                 └─────────────┘                         │ cm × 10
                                                                        ▼
                           ┌────────────────────────────────────────────┐
                           │               Đầu ra ứng dụng              │
                           ├───────────────────┬───────────────┬────────┤
                           │ Display4           │ LevelLED      │ Buzzer │
                           │ LED 7 đoạn 4 số   │ 5 LED mức     │ Còi    │
                           └───────────────────┴───────────────┴────────┘
```

Luồng dự kiến theo giao diện hiện có:

1. `HCSR04_Task()` tạo/điều phối phép đo; bộ capture TIM3 ghi độ rộng Echo theo microsecond.
2. Ứng dụng lấy `HCSR04_Result` bằng `HCSR04_GetResult()`, kiểm tra `status`, rồi đổi `echo_us` thành khoảng cách thô theo cm.
3. `DF_Push()` nhận mẫu cm, tích lũy đủ 5 mẫu hợp lệ và trả khoảng cách lọc cùng cờ outlier.
4. Ứng dụng chuyển khoảng cách thành đơn vị `cm_x10` (centimet × 10) cho `Display4`, `LevelLED` và `Buzzer`.
5. TIM4 ngắt mỗi 500 µs để gọi `Display4_ScanISR()` quét multiplex 4 số. Các task đầu ra còn lại được gọi từ vòng lặp ứng dụng.



## 3. API giữa các module

| Header | API | Vai trò / dữ liệu |
|---|---|---|
| `parking_app.h` | `HAL_StatusTypeDef Parking_Init(void)` | Khởi tạo ứng dụng; trả trạng thái HAL. |
|  | `void Parking_Task(void)` | Một bước xử lý ứng dụng; dự kiến gọi liên tục trong `while(1)`. |
| `hcsr04.h` | `HAL_StatusTypeDef HCSR04_Init(void)` | Khởi tạo đo siêu âm. |
|  | `void HCSR04_Task(void)` | Điều phối một bước tác vụ đo, không có tham số. |
|  | `uint8_t HCSR04_GetResult(HCSR04_Result *result)` | Lấy kết quả qua con trỏ. `HCSR04_Result` gồm `echo_us` và `status`; API trả 0/1 theo quy ước do implementation cần xác định. |
|  | `void HCSR04_CaptureCallback(TIM_HandleTypeDef *htim)` | Callback để module xử lý capture timer. |
|  | `HCSR04_Status` | `HCSR04_OK`, `HCSR04_TIMEOUT`, `HCSR04_ECHO_HIGH`, `HCSR04_BAD_PULSE`, `HCSR04_HW_ERROR`. |
| `distance_filter.h` | `void DF_Reset(void)` | Đặt lại trạng thái bộ lọc. |
|  | `uint8_t DF_Push(float raw_cm, float *filtered_cm, uint8_t *outlier)` | Đưa mẫu khoảng cách cm vào bộ lọc. Header ghi chú kết quả cần đủ 5 mẫu; trả 1 khi có kết quả, 0 khi đang khởi động hoặc dữ liệu lỗi. Quy tắc outlier cụ thể không được khai báo. |
| `display4.h` | `void Display4_Init(void)` | Khởi tạo hiển thị. |
|  | `void Display4_ShowTenths(uint16_t cm_x10)` | Hiển thị giá trị theo đơn vị cm × 10. Ví dụ 123 có nghĩa 12,3 cm nếu module đặt dấu thập phân theo quy ước đó. |
|  | `void Display4_ShowInvalid(void)` | Hiển thị trạng thái không hợp lệ. |
|  | `void Display4_ScanISR(void)` | Quét một digit; header yêu cầu gọi từ ngắt TIM4 mỗi 500 µs (2 kHz). |
| `level_led.h` | `void LevelLED_Init(void)` | Khởi tạo dãy LED. |
|  | `void LevelLED_Task(uint8_t valid, uint16_t cm_x10)` | Cập nhật 5 LED báo mức theo tính hợp lệ và khoảng cách. Ngưỡng mức không nằm trong header. |
|  | `uint8_t LevelLED_Level(void)` | Đọc mức LED hiện tại. |
| `buzzer.h` | `HAL_StatusTypeDef Buzzer_Init(void)` | Khởi tạo còi; `buzzer_on` là trạng thái bật/tắt toàn cục kiểu `volatile uint8_t`. |
|  | `void Buzzer_Task(uint8_t valid, uint16_t distance_x10)` | Cập nhật cảnh báo còi theo tính hợp lệ và khoảng cách × 10. |

Header ứng dụng hiện không định nghĩa cấu trúc dữ liệu chia sẻ; luồng kết nối giữa module dự kiến được điều phối bởi `Parking_Task()`.

## 4. Bảng chân MCU và timer


| Chân | Cấu hình hiện tại | Dùng để đấu nối / ghi chú |
|---|---|---|
| PA1 | TIM2_CH2, PWM output | Đã cấu hình PWM; header chưa xác nhận ngoại vi nào dùng chân này. |
| PA2, PA3, PA4, PA5 | GPIO output push-pull, khởi động mức thấp | Bốn ngõ ra số. Tên chức năng cụ thể chưa xác định trong source hiện có. |
| PA6 | TIM3_CH1 input capture; timer 1 MHz | Ứng viên chắc chắn cho tín hiệu Echo vì đây là input capture duy nhất và API HC-SR04 có capture callback. Kết nối Echo qua mạch hạ mức/đảm bảo mức logic phù hợp 3,3 V. |
| PA8 | TIM1_CH1, PWM output, cấu hình active-low | Đã cấu hình PWM; header chưa xác nhận ngoại vi nào dùng chân này. |
| PB0, PB1, PB5–PB10, PB12–PB15 | GPIO output push-pull, khởi động mức cao | 12 ngõ ra số; chức năng cụ thể (segment, chọn digit, LED hoặc còi) chưa thể xác nhận từ snapshot. |
| PB11 | GPIO output push-pull, khởi động mức thấp | Ngõ ra số; chức năng chưa xác định. |
| PA13, PA14 | SWDIO, SWCLK | Dùng nạp/debug SWD; giữ lại khi kết nối ST-Link. JTAG bị tắt, SWD được giữ. |
| PD0, PD1 | HSE OSC_IN/OSC_OUT | Dao động ngoài theo cấu hình clock. |
| GND, 3V3 | Nguồn MCU | Nối chung mass giữa MCU, cảm biến và mạch ngoại vi. |



### Cấu hình timer

| Timer | Cấu hình | Chân/ứng dụng đã xác minh |
|---|---|---|
| TIM1 | PSC=71, ARR=499; PWM CH1, polarity low | PA8; chu kỳ PWM 500 µs (2 kHz) theo clock timer 72 MHz. |
| TIM2 | PSC=71, ARR=12; PWM CH2, pulse=1 | PA1; chu kỳ PWM 13 µs, khoảng 76,9 kHz. |
| TIM3 | PSC=71, ARR=65535; input capture CH1, rising edge ban đầu | PA6; độ phân giải đếm 1 µs, interrupt priority 1. |
| TIM4 | PSC=71, ARR=499; base interrupt | Interrupt mỗi 500 µs (2 kHz), priority 2; thích hợp quét LED 7 đoạn. |


## 5. Đấu nối phần cứng

### Nguồn và debug

1. Cấp nguồn đúng theo board STM32F103C8T6; nối GND chung với tất cả module.
2. Với ST-Link: SWDIO ↔ PA13, SWCLK ↔ PA14, GND ↔ GND; cấp nguồn mục tiêu phù hợp board.
3. Đảm bảo mạch HSE 8 MHz tương thích cấu hình ngoài ở PD0/PD1 (nhiều board Blue Pill đã có sẵn thạch anh).

### HC-SR04

| HC-SR04 | MCU | Hướng tín hiệu |
|---|---|---|
| VCC | Nguồn 5 V theo module | Nguồn cảm biến; không lấy 5 V từ chân GPIO. |
| GND | GND chung | Mass chung với MCU. |
| TRIG | Một GPIO output còn trống đã được chọn trong firmware | **Chưa xác định trong snapshot**; cần chọn chân và đồng bộ lại `gpio.c`/driver trước khi lắp. |
| ECHO | PA6/TIM3_CH1 qua chia áp/level shifter | Tín hiệu vào capture. ECHO của HC-SR04 thường lên gần 5 V; không đưa trực tiếp mức 5 V vào MCU 3,3 V. |

Ví dụ chia áp tham khảo cho ECHO: điện trở 1 kΩ nối từ ECHO tới PA6 và 2 kΩ từ PA6 xuống GND (mức 5 V thành khoảng 3,3 V). Kiểm tra điện áp thực tế và chân MCU/board trước khi cấp nguồn.

### LED 7 đoạn, LED mức và còi

- Xác định loại LED 7 đoạn **common anode** hay **common cathode**, chân segment a–g/dp, chân chọn digit và điện trở hạn dòng; loại hiển thị phải khớp với logic driver.
- Dùng điện trở hạn dòng cho từng segment/LED; với tải vượt dòng chân MCU, dùng transistor/driver phù hợp.
- Còi chủ động có thể điều khiển qua tầng transistor; còi thụ động cần PWM. Không đấu trực tiếp tải dòng lớn vào chân MCU.
- Đối chiếu chân điều khiển thực tế với sơ đồ mạch hoặc implementation module khi có đầy đủ source. Không dựa vào trạng thái mức khởi tạo GPIO để suy ra chân segment/digit.

## 6. Cấu hình ứng dụng

`Core/Inc/parking_config.h` chứa các tham số:

```c
#define PARKING_TEMP_C 20.0f
#define PARKING_CAL_A  1.0f
#define PARKING_CAL_B_CM 0.0f
```

- `PARKING_TEMP_C`: nhiệt độ nhập thủ công, **không đọc từ cảm biến nhiệt**. Có thể được dùng để bù tốc độ âm thanh nếu implementation hỗ trợ.
- Hiệu chuẩn khoảng cách theo công thức ghi trong header: `d_dung = A * d_loc + B`; mặc định không hiệu chỉnh.
- Đơn vị `PARKING_CAL_B_CM` là cm. Cần xác nhận cách áp dụng hiệu chuẩn với `parking_app.c` khi file này có mặt.



## 7. Cấu trúc thư mục

```text
Core/
  Inc/       Header HAL, cấu hình và API các module ứng dụng
  Src/       main.c, GPIO, timer, ngắt và MSP do CubeMX sinh
Drivers/     STM32 HAL và CMSIS
MDK-ARM/     Project Keil uVision và sản phẩm build cũ
D07.ioc      Cấu hình STM32CubeMX
.mxproject   Metadata tích hợp CubeMX
```


## 8. Build và nạp

1. Mở `D07.ioc` bằng STM32CubeMX để xem/chỉnh cấu hình ngoại vi; tránh regenerate nếu chưa sao lưu các vùng code người dùng.
2. Mở `MDK-ARM/D07.uvprojx` bằng Keil uVision, chọn target `STM32F103C8` và cấu hình ST-Link phù hợp.
3. Build, nạp qua SWD, kiểm tra clock và các tín hiệu bằng debugger/oscilloscope.
4. Để build ứng dụng parking hoàn chỉnh, cần khôi phục/thêm các implementation module và đưa chúng vào project Keil: `hcsr04.c`, `distance_filter.c`, `display4.c`, `level_led.c`, `buzzer.c`, `parking_app.c`; cập nhật `main.c` để gọi app và các timer/interrupt đúng cách.



## Sơ đồ cấu hình ngoại vi

```text
HSE ──► PLL ×9 ──► SYSCLK/HCLK 72 MHz
                       │
                       ├── TIM1_CH1 PWM ──► PA8
                       ├── TIM2_CH2 PWM ──► PA1
                       ├── TIM3_CH1 Input Capture ◄── PA6
                       ├── TIM4 Base Timer + Interrupt
                       └── GPIO output ──► PA2–PA5, PB0–PB1, PB5–PB15

PA13 = SWDIO       PA14 = SWCLK
PD0 = OSC_IN        PD1 = OSC_OUT
```


## Bảng chân GPIO và timer

### GPIO

| Chân | Cấu hình trong source | Mức khởi tạo |
|---|---|---|
| PA1 | TIM2_CH2, alternate-function push-pull | PWM |
| PA2–PA5 | GPIO output push-pull | Thấp |
| PA6 | TIM3_CH1 input capture, GPIO input không pull | Input |
| PA8 | TIM1_CH1, alternate-function push-pull | PWM |
| PB0, PB1, PB5–PB10, PB12–PB15 | GPIO output push-pull | Cao |
| PB11 | GPIO output push-pull | Thấp |
| PA13 | SWDIO | Serial Wire debug |
| PA14 | SWCLK | Serial Wire debug |
| PD0 | HSE OSC_IN | Dao động ngoài |
| PD1 | HSE OSC_OUT | Dao động ngoài |

PA2–PA5 và các chân PB nêu trên được khởi tạo trong `MX_GPIO_Init()` của `Core/Src/gpio.c`. Các chân PA1, PA6 và PA8 được cấu hình trong `Core/Src/tim.c`.

### Ánh xạ GPIO theo module

| Chân MCU | Tín hiệu/module | Logic trong driver |
|---|---|---|
| PA1 / TIM2_CH2 | HC-SR04 TRIG | Driver `hcsr04.c` phát trigger bằng PWM one-pulse. |
| PA6 / TIM3_CH1 | HC-SR04 ECHO | Input Capture; driver bắt cạnh lên rồi cạnh xuống để đo độ rộng xung. |
| PA8 / TIM1_CH1 | Buzzer | PWM; `buzzer.c` đặt CCR=250 khi bật, CCR=0 khi tắt. |
| PA2 | LED mức 1 | HIGH bật |
| PA3 | LED mức 2 | HIGH bật |
| PA4 | LED mức 3 | HIGH bật |
| PA5 | LED mức 4 | HIGH bật |
| PB11 | LED mức 5 | HIGH bật |
| PB0 | Segment a | LOW bật |
| PB1 | Segment b | LOW bật |
| PB5 | Segment c | LOW bật |
| PB6 | Segment d | LOW bật |
| PB7 | Segment e | LOW bật |
| PB8 | Segment f | LOW bật |
| PB9 | Segment g | LOW bật |
| PB10 | Segment dp | LOW bật |
| PB12 | Chọn digit index 0 | LOW bật digit |
| PB13 | Chọn digit index 1 | LOW bật digit |
| PB14 | Chọn digit index 2 | LOW bật digit |
| PB15 | Chọn digit index 3 | LOW bật digit |



#### Nối tín hiệu theo chân MCU

```text
HC-SR04 TRIG  ── PA1  (TIM2_CH2)
HC-SR04 ECHO  ── PA6  (TIM3_CH1)

LED mức 1      ── PA2
LED mức 2      ── PA3
LED mức 3      ── PA4
LED mức 4      ── PA5
LED mức 5      ── PB11

7-segment a    ── PB0       7-segment e  ── PB7
7-segment b    ── PB1       7-segment f  ── PB8
7-segment c    ── PB5       7-segment g  ── PB9
7-segment d    ── PB6       7-segment dp ── PB10
Digit index 0  ── PB12      Digit index 2 ── PB14
Digit index 1  ── PB13      Digit index 3 ── PB15

Buzzer PWM     ── PA8  (TIM1_CH1)
```

### Timer

| Timer | Prescaler | Period | Cấu hình kênh / ngắt |
|---|---:|---:|---|
| TIM1 | 71 | 499 | PWM CH1, pulse 0, polarity low; PA8 |
| TIM2 | 71 | 12 | PWM2 CH2, pulse 1; PA1 |
| TIM3 | 71 | 65535 | Input Capture CH1, rising edge, direct input; PA6; interrupt priority 1 |
| TIM4 | 71 | 499 | Base timer; interrupt priority 2 |

Với clock timer 72 MHz và prescaler 71, bộ đếm timer chạy ở 1 MHz. Theo các giá trị `Period` trong source, TIM1 và TIM4 có chu kỳ 500 µs; TIM2 có chu kỳ 13 µs; TIM3 tăng mỗi 1 µs và tràn sau 65.536 ms.

## API được khai báo

| Header | Khai báo | Thông tin có trong header |
|---|---|---|
| `parking_app.h` | `HAL_StatusTypeDef Parking_Init(void)` | Hàm khởi tạo ứng dụng. |
|  | `void Parking_Task(void)` | Hàm tác vụ ứng dụng. |
| `hcsr04.h` | `HAL_StatusTypeDef HCSR04_Init(void)` | Hàm khởi tạo module. |
|  | `void HCSR04_Task(void)` | Hàm tác vụ module. |
|  | `uint8_t HCSR04_GetResult(HCSR04_Result *result)` | Hàm lấy `HCSR04_Result`. |
|  | `void HCSR04_CaptureCallback(TIM_HandleTypeDef *htim)` | Callback nhận timer handle. |
|  | `HCSR04_Status`, `HCSR04_Result` | Enum trạng thái gồm `HCSR04_OK`, `HCSR04_TIMEOUT`, `HCSR04_ECHO_HIGH`, `HCSR04_BAD_PULSE`, `HCSR04_HW_ERROR`; struct có `echo_us` và `status`. |
| `distance_filter.h` | `void DF_Reset(void)` | Hàm reset bộ lọc. |
|  | `uint8_t DF_Push(float raw_cm, float *filtered_cm, uint8_t *outlier)` | Header ghi chú: trả 1 khi đủ 5 mẫu và có kết quả; trả 0 khi đang khởi động hoặc dữ liệu lỗi. |
| `display4.h` | `void Display4_Init(void)` | Hàm khởi tạo module hiển thị. |
|  | `void Display4_ShowTenths(uint16_t cm_x10)` | Hàm nhận giá trị `cm_x10`. |
|  | `void Display4_ShowInvalid(void)` | Hàm hiển thị trạng thái invalid. |
|  | `void Display4_ScanISR(void)` | Header ghi chú hàm được gọi từ ngắt TIM4 mỗi 500 µs (2 kHz). |
| `level_led.h` | `void LevelLED_Init(void)` | Hàm khởi tạo module. |
|  | `void LevelLED_Task(uint8_t valid, uint16_t cm_x10)` | Hàm tác vụ nhận `valid` và `cm_x10`. |
|  | `uint8_t LevelLED_Level(void)` | Hàm đọc mức LED. |
| `buzzer.h` | `HAL_StatusTypeDef Buzzer_Init(void)` | Hàm khởi tạo module. |
|  | `void Buzzer_Task(uint8_t valid, uint16_t distance_x10)` | Hàm tác vụ nhận `valid` và `distance_x10`. |
|  | `extern volatile uint8_t buzzer_on` | Biến trạng thái được khai báo bên ngoài. |

## Tham số cấu hình

`Core/Inc/parking_config.h` khai báo:

```c
#define PARKING_TEMP_C 20.0f
#define PARKING_CAL_A 1.0f
#define PARKING_CAL_B_CM 0.0f
```

Header ghi chú `PARKING_TEMP_C` là nhiệt độ nhập thủ công, không phải số đo cảm biến nhiệt. Công thức hiệu chuẩn được ghi trong header là `d_dung = A * d_loc + B`.

## Cấu trúc mã nguồn

- `Core/Inc/`: header HAL, cấu hình và khai báo API.
- `Core/Src/`: `main.c`, `gpio.c`, `tim.c`, `stm32f1xx_it.c`, `stm32f1xx_hal_msp.c`, `system_stm32f1xx.c`.
- `Drivers/`: STM32 HAL và CMSIS.
- `MDK-ARM/`: project Keil và các tệp build có sẵn.
- `D07.ioc`: cấu hình STM32CubeMX.



