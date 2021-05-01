# Laser_Shooting_Game
빛돌 2019년작 레이저 사격 게임


---
### Processing Unit
* 과녁 부분은 16MHz OSC 기반의 ATmega128A MCU를 사용 (LSG Target)
* 총 부분은 8MHz OSC 기반의 ATmega328P MCU를 사용 (LSG Gun)
* 전체적인 Firmware 구성은 모두 Atmel Studio 기반에서 작성

#### Bluetooth / UART
* 과녁 부분을 담당하는 Firmware
* 과녁 부분과 총 부분 사이에서 무선 근거리 통신이 가능한 Bluetooth를 이용
* 8-bit UART 기반의 Bluetooth 통신으로 LASER가 과녁에 명중했을 때 임의의 값을 총 부분으로 전송
* 총 부분에서는 수신한 값에 따라 명중된 과녁을 파악하여 점수 계산
* 과녁 부분의 OSC는 16MHz이고, 총 부분의 OSC는 8MHz로 서로 달라 두 MCU 간의 Bluetooth 통신을 위해 BAUD rate를 통일

#### ADC
* 10-bit ADC를 이용 (0 ~ 1023)
* LASER의 인식 확률을 높이기 위해 LASER 인식 전과 인식 후의 ADC 값의 차이를 최대화
* 계산을 통해 가변 저항값은 LASER 인식 전의 저항값과 인식 후의 저항값의 기하평균이 되어야 함을 구현

### PWM
* 과녁은 서보 모터가 회전하는 것으로 구현
* 서보 모터는 평상시에 -90도의 상태로 서 있다가 과녁에 명중하면 0도로 회전
* 0도로 회전하면 1초가 지난 후에 다시 -90도로 돌아오도록 설계
* 16-bit Timer / Counter의 PWM 출력 핀을 이용하여 비반전 모드로 구현
* 서보 모터로 사용한 HS-7985MG는 OC3A(PE3) 또는 OC3B(PE4)의 값이 2700일 때 -90도, 4500일 때 0도로 회전

## Interrupt
* 방아쇠를 구현하기 위해 1kΩ Pull-up 저항을 포함하여 외부 인터럽트 이용
* LASER가 발사되는 시간이 매우 길면 그 시간 안에 2개 이상의 과녁에 명중할 수 있기 때문에 발사되는 시간을 약 0.3초 이내로 제한방
* 방아쇠에 연결된 스위치를 LASER에 직접적으로 연결하지 않고 외부 Interrupt를 이용
* 방아쇠를 당기면 스위치가 닫히면서 Interrupt 핀에 Negative edge 신호를 입력하여 외부 Interrupt를 발생하고, Interrupt 발생 이후 종료되는 시간이 약 0.3초가 되도록 delay 함수를 구현
* ATmega328P의 PD2 입력 핀에 Interrupt가 발생하면 PB0 핀에서 High signal을 출력
* 이 신호는 KA78R33 Voltage Regulator의 Control 핀에 High signal이 인가되면 Enable이 되어서 정전압이 출력되고 LASER가 발사되며, 0.3초가 지나면 PB0 핀에서 Low signal이 출력되므로 Regulator은 Disable이 되어서 LASER가 꺼짐

## LCD
* 4 X 16 크기의 캐릭터 LCD를 4-bit 데이터 핀으로 제어
* 8-bit 모드에 비해 데이터 전송 시간이 더 걸리지만 I/O 선의 개수를 줄일 수 있었음
* 4-bit 모드이기 때문에 소프트웨어적으로 4-bit 데이터 전송을 두 번 실행하도록 구현
* 정수 데이터는 LCD 상에 그대로 표현되지 않아 가령 네 자리의 정수를 천의 자리, 백의 자리, 십의 자리, 일의 자리로 분리하여 배열에 각각 저장하고, 분리된 각각의 정수를 ASCII 코드로 변환하여 LCD 상에 출력
* LASER가 과녁에 명중하면 과녁 부분에서 생성된 임의의 ADC 값을 통해 명중한 과녁의 위치를 파악하여 LCD 상에 표시하도록 구현
* 남은 발 수가 0이 되면 게임을 종료한 이후 5초 뒤에 자동으로 다시 시작한다는 메시지를 LCD에 표시


---
### LSG 과녁 부분 HW 구성도
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_targetHW.jpg)
### LSG 총 부분 HW 구성도
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_gunHW.jpg)
### LSG 과녁 부분 SW 구성도
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_targetSW.jpg)
### LSG 총 부분 SW 구성도
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_gunSW.jpg)


---
### LSG 총 부분 모델링
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_gun_3D.jpg)
### LSG 총 부분 ATmega328P 회로
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_gun_circuit.jpg)
### LSG 과녁 부분 ATmega128A 회로
![](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_target_circuit.jpg)
### [LSG 구현 영상](https://github.com/zbumjin97/Laser_Shooting_Game/blob/main/LSG_video.mp4)
* 영상이 다소 길어 다운로드 링크로 연결됩니다.
