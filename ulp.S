#include "soc/soc_ulp.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_cntl_reg.h"

.bss//Declaracao de variaveis aqui


.text

	.global main
	main://Inicio do codigo (Entry point)
		WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S+12, 1, 1)//LED OFF (GPIO2 = LOW)


	loop:

		move r0, 0	//R0 = 0
		move r1, 0	//R1 = 0
		stage_rst	//stage_cnt = 0

		//Aqui sera feito um laco FOR() para 4 leituras do ADC e depois, tiramos uma media
		1:
			stage_inc 1	//stage_cnt++
			adc r1, 0, 7	//efetua a leitura ADC do GPIO34 e guarda no R1
		    	add r0, r0, r1	//R0 = R0 + R1 (guarda o total das leituras)
			wait 65000	
			wait 12000	//delay de 10ms
		jumps 1b, 4, lt		//retorna a label 1 enquanto stage_cnt < 4

		rsh r0, r0, 2		//divide o total das leituras (4) por 4
		jumpr wkup, 1600, ge	//se valor do ADC >= 1600, ativa o LED para indicar o evento, acorda o sistema principal e encerra o ULP
					//entretanto, o Timer0 foi ativado para 100ms, entao apos 100ms do HALT, ULP iniciara novamente

	jump loop//retorn ao loop


	wkup:

		WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S+12, 1, 1)//LED ON  (GPIO2 = 1)
		stage_rst
		1:
			stage_inc 1
			wait 32000
		jumps 1b, 125, lt	//delay de 500ms
		WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S+12, 1, 1)//LED OFF (GPIO2 = 0)

		wake	//Acorda o sistema principal
		halt	//Encerra o codigo do ULP, mas iniciara novamente apos 100ms
