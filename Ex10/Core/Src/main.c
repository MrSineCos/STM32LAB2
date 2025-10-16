/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
static GPIO_TypeDef *const LED7_PORT[7] = {
Led7_a_GPIO_Port, Led7_b_GPIO_Port, Led7_c_GPIO_Port, Led7_d_GPIO_Port,
Led7_e_GPIO_Port, Led7_f_GPIO_Port, Led7_g_GPIO_Port };

static const uint16_t LED7_PINS[7] = {
Led7_a_Pin, Led7_b_Pin, Led7_c_Pin, Led7_d_Pin, Led7_e_Pin, Led7_f_Pin,
Led7_g_Pin };

static const uint8_t LED7_MAP[10] = { 0b00111111, // 0
		0b00000110, // 1
		0b01011011, // 2
		0b01001111, // 3
		0b01100110, // 4
		0b01101101, // 5
		0b01111101, // 6
		0b00000111, // 7
		0b01111111, // 8
		0b01101111,  // 9
		};

const int MAX_LED = 4;

typedef enum {
	NUM1, NUM2, NUM3, NUM4
} num;
typedef enum {
	DARK, BRIGHT
} dot;

int led_buffer[4] = { 1, 0, 2, 0 };

const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;
uint8_t matrix_buffer[8] = { 0x00, // cột 0 (trái ngoài)
		0x7C, // cột 1
		0x12, // cột 2
		0x11, // cột 3
		0x11, // cột 4
		0x12, // cột 5
		0x7C, // cột 6
		0x00  // cột 7 (phải ngoài)
		};

static GPIO_TypeDef *const MATRIX_PORT[8] = {
ROW0_GPIO_Port, ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port,
ROW4_GPIO_Port, ROW5_GPIO_Port, ROW6_GPIO_Port, ROW7_GPIO_Port };

static const uint16_t MATRIX_PIN[8] = {
ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin };

#define GLYPH_LEN 8

// Mảng mô tả ký tự 'A' (các cột từ trái sang phải).
// Mình để padding 0 ở đầu và cuối để ký tự chui vào/ra màn hình mượt.
const uint8_t glyph_A[GLYPH_LEN] = {
    0x00, // padding
    0x7C, // 01111100
    0x12, // 00010010
    0x11, // 00010001
    0x11, // 00010001
    0x12, // 00010010
    0x7C, // 01111100
    0x00, // padding
};

// Scroll control
int scroll_offset = 0;         // vị trí bắt đầu lấy cửa sổ từ glyph_A
int scroll_tick = 0;           // đếm số frame (full refresh) đã trôi qua
const int SCROLL_SPEED_FRAMES = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef*);
static void set_LED7_by_mask(uint8_t);
static void clear7SEG(void);
static void display7SEG(int);
static void initState(void);
static void update7SEG(int);
static void updateClockBuffer();
static void updateLEDMatrix(int);
static void displayLEDmatrix(uint8_t);
static void updateMatrixWindow(int);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int timer0_counter = 0;
int timer0_flag = 0;

int timer1_counter = 0;
int timer1_flag = 0;

int timer2_counter = 0;
int timer2_flag = 0;

int timer3_counter = 0;
int timer3_flag = 0;

int TIMER_CYCLE = 10;

int minute = 0;
int hour = 0;
int second = 0;
//Timer for counter seconds
void setTimer0(int duration) {
	timer0_counter = duration / TIMER_CYCLE;
	timer0_flag = 0;
}

//Timer for adjusting frequency of LED 7SEGs
void setTimer1(int duration) {
	timer1_counter = duration / TIMER_CYCLE;
	timer1_flag = 0;
}

//Timer for controlling LED dots
void setTimer2(int duration) {
	timer2_counter = duration / TIMER_CYCLE;
	timer2_flag = 0;
}

//Timer for controlling LED matrix
void setTimer3(int duration) {
	timer3_counter = duration / TIMER_CYCLE;
	timer3_flag = 0;
}

