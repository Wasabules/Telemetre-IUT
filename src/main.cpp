#include <Arduino.h>
#include <util/delay.h>
  
byte chiffres[10] = {0b0111111,0b0000110,0b0011011,0b1001111,0b0100110,0b1101101,0b1111101,0b0000111,0b1111111,0b1101111};

int nombre;
int digitOn = 1;

void afficheur(int digit, int numero){
  if(digit == 1){         // Unité
      PORTB = 0b000111;
      PORTC = chiffres[numero];
  }else if(digit == 2){   // Dizaine
      PORTB = 0b001011;
      PORTC = chiffres[numero];  
  }else if(digit == 3){   // Centaine
      PORTB = 0b001101;
      PORTC = chiffres[numero];
  }else if(digit == 4){   // Millier
      PORTB = 0b001110;
      PORTC = chiffres[numero];
  }
}

ISR(TIMER1_OVF_vect){
  switch(digitOn)
  {
    case 4:
      afficheur(4, (nombre /1000)%10);
      digitOn = 3;
      break;
    case 3:
      afficheur(3, (nombre /100)%10);
      digitOn = 2;
      break;
    case 2:
      afficheur(2, (nombre /10)%10);
      digitOn = 1;
      break;
    case 1:
      afficheur(1, (nombre)%10);
      digitOn = 4;
      break;
  }
}

void setup() {

  Serial.begin(9600);

  DDRC = 0b1111111;
  DDRB = 0b111111;
  PORTC = 0b0000000;
  PORTB = 0b000000;

  TIMSK1 |= (1 << TOIE1);
  TCNT1 = 0;
  TCCR1B = (1 << CS11) | (1 << CS10);
  
  sei();

  Serial.println("Démarrage");
}

void loop() {
  for(int i = 0; i<10000; i++){
    nombre = i;
    _delay_ms(10);
    Serial.println(nombre);
  }
}