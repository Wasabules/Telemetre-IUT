#include <Arduino.h>
#include <util/delay.h>
  
byte chiffres[10] = {0b0111111,0b0000110,0b0011011,0b1001111,0b0100110,0b1101101,0b1111101,0b0000111,0b1111111,0b1101111};    // Table de transcodage des chiffres sur les digits
byte selectDigit[4] = {0b000111, 0b001011, 0b001101, 0b001110};                                                               // Table de transcodage du digit à allumer/sélectionner
int nombre;
int digitOn = 0;

int receptionSignal,tempsEmis, finSignal;

float distance(){   // Retourne la valeur de la distance en cm (2 chiffre après virgule)
  PORTD |= (1 << PD2);
  delayMicroseconds(10);
  PORTD &=~ (1 << PD2);

  
  while(bit_is_clear(PIND,PD3));    // On attend que le niveau soit haut
  receptionSignal = micros();       // On note le temps
  while(bit_is_set(PIND,PD3));      // On attend que le niveau repasse à bas
  finSignal = micros();             // On note le temps de fin

  tempsEmis = finSignal - receptionSignal;  // On obtient la période du signal

  Serial.println(tempsEmis);
  return (tempsEmis/58);            // En divisant par 58, on retourne la valeur en centimètre
}

void afficheur(int digit, int numero){    // Affiche un numéro sur un digit
  PORTB = selectDigit[digit];
  PORTC = chiffres[numero];
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

/* int sommeNombre, moyenneNombre;

double readDistance(){
  double moyenne;
  double moyenneFinal;
  moyenne = 0;
  for (int j =0; j<100; j++) 
  {
    moyenne = moyenne + distance();
  }
  
  moyenneFinal = moyenne/100;
  return moyenneFinal;
} */

void setupTimer7digit(){      // Configure le timer dédié aux 4 digits
  TIMSK1 |= (1 << TOIE1);                 // Activation de l'interruption par overflow du Timer 1
  TCNT1 = 0;                              // Mise à 0 du compteur
  TCCR1B = (1 << CS11) | (1 << CS10);     // Selection du prescaler du timer1 (/64), celui-ci changera directement la fréquence de rafraichissement des digits
  sei();                                  // Autorisation des interruptions
}

void setupPort(){             // Configure les ports pour les Digits & Module Ultra son
  // Setup du 7 digit
  DDRC = 0b1111111;         // Mise en sortie du port C pour les chiffres des digitis
  DDRB = 0b111111;          // Mise en sortie du port B pour la selection des digits 
  PORTC = 0b0000000;        // Mise à 0 des sorties
  PORTB = 0b000000;         // Mise à 0 des sorties

  // Setup du module ultra son
  DDRD |= (1 << PD2);       // Mise en sortie PD2 pour le "Trigger" du module ultra son
  DDRD &=~ (1 << PD3);      // Mise en entrée PD3 pour le "Echo" du module ultra son
}

void setup() {  
  Serial.begin(9600);
  setupPort();
  setupTimer7digit();
  Serial.println("Démarrage");
}

void loop() {
  nombre = distance()*100;
  delay(500);
}