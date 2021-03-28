/*
 * adc.h
 *
 * Created: 27/03/2021 17:22:50
 *  Author: Karim Bouanane
 */

#ifndef ADC_H_
#define ADC_H_

#define SIZE_ANALOG_VALUE 7

typedef enum
{
    A0 = 0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7
} ADC_PIN;

class ANALOG
{
private:
    char analogStrValue[SIZE_ANALOG_VALUE];

public:
    void init();
    uint16_t getValueFrom(ADC_PIN pin);
    char *getStrValueFrom(ADC_PIN pin);
};

#endif /* ADC_H_ */