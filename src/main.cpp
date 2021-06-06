#include <util/delay.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define Dig1 PB0
#define Dig2 PB1
#define Dig3 PB2
#define Dig4 PB3
#define trigger PB5
#define echo PB4
#define bouton PB6

char chiffres[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111}; // Table de transcodage des chiffres sur les digits
char selectDigit[4] = {0b01000111, 0b01001011, 0b01001101, 0b01001110};                                                                       // Table de transcodage du digit à allumer/sélectionner
char OUT[3]= {0b01111000,0b00111110,0b00111111};
char outDigit[4] = {0b01000111, 0b01001011, 0b01001101, 0b01001111};   

int unsigned nombre;
unsigned char digitOn = 0;
int unsigned timer0_overflow_count;

int unsigned receptionSignal, tempsEmis, finSignal;
int unsigned centimetre = 0;
int unsigned moyenneDistance, sommeDistance;
int unsigned clic = 0;
bool numberSet = false;
int counter;


int distance()
{ // Retourne la valeur de la distance en cm (2 chiffre après virgule)

  centimetre=0;
  PORTB |= (1 << trigger);
  _delay_us(100);
  PORTB &=~ (1 << trigger);
  
  loop_until_bit_is_set(PINB, echo);
    
  TIMSK |= (1 << OCIE0A);
  loop_until_bit_is_clear(PINB, echo);

  TIMSK &= ~(1 << OCIE0A);
  return centimetre;

  // En divisant par 58, on retourne la valeur en centimètre

}

void afficheur(unsigned char digit, unsigned int numero)
{ // Affiche un numéro sur un digit
  if(nombre != 9999){
    PORTA = chiffres[numero];
    PORTB = selectDigit[digit];
  }else{
    PORTA = OUT[digit];
    PORTB = outDigit[digit];
  }
  
}

ISR(TIMER1_OVF_vect)
{ // Permet l'affichage des digits de façon fluide et qui n'influe pas sur la vitesse du programme (du moins, c'est négligeable)

  switch (digitOn)
  {
  case 3:
    afficheur(3, (nombre / 1000) % 10);
    digitOn = 2;
    break;
  case 2:
    afficheur(2, (nombre / 100) % 10);
    digitOn = 1;
    break;
  case 1:
    afficheur(1, (nombre / 10) % 10);
    digitOn = 0;
    break;
  case 0:
    afficheur(0, (nombre) % 10);
    digitOn = 3;
    break;
  }
}

void setupTimer1_7digit()
{                                     // Configure le timer dédié aux 4 digits
  TIMSK |= (1 << TOIE1);              // Activation de l'interruption par overflow du Timer 1
  TCNT1 = 0;                          // Mise à 0 du compteur
  TCCR1B = (1 << CS12) | (1 << CS11); // Selection du prescaler du timer1 (/64), celui-ci changera directement la fréquence de rafraichissement des digits

}


ISR(TIMER0_COMPA_vect)
{
  centimetre++;
}

void setupTimer0_duree()
{
  OCR0A = 5;           // Valeur de comparaison (58µs = 1cm - 58µs/100 = 1mm)
  // sauf que 58µs/100 équivaut à une fréquence de 1.7Mhz
  TCCR0B |= (1 << CS00); // Prescaler à 1 (8Mhz)
  TCCR0A |= (1 << CTC0); // Mode comparaison CTC
  TIMSK &= ~(1 << OCIE0A);
}

void setupPort()
{ // Configure les ports pour les Digits & Module Ultra son
  // Setup du 7 digit
  DDRA = 0b01111111;  // Mise en sortie du port C pour les chiffres des digitis
  DDRB = 0b00111111;  // Mise en sortie du port B pour la selection des digits
  PORTA = 0b00000000; // Mise à 0 des sorties
  PORTB = 0b00000000; // Mise à 0 des sorties

  DDRB |= (1 << trigger); // Mise en sortie PD2 pour le "Trigger" du module ultra son
  DDRB &= ~(1 << echo);   // Mise en entrée PD3 pour le "Echo" du module ultra son
  DDRB &=~ (1<< bouton);
  PORTB |= (1<< bouton);
}

void setupInterrupt(){
  GIMSK |= 1 << INT0;
}

void eteindre(){
  DDRB = 0;
  DDRA = 0;
  sleep_mode();
}

int main()
{
  cli();
  setupPort();          // Configuration des ports entrées/sorties
  setupTimer1_7digit(); // Configuration timer pour afficher chaque digit
  setupTimer0_duree();  // Configuration du timer pour la durée (pour la distance)
  setupInterrupt();     // On met en place l'interruption INT0
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Configuration du mode économie énergie (sortie sur INT0)
  
  sei(); // Autorisation des interruptions

  while (1)
  {
    if(counter <= 40){  // Au bout de 4s (environ, la distance prend du temps)
      if(bit_is_clear(PORTB,PB6)){  // S'il y a appuie sur le bouton
        counter = 0;                // Remise à 0 du compteur
        loop_until_bit_is_clear(PORTB,PB6);
      }
      // Calcul de somme de distance pour augmenter la fidélité du résultat
      sommeDistance = 0;
      for(int i = 0; i<10;i++){
        sommeDistance += distance();
      }
      moyenneDistance = sommeDistance/10;
      if(moyenneDistance >  1000){ // Si le nombre est suppérieur à sa limite de fonctionnement
        nombre = 9999;
      }else{
        nombre = moyenneDistance; // "nombre" est directement afficher sur l'écran et affichera la moyenne
      }
      
    }else{
      eteindre();   // On éxecute des opérations pour éteindre
    }
    counter++;    // On incrémente notre compteur
    _delay_ms(100);
  }

  return 0;
}