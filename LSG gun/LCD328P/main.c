/*
 * LCD.c
 *
 * Created: 2019-08-29 오후 2:23:48
 * Author : zbumj
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>

#define PORT_DATA PORTD
#define PORT_CONTROL PORTC
#define DDR_DATA DDRD
#define DDR_CONTROL DDRC

#define RS_PIN 0
#define RW_PIN 1
#define E_PIN 2

#define COMMAND_CLEAR_DISPLAY 0x01
#define COMMAND_8_BIT_MODE 0x38
#define COMMAND_4_BIT_MODE 0x28

#define COMMAND_DISPLAY_ON_OFF_BIT 2
#define COMMAND_CURSOR_ON_OFF_BIT 1
#define COMMAND_BLINK_ON_OFF_BIT 0

uint8_t MODE = 4;

void LCD_pulse_enable(void) 		// 하강 에지에서 동작
{
	PORT_CONTROL |= (1 << E_PIN);	// E를 HIGH로
	_delay_us(1);
	PORT_CONTROL &= ~(1 << E_PIN);	// E를 LOW로
	_delay_ms(1);
}

void LCD_write_data(uint8_t data)
{
	PORT_CONTROL |= (1 << RS_PIN);	// 문자 출력에서 RS는 1
	
	if(MODE == 8){
		PORT_DATA = data;			// 출력할 문자 데이터
		LCD_pulse_enable();
	}
	else
	{
		PORT_DATA = data & 0xF0;		// 상위 4비트
		LCD_pulse_enable();
		PORT_DATA = (data << 4) & 0xF0;	// 하위 4비트
		LCD_pulse_enable();
	}
	_delay_ms(2);
}

void LCD_write_command(uint8_t command)
{
	PORT_CONTROL &= ~(1 << RS_PIN);	// 명령어 실행에서 RS는 0
	
	if(MODE == 8){
		PORT_DATA = command;		// 데이터 핀에 명령어 전달
		LCD_pulse_enable();			// 명령어 실행
	}
	else{
		PORT_DATA = command & 0xF0;		// 상위 4비트
		LCD_pulse_enable();
		PORT_DATA = (command << 4) & 0xF0;	// 하위 4비트
		LCD_pulse_enable();
	}
	_delay_ms(2);
}

void LCD_clear(void)
{
	LCD_write_command(COMMAND_CLEAR_DISPLAY);
	_delay_ms(2);
}

void LCD_init(void)
{
	_delay_ms(50);				// 초기 구동 시간
	
	// 연결 핀을 출력으로 설정
	if(MODE == 8) DDR_DATA = 0xFF;
	else DDR_DATA |= 0xF0;
	PORT_DATA = 0x00;
	
	DDR_CONTROL |= (1 << RS_PIN) | (1 << RW_PIN) | (1 << E_PIN);
	
	// RW 핀으로 LOW를 출력하여 쓰기 전용으로 사용
	PORT_CONTROL &= ~(1 << RW_PIN);
	
	if(MODE == 8)
	LCD_write_command(COMMAND_8_BIT_MODE);		// 8비트 모드
	else{
		LCD_write_command(0x02);				// 4비트 모드 추가 명령
		LCD_write_command(COMMAND_4_BIT_MODE);		// 4비트 모드
	}
	
	// display on/off control
	// 화면 on, 커서 off, 커서 깜빡임 off
	uint8_t command = 0x08 | (1 << COMMAND_DISPLAY_ON_OFF_BIT);
	LCD_write_command(command);

	LCD_clear();			// 화면 지움

	// Entry Mode Set
	// 출력 후 커서를 오른쪽으로 옮김, 즉, DDRAM의 주소가 증가하며 화면 이동은 없음
	LCD_write_command(0x06);
}

void LCD_write_string(char *string)
{
	uint8_t i;
	for(i = 0; string[i]; i++)			// 종료 문자를 만날 때까지
	LCD_write_data(string[i]);		// 문자 단위 출력
}

void LCD_goto_XY(uint8_t row, uint8_t col)
{
	col %= 16;		// [0 15]
	row %= 4;		// [0 1]

	// 첫째 라인 시작 주소는 0x00, 둘째 라인 시작 주소는 0x40
	uint8_t address = 0x00;
	if(row < 2) address = (0x40 * row) + col;
	else address = 0x10 + (0x40 * (row - 2)) + col;
	uint8_t command = 0x80 + address;
	
	LCD_write_command(command);	// 커서 이동
}

char data;
char adc_str[4][7] = {"1:abcd\0", "2:abcd\0", "3:abcd\0", "4:abcd\0"};
int adc_int[4];

void delay(void);
volatile int cnt = 0;
int shot_int = 20;
int score_int = 0, tag = 0, flag = 0, gag = 0;
int i, j, k;
char score[5] = "abcd\0";
char shot[3] = "ab\0";

int main(void)
{
    LCD_init();
	UBRR0H = 0x00;
	UBRR0L = 207; // 8MHz 9,600 BAUD rate
	UCSR0A |= 0x02; // 2배속 모드, 비동기, 8비트 데이터, 패리티 없음, 1비트 정지 비트 모드
	UCSR0C |= 0x06;
	UCSR0B |= 0x18;
	
	DDRB |= 0x01;
	PORTD |= 1<<PD2;   // enable PORTD.2 pin pull up resistor
	EIMSK |= 1<<INT0;  // enable external interrupt 0
	EICRA |= 1<<ISC01; // interrupt on falling edge
	sei();
	PORTB=0x00;
	
	//LCD_clear();
    while (1)
	{
		for(int i = 0; i < 3; i++)
		{
			while( !(UCSR0A & (1<<RXC0)) );
			data = UDR0;
		}
		while(1)
		{
			if(tag == 1)
			{
				while(1)
				{
					_delay_ms(500);
					while( !(UCSR0A & (1<<RXC0)) );
					data = UDR0;
					if(data < 119 || data > 122)
					{
						tag = 0;
						break;
					}
				}
			}
			
			while( !(UCSR0A & (1<<RXC0)) );	// 데이터 수신 대기
			data = UDR0;
			
			if(data >= 119 && data <= 122 && tag == 0)
			{
				LCD_clear();
				switch(data)
				{
					case 119: LCD_write_string("1: Hit!"); break;
					case 120: LCD_write_string("2: Hit!"); break;
					case 121: LCD_write_string("3: Hit!"); score_int += 100; tag = 1; break;
					case 122: LCD_write_string("4: Hit!"); score_int += 200; tag = 1; break;
				}
				_delay_ms(1000);
				LCD_clear();
			}
			if(data == 35 && flag == 0) {flag = 1; break;}
			else if(data == 36 && flag == 1) {flag = 0; break;}
		}
		int row = data - 33; // row = 0 ~ 3
		for(int col = 0; col <= 3; col++)
		{
			while( !(UCSR0A & (1<<RXC0)) );	// 데이터 수신 대기
			data = UDR0;
			adc_str[row][col + 2] = data; // Stored as string type
			adc_int[row] = adc_int[row] * ((col + 3) / 4) + pow(10, col ^ 3) * (data - 48); // Stored as int type
		}
		
		LCD_goto_XY((row / 2), (row % 2) * 8);
		LCD_write_string(adc_str[row]); // LCD 출력
		LCD_goto_XY(2, 0);
		LCD_write_string("Score: ");
		LCD_goto_XY(2, 8);
		
		for(i = 0; score_int - i*1000 >= 1000; i++);
		score[0] = i + 48; //1000s, thousands
		_delay_ms(10);
		
		for(j = 0; score_int - i*1000 - j*100 >= 100; j++);
		score[1] = j + 48; //100s, hundreds
		_delay_ms(10);
		
		for(k = 0; score_int - i*1000 - j*100 - k*10 >= 10; k++);
		score[2] = k + 48; //10s, tens
		_delay_ms(10);
		
		score[3] = score_int - i*1000 - j*100 - k*10 + 48; //1s, units
		_delay_ms(10);
		
		LCD_write_string(score);
		
		LCD_goto_XY(3, 0);
		LCD_write_string("Shots: ");
		LCD_goto_XY(3, 8);
		
		for(k = 0; shot_int - k*10 >= 10; k++);
		shot[0] = k + 48; //10s, tens
		_delay_ms(10);
		
		shot[1] = shot_int - k*10 + 48; //1s, units
		_delay_ms(10);
		
		LCD_write_string(shot);
		
		
		//LCD_clear();
		//_delay_ms(10);
		
		if(shot_int <= 0)
		{
			LCD_goto_XY(0, 0);
			LCD_write_string("Game Over!");
			LCD_goto_XY(1, 0);
			LCD_write_string("Retry in 5 sec");
			_delay_ms(5000);
			LCD_clear();
			score_int = 0;
			shot_int = 20;
		}
		
		_delay_ms(80);
	}
}

void delay(void)
{
	TCNT0 = 0x00;
	TCCR0B = 0x05;     // start timer 0,normal mode, prescaler
	while ((TIFR0 & (1<<TOV0)) == 0);
	TCCR0B = 0x00;     // stop timer 0
	TIFR0 |= 1 << TOV0;  // clear timer 0 overflow flag
}

ISR(INT0_vect)
{
	cnt = 0;
	if(gag == 0) {shot_int -= 1; gag = 1;}
	else gag = 0;
	PORTB |= 0x01;
	//while(cnt < 31500)
	//{
	//   cnt++;
	//}
	while(cnt < 10)	//300ms
	{
		delay();
		cnt++;
	}
	PORTB &= 0xFE;
}