void timer_run() {
	if (timer0_counter > 0) {
		timer0_counter--;
		if (timer0_counter == 0)
			timer0_flag = 1;
	}
	if (timer1_counter > 0) {
		timer1_counter--;
		if (timer1_counter == 0)
			timer1_flag = 1;
	}
	if (timer2_counter > 0) {
		timer2_counter--;
		if (timer2_counter == 0)
			timer2_flag = 1;
	}
	if (timer3_counter > 0) {
		timer3_counter--;
		if (timer3_counter == 0)
			timer3_flag = 1;
	}
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	initState();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	setTimer0(1000);
	setTimer1(1000);
	setTimer2(1000);
	setTimer3(1000);
	updateMatrixWindow(scroll_offset);
	dot dot_state = BRIGHT;
	second = 50;
	minute = 59;
	hour = 10;

	int time0 = 1000;
	int time1 = 500;
	int time2_dark = 300;
	int time2_bright = 700;
	int time3 = 250;

	int index_led = 0;
	int index_col = 0;
	while (1) {
		/* USER CODE END WHILE */
		if (timer0_flag == 1) {
			setTimer0(time0);
			second++;
			if (second >= 60) {
				second = 0;
				minute++;
			}
			if (minute >= 60) {
				minute = 0;
				hour++;
			}
			if (hour >= 24) {
				hour = 0;
			}
			updateClockBuffer();
		}

		if (timer1_flag == 1) {
			setTimer1(time1);
			update7SEG(index_led);
			index_led++;
			if (index_led >= MAX_LED) {
				index_led = 0;
			}
		}

		if (timer2_flag == 1) {
			switch (dot_state) {
			case BRIGHT:
				setTimer2(time2_dark);
				HAL_GPIO_WritePin(DOT_GPIO_Port, DOT_Pin, GPIO_PIN_SET);
				dot_state = DARK;
				break;
			case DARK:
				setTimer2(time2_bright);
				HAL_GPIO_WritePin(DOT_GPIO_Port, DOT_Pin, GPIO_PIN_RESET);
				dot_state = BRIGHT;
				break;
			default:
				break;
			}

		}

		if (timer3_flag == 1)
		{
		    setTimer3(time3);
		    updateLEDMatrix(index_col);
		    index_col++;
		    if(index_col >= MAX_LED_MATRIX)
		    {
		        index_col = 0;

		        // Một full-frame (đã quét 8 cột) hoàn thành -> tăng tick
		        scroll_tick++;
		        if (scroll_tick >= SCROLL_SPEED_FRAMES) {
		            scroll_tick = 0;
		            // dịch ký tự sang trái 1 cột
		            scroll_offset++;
		            if (scroll_offset >= GLYPH_LEN) scroll_offset = 0; // quay vòng
		            updateMatrixWindow(scroll_offset); // cập nhật matrix_buffer mới
		        }
		    }
		}

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 7999;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 9;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
			ENM0_Pin | ENM1_Pin | DOT_Pin | LED_RED_Pin | EN0_Pin | EN1_Pin
					| EN2_Pin | EN3_Pin | ENM2_Pin | ENM3_Pin | ENM4_Pin
					| ENM5_Pin | ENM6_Pin | ENM7_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			Led7_a_Pin | Led7_b_Pin | Led7_c_Pin | ROW2_Pin | ROW3_Pin
					| ROW4_Pin | ROW5_Pin | ROW6_Pin | ROW7_Pin | Led7_d_Pin
					| Led7_e_Pin | Led7_f_Pin | Led7_g_Pin | ROW0_Pin | ROW1_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : ENM0_Pin ENM1_Pin DOT_Pin LED_RED_Pin
	 EN0_Pin EN1_Pin EN2_Pin EN3_Pin
	 ENM2_Pin ENM3_Pin ENM4_Pin ENM5_Pin
	 ENM6_Pin ENM7_Pin */
	GPIO_InitStruct.Pin = ENM0_Pin | ENM1_Pin | DOT_Pin | LED_RED_Pin | EN0_Pin
			| EN1_Pin | EN2_Pin | EN3_Pin | ENM2_Pin | ENM3_Pin | ENM4_Pin
			| ENM5_Pin | ENM6_Pin | ENM7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : Led7_a_Pin Led7_b_Pin Led7_c_Pin ROW2_Pin
	 ROW3_Pin ROW4_Pin ROW5_Pin ROW6_Pin
	 ROW7_Pin Led7_d_Pin Led7_e_Pin Led7_f_Pin
	 Led7_g_Pin ROW0_Pin ROW1_Pin */
	GPIO_InitStruct.Pin = Led7_a_Pin | Led7_b_Pin | Led7_c_Pin | ROW2_Pin
			| ROW3_Pin | ROW4_Pin | ROW5_Pin | ROW6_Pin | ROW7_Pin | Led7_d_Pin
			| Led7_e_Pin | Led7_f_Pin | Led7_g_Pin | ROW0_Pin | ROW1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	timer_run();
}

static void set_LED7_by_mask(uint8_t mask) {
	for (int s = 0; s < 7; s++) {

		GPIO_TypeDef *port = (GPIO_TypeDef*) LED7_PORT[s];
		uint16_t pin = LED7_PINS[s];

		if ((mask >> s) & 1) {
			HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // Turn on segment
		} else {
			HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); // Turn off segment
		}
	}
}

