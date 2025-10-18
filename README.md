# LAB 2: Timer Interrupt & LED Scanning — STM32F103C6

**Mô tả ngắn:**  
Project này là bài tập về Timer Interrupt và LED Scanning cho vi xử lý **STM32F103C6**, được phát triển bằng **STM32CubeIDE** và mô phỏng trên **Proteus**.

Các nhánh trong repo này tương ứng với từng exercise trong bài LAB.

---

## Cấu trúc thư mục
**Nhánh main** 
- **`Code/`**: Thư mục project STM32CubeIDE cho bài tập hiện tại.
  - `LAB2.ioc`: File cấu hình STM32CubeMX.
  - `Core/`: Chứa mã nguồn chính (`main.c`, `stm32f1xx_it.c`).
  - `Debug/`: Chứa file `.hex`/`.elf` sau khi build.
- **`README.md`**: File hướng dẫn này.

**Nhánh ex...** (ví dụ: nhánh `ex1`)
- **`Code/`**: Thư mục chứa toàn bộ mã nguồn và file mô phỏng cho bài tập.
  - **`[Tên project]/`**: Thư mục project STM32CubeIDE cho bài tập tương ứng.
    - `[Tên project].ioc`: File cấu hình CubeMX.
    - `Core/`: Chứa mã nguồn chính.
    - `Debug/`: Chứa file `.hex` sau khi build.
  - **`[Tên file Proteus].pdsprj`**: File mô phỏng Proteus chứa sơ đồ nguyên lý.

---

## Mô tả các bài tập (dựa trên project cơ sở)

- **`main` (Project `LAB2` hiện tại)**  
  Cài đặt clock, GPIO, Timer2 và khởi tạo interrupt. Đây là điểm khởi đầu.

- **`ex1`** — Quét 2 LED 7 đoạn:  
  Hiển thị số `12` trên 2 LED 7 đoạn, chuyển đổi hiển thị giữa hai LED sau mỗi 0.5s.

- **`ex2`** — Quét 4 LED 7 đoạn và 2 LED đơn:  
  Mở rộng lên 4 LED 7 đoạn, 2 LED đơn ở giữa nhấp nháy mỗi giây.

- **`ex3`** — Tối ưu bằng hàm `update7SEG(index)`:  
  Tách logic hiển thị ra hàm riêng và dùng mảng `led_buffer[]` để quản lý dữ liệu.

- **`ex4`** — Điều chỉnh tần số quét:  
  Thay đổi tần số quét để 4 LED 7 đoạn hiển thị ổn định (ví dụ: 1Hz).

- **`ex5`** — Xây dựng đồng hồ số:  
  Hiển thị giờ và phút, tự động cập nhật thời gian.

- **`ex6`** — Sử dụng Software timer:  
  Tạo các bộ đếm thời gian bằng phần mềm để thay thế cho `HAL_Delay`.

- **`ex7`** — Đồng hồ số với Software timer:  
  Áp dụng software timer để cập nhật đồng hồ một cách không-chặn (non-blocking).

- **`ex8`** — Tối ưu xử lý trong ISR:  
  Chuyển việc quét LED từ hàm ngắt ra vòng lặp `main()`, chỉ dùng ngắt để cập nhật software timer.

- **`ex9`** — Điều khiển LED Matrix 8×8:  
  Tích hợp và hiển thị một ký tự (ví dụ: chữ 'A') trên LED matrix.

- **`ex10`** — Tạo hiệu ứng trên LED Matrix:  
  Lập trình hiệu ứng dịch chuyển ký tự trên LED matrix.

*(Chi tiết yêu cầu cho từng bài tập có trong tài liệu LAB đi kèm).*