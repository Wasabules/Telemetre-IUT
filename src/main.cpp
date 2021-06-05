#include <util/delay.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#define Dig1 PB0
#define Dig2 PB1
#define Dig3 PB2
#define Dig4 PB3
#define trigger PB4
#define echo PB5
#define bouton BP6

char chiffres[10] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};    // Table de transcodage des chiffres sur les digits
char selectDigit[4] = {0b00000111, 0b00001011, 0b00001101, 0b00001110};                                                               // Table de transcodage du digit à allumer/sélectionner
int unsigned nombre;
unsigned char digitOn = 0;
int unsigned timer0_overflow_count;

int unsigned receptionSignal,tempsEmis, finSignal;
int unsigned centimetre=0;

void distance(){   // Retourne la valeur de la distance en cm (2 chiffre après virgule)
  PORTB |= (1 << trigger);
  _delay_us(10);
  PORTB &=~ (1 << trigger);

  while(bit_is_clear(PINB,echo));    // On attend que le niveau soit haut
  TCNT0H = 0;
  TCNT0L = 0;
  TIMSK |= (1 << OCIE0A);
  while(bit_is_set(PINB,echo));      // On attend que le niveau repasse à bas
  TIMSK &=~ (1 << OCIE0A);
  nombre=centimetre;
    // En divisant par 58, on retourne la valeur en centimètre
}

void afficheur(unsigned char digit, unsigned int numero){    // Affiche un numéro sur un digit
  PORTB = selectDigit[digit];
  PORTA = chiffres[numero];
}



ISR(TIMER1_OVF_vect){         // Permet l'affichage des digits de façon fluide et qui n'influe pas sur la vitesse du programme (du moins, c'est négligeable)
  switch(digitOn)
  {
    case 3:
      afficheur(3, (nombre /1000)%10);
      digitOn = 2;
      break;
    case 2:
      afficheur(2, (nombre /100)%10);
      digitOn = 1;
      break;
    case 1:
      afficheur(1, (nombre /10)%10);
      digitOn = 0;
      break;
    case 0:
      afficheur(0, (nombre)%10);
      digitOn = 3;
      break;
  }
}

void setupTimer1_7digit(){              // Configure le timer dédié aux 4 digits
  TIMSK |= (1 << TOIE1);                 // Activation de l'interruption par overflow du Timer 1
  TCNT1 = 0;                              // Mise à 0 du compteur
  TCCR1B = (1 << CS12) | (1 << CS11);     // Selection du prescaler du timer1 (/64), celui-ci changera directement la fréquence de rafraichissement des digits

  sei();                                  // Autorisation des interruptions
}

ISR(TIMER0_COMPA_vect){
	centimetre++;
	PORTB &=~(1<<PB6); // Test du timer 2
}

void setupTimer0_duree(){
  OCR0A = 116;              // Valeur de comparaison (période de 58µs)
  TCCR0B |= (1 << CS01);    // Prescaler à 8
  TCCR0A |= (1 << CTC0);    // Mode comparaison CTC
  TIMSK &=~ (1 << OCIE0A);
}

void setupPort(){             // Configure les ports pour les Digits & Module Ultra son
  // Setup du 7 digit
  DDRA = 0b01111111;         // Mise en sortie du port C pour les chiffres des digitis
  DDRB = 0b00111111;          // Mise en sortie du port B pour la selection des digits 
  PORTA = 0b00000000;        // Mise à 0 des sorties
  PORTB = 0b00000000;         // Mise à 0 des sorties

  DDRB |= (1 << trigger);       // Mise en sortie PD2 pour le "Trigger" du module ultra son
  DDRB &=~ (1 << echo);      // Mise en entrée PD3 pour le "Echo" du module ultra son
}

 int main() { 
  setupPort();
  setupTimer1_7digit();
  setupTimer0_duree();

  while(1) {
    _delay_ms(10);
    nombre++;
  }

  return 0;
}