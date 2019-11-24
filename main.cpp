extern "C"
{
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <ulp/ulp.c>
#include <ulp_main.h>
#include <driver/adc.h>
#include <esp_log.h>
#include <esp_system.h>
}

static const char* tag = "ESP32";//TAG usada no LOG (Serial monitor)

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");//Inicio do binario
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");//Fim do binario
RTC_DATA_ATTR uint8_t wk_ctr = 0;//Variavel alocada na RTC_RAM, os dados se mantem intactos durante e apos o deep sleep

void initULP()
{
	//Configura o pino 34 para ADC com 12bits e 3.3V para uso do ULP
	adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_ulp_enable();

	//Configura o GPIO2 como saida no RTC Domain para uso do ULP
	rtc_gpio_init(GPIO_NUM_2);
	rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY);

	
	ulp_set_wakeup_period(0, 100000);//Configura o Timer0 de wakeup do ULP para 100ms
	ulp_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));//Carrega o binario na RTC_SLOW_MEM
	ulp_run((&ulp_main - RTC_SLOW_MEM) / sizeof(uint32_t));//Inicializa o ULP
}


extern "C" void app_main()
{
	//Essa variavel mantem a contagem de ciclos (Sleep -> Wake), apenas um contador a cada wakeup
	wk_ctr++;
	ESP_LOGI(tag, "Wakeup counter: %d", wk_ctr);//Mostra a quantidade de wakeups no Monitor

	if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_ULP)//Se o motivo do wakeup for do ULP
	{
		ESP_LOGI(tag, "ULP Wakeup");//Mostra que foi o ULP
	}
	else
	{
		ESP_LOGI(tag, "!= Wakeup");//Se nao, diz que foi um motivo diferente
		initULP();//Inicializa o ULP
	}

	esp_sleep_enable_ulp_wakeup();//Ativa o wakeup pelo ULP
	esp_deep_sleep(1800000000);//Dorme por 30 minutos
}
