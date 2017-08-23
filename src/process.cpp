#include "process.h"
#include "utilities.h"
#include "MessageWriter.h"
#include "storage.h"

uint8_t processCalibrate(){
  uint8_t cycle = 0;
  TIMSK3 |= (1 << OCIE3A);
  while(cycle < 5){
    Delay_ms(1000);
    cycle++;
  }
  Serial.println("Testing Functions!!!");
  TIMSK3 &= (0 << OCIE3A);
  return 1;
}

uint8_t processRun(){
  message_output_t outputMessage;
  writerPrepMessage(&outputMessage, NULL, NULL, 's', 'r0x60\0', '4000\0');
  writerSendMessage(&outputMessage);
  return 1;
}

uint8_t processHelp(){
  return 1;
}

uint8_t processAttributes(){
  return 1;
}

uint8_t processSave(){
  return 1;
}

uint8_t processLoad(){
  return 1;
}

const uint8_t fsrPin = 0;
volatile int16_t fsrReading;     // the analog reading from the FSR resistor divider
volatile int16_t fsrVoltage;     // the analog reading converted to voltage
volatile uint32_t fsrResistance;  // The voltage converted to resistance, can be very big so make "long"
volatile uint32_t fsrConductance;
volatile int32_t fsrForce;       // Finally, the resistance converted to force

ISR(TIMER3_COMPA_vect){
  fsrReading = analogRead(fsrPin);
  Serial.print("Analog reading = ");
  Serial.println(fsrReading);

  // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
  Serial.print("Voltage reading in mV = ");
  Serial.println(fsrVoltage);

  if (fsrVoltage == 0) {
    Serial.println("No pressure");
  } else {
    // The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V        yay math!
    fsrResistance = 5000 - fsrVoltage;     // fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 10000;                // 10K resistor
    fsrResistance /= fsrVoltage;
    Serial.print("FSR resistance in ohms = ");
    Serial.println(fsrResistance);

    fsrConductance = 1000000;           // we measure in micromhos so
    fsrConductance /= fsrResistance;
    Serial.print("Conductance in microMhos: ");
    Serial.println(fsrConductance);

    // Use the two FSR guide graphs to approximate the force
    if (fsrConductance <= 1000) {
      fsrForce = fsrConductance / 80 * 0.22481;
      Serial.print("Force in Pounds: ");
      Serial.println(fsrForce);
    } else {
      fsrForce = fsrConductance - 1000;
      fsrForce /= 30;
      Serial.print("Force in Pounds: ");
      Serial.println(fsrForce * 0.22481);
    }
  }
  Serial.println("--------------------");
}
