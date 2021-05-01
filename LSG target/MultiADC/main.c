/*
 * MultiADC.c
 *
 * Created: 2019-08-29 오후 2:23:48
 * Author : zbumj
 */ 
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>

void InitializeTimer1(void)
{
	// 모드 14, 고속 PWM 모드
	TCCR3A |= (1 << WGM31);
	TCCR3B |= (1 << WGM32) | (1 << WGM33);
	
	// 비반전 모드
	// TOP : ICR1, 비교일치값 : OCR1A 레지스터
	TCCR3A |= (1 << COM3A1) | (1 << COM3B1);
	TCCR3B |= (1 << CS31);		// 분주율 8, 2MHz
	ICR3 = 39999;				// 20ms 주기
}

int read, i, j, k, cnt;
int adc_select = 0;

int main(void)
{
	UBRR0H = 0x00; // 9,600 BAUD rate
	UBRR0L = 207;
	UCSR0A |= 0x02; // 2배속 모드
	// 비동기, 8비트 데이터, 패리티 없음, 1비트 정지 비트 모드
	UCSR0C |= 0x06;
	UCSR0B |= 0x18;
	
	ADCSRA |= 0xA7;			// 분주비 설정, ADC 활성화, 프리 러닝 모드
	ADMUX |= 0x40;
	ADCSRA |= 0x40;			// 변환 시작
	
	// 모터 제어 핀을 출력으로 설정
	DDRE |= (1 << PE3) | (1 << PE4);
	InitializeTimer1();			// 1번 타이머/카운터 설정
	OCR3A = 2800;
	OCR3B = 2800;
	
    while (1)
	{
		ADMUX = 0x40 + adc_select; // use AVCC as a standard voltage
		// 0x40 = ADC0, 0x41 = ADC1, 0x42 = ADC2, 0x43 = ADC3
		_delay_ms(10);
		
		while(!(ADCSRA & 0x10));	// 변환 종료 대기
		read = ADC + 1000; // 1000 ~ 2023
		_delay_ms(10);
		
		//while( !(UCSR0A & (1 << UDRE0)) );
		UDR0 = 33 + adc_select;
		_delay_ms(10);
		
		if(adc_select == 0 && read < 1450)
		{/*
			for(cnt = 0; cnt < 100; cnt++)
			{
				UDR0 = 119;
				_delay_ms(10);
			}*/
		}
		else if(adc_select == 1 && read < 1450)
		{/*
			for(cnt = 0; cnt < 100; cnt++)
			{
				UDR0 = 120;
				_delay_ms(10);
			}*/
		}
		else if(adc_select == 2 && read < 1450)
		{
			for(cnt = 0; cnt < 100; cnt++)
			{
				UDR0 = 121;
				_delay_ms(10);
			}
			OCR3A = 4000;
			_delay_ms(1000);
			OCR3A = 2700;
		}
		else if(adc_select == 3 && read < 1450)
		{
			for(cnt = 0; cnt < 100; cnt++)
			{
				UDR0 = 122;
				_delay_ms(10);
			}
			OCR3B = 4000;
			_delay_ms(1000);
			OCR3B = 2700;
		}
		else
		{
			//while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
			for(i = 0; read - i*1000 >= 1000; i++);
			UDR0 = i + 48; //1000s, thousands
			_delay_ms(10);
			
			for(j = 0; read - i*1000 - j*100 >= 100; j++);
			UDR0 = j + 48; //100s, hundreds
			_delay_ms(10);
			
			for(k = 0; read - i*1000 - j*100 - k*10 >= 10; k++);
			UDR0 = k + 48; //10s, tens
			_delay_ms(10);
			
			UDR0 = read - i*1000 - j*100 - k*10 + 48; //1s, units
			_delay_ms(10);
		}
		if (adc_select == 3) adc_select = 0;
		else adc_select++;
		/*
		//while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
		for(i = 0; read - i*1000 >= 1000; i++);
		UDR0 = i + 48; //1000s, thousands
		_delay_ms(10);
		
		for(j = 0; read - i*1000 - j*100 >= 100; j++);
		UDR0 = j + 48; //100s, hundreds
		_delay_ms(10);
		
		for(k = 0; read - i*1000 - j*100 - k*10 >= 10; k++);
		UDR0 = k + 48; //10s, tens
		_delay_ms(10);
		
		UDR0 = read - i*1000 - j*100 - k*10 + 48; //1s, units
		_delay_ms(10);
		*/
	}
	return 0;
}

