#include <Arduino.h>
#include <util/delay.h>
  
#define Dig1 PB0
#define Dig2 PB1
#define Dig3 PB2
#define Dig4 PB3
#define trigger PB4
#define echo PB5
#define bouton BP6

byte chiffres[10] = {0b00111111,0b00000110,0b00011011,0b01001111,0b00100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};    // Table de transcodage des chiffres sur les digits
byte selectDigit[4] = {0b00000111, 0b00001011, 0b00001101, 0b00001110};                                                               // Table de transcodage du digit à allumer/sélectionner
int nombre;
int digitOn = 0;

int receptionSignal,tempsEmis, finSignal;

float distance(){   // Retourne la valeur de la distance en cm (2 chiffre après virgule)
  PORTB |= (1 << trigger);
  delayMicroseconds(10);
  PORTB &=~ (1 << trigger);

  
  while(bit_is_clear(PINB,echo));    // On attend que le niveau soit haut
  receptionSignal = micros();       // On note le temps
  while(bit_is_set(PINB,echo));      // On attend que le niveau repasse à bas
  finSignal = micros();             // On note le temps de fin

  tempsEmis = finSignal - receptionSignal;  // On obtient la période du signal

  float resultat = static_cast<float>(tempsEmis)/58;
  Serial.println(resultat);
  return (resultat);            // En divisant par 58, on retourne la valeur en centimètre
}

void afficheur(int digit, int numero){    // Affiche un numéro sur un digit
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

void setupTimer7digit(){      // Configure le timer dédié aux 4 digits
  TIMSK |= (1 << TOIE1);                 // Activation de l'interruption par overflow du Timer 1
  TCNT1 = 0;                              // Mise à 0 du compteur
  TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10);     // Selection du prescaler du timer1 (/64), celui-ci changera directement la fréquence de rafraichissement des digits
  sei();                                  // Autorisation des interruptions
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

void setup() { 
  setupPort();
  setupTimer7digit();
}

void loop() {
  nombre = distance()*100;
  delay(100);
}