void clear7SEG() {
	display7SEG(10);
}

void display7SEG(int number) {
	if (number > 9) {
		set_LED7_by_mask(0x00); // Turn off all segments
	} else {
		uint8_t mask = LED7_MAP[number];
		set_LED7_by_mask(mask);
	}
}

void initState() {
	clear7SEG();
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DOT_GPIO_Port, DOT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
}

void update7SEG(int index) {
	switch (index) {
	case 0:
		display7SEG(led_buffer[0]);
		HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);
		break;
	case 1:
		display7SEG(led_buffer[1]);
		HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);
		break;
	case 2:
		display7SEG(led_buffer[2]);
		HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);
		break;
	case 3:
		display7SEG(led_buffer[3]);
		HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);
		break;
	default:
		break;
	}
}

void updateClockBuffer() {
	led_buffer[0] = hour / 10;
	led_buffer[1] = hour % 10;
	led_buffer[2] = minute / 10;
	led_buffer[3] = minute % 10;
}

void updateLEDMatrix(int index) {
	switch (index) {
	case 0:
		displayLEDmatrix(matrix_buffer[0]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 1:
		displayLEDmatrix(matrix_buffer[1]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 2:
		displayLEDmatrix(matrix_buffer[2]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 3:
		displayLEDmatrix(matrix_buffer[3]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 4:
		displayLEDmatrix(matrix_buffer[4]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 5:
		displayLEDmatrix(matrix_buffer[5]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 6:
		displayLEDmatrix(matrix_buffer[6]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_SET);
		break;
	case 7:
		displayLEDmatrix(matrix_buffer[7]);
		HAL_GPIO_WritePin(ENM0_GPIO_Port, ENM0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM1_GPIO_Port, ENM1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM2_GPIO_Port, ENM2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM3_GPIO_Port, ENM3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM4_GPIO_Port, ENM4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM5_GPIO_Port, ENM5_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM6_GPIO_Port, ENM6_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ENM7_GPIO_Port, ENM7_Pin, GPIO_PIN_RESET);
	default:
		break;
	}
}

void displayLEDmatrix(uint8_t mask) {
	for (int s = 0; s < MAX_LED_MATRIX; s++) {

		GPIO_TypeDef *port = (GPIO_TypeDef*) MATRIX_PORT[s];
		uint16_t pin = MATRIX_PIN[s];

		if ((mask >> s) & 1) {
			HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // Turn on segment
		} else {
			HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); // Turn off segment
		}
	}
}

void updateMatrixWindow(int offset)
{
    for (int c = 0; c < MAX_LED_MATRIX; c++) {
        int idx = offset + c;
        if (idx >= 0 && idx < GLYPH_LEN) {
            matrix_buffer[c] = glyph_A[idx];
        } else {
            matrix_buffer[c] = glyph_A[idx - MAX_LED_MATRIX]; // nếu vượt ngoài glyph, dùng cột trắng
        }
    }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
