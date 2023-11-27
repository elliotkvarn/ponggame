/*
  File: pong.ino
  Author: Elliot Kvarnström
  Date: 2023-11-20
  Description: Ett enkelt pong-spel som använder joysticks och en OLED-skärm
*/


#include <U8glib.h>
#include <Wire.h>

U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NO_ACK); // definerar skärmen för U8G biblioteket

int xValueA = 0; // X-värde på ena joysticken (används inte men sparar ändå)
int yValueA = 0; // Y-värde
int xValueB = 0; // X-värde på andra joysticken
int yValueB = 0; // Y-värde

#define VRX_PIN  A0 // För joystick 1: definerar om A0 till VRX_PIN
#define VRY_PIN  A1 // A1 till VRY_PIN
#define VRX_PINB  A2 // För joystick 2: definerar om A2 till VRX_PINB
#define VRY_PINB  A3 // A3 till VRY_PINB

//definerar skärmens storlek, används senare
int screenHeight = 64;
int screenWidth = 128;

//variabler som hör till spelarna och deras paddles
int py = 25; // står för player y; är y koordinaten på övre vänstra hörnet på paddlen spelare 1 styr
int p2y = 25; // står för player 2 y med samma princip som spelare 1
int px = 5; // står för player x; är x-koordinaten på player 1 och ändras aldrig
int p2x = 119; // samma för spelare 2
int pWidth = 4; // paddle width i antal pixlar; defineras bara för att lätt kunna ändra om man t.ex skulle vilja lägga till en inställningsmeny
int pHeight = 14; // paddle height i antal pixlar
int p1score=0; // score för player 1
int p2score=0; // score för player 2
int paddlemin = 2; // paddle movementen mappas om från joystick-inputen. detta är lägsta hastigheten på paddlespeed...
int paddlemax = 4; // ... och detta är högsta

//variabler som hör till bollen
int bx = 64; // ball x; x-koordinaten i bollens centrum
int by = 32; // ball y; y-koordinaten i bollens centrum
int br = 3; // bollens radie från mitten ()
int xinc = 3; // x incrementation; antal pixlar bollen rör sig på skärmen i x-axeln per update. startar på 3 men slumpas varje gång bollen byter riktning
int speedmin = 3; // minsta värdet x incrementationen kan slumpas till
int speedmax = 6; // högsta värdet x incrementationen kan slumpas till
int yinc = 0;   /* y incrementation; antal pixlar bollen rör sig i y-axeln
                börjar med 0 alltså åker den rakt fram. detta var ett lätt sätt att få bollen att kunna 
                studsa runt utan att behöva beräkna vinkeln eller hypotenusan.
                  */
int skew = 3; // hur mycket y incrementationen slumpas när den studsar på en paddle. går från 1 till 1+skew på nedre 3/7 och negativa för övre 3/7



/*
  setup()
  Setup, körs en gång när programmet startas. 
  Inga parametrar eller returns. 
*/
void setup() {
  Serial.begin(9600); //startar seriell kommunikation som används för att se score, hann inte skriva ut det på skärmen på något snyggt sätt
  oled.setFont(u8g_font_helvB10); // sätter font till skärmen (användes inte men har kvar för eventuella förbättringar)
}

/*
  updateOled
  Uppdaterar skärmen. Målar paddlesarna och bollen
  Inga parametrar eller returns
*/

void updateOled(){
  oled.firstPage(); 
  do {
    oled.drawBox(px,py,pWidth,pHeight); // spelare 1:s paddle
    oled.drawBox(p2x,p2y,pWidth,pHeight); // spelares 2:s paddle
    oled.drawDisc(bx, by, br, U8G_DRAW_ALL); // målar bollen
    } 
     while(oled.nextPage());
}


/*
  readJoystick()
  Läser av joysticken och sköter movementen för båda spelares paddles
  Inga perametrar eller returns
*/

void readJoystick() {
  int xValueA = analogRead(VRX_PIN);  // läser av joysticksen och uppdaterar variablerna med deras values
  int yValueA = analogRead(VRY_PIN);  //
  int xValueB = analogRead(VRX_PINB); //
  int yValueB = analogRead(VRY_PINB); //

  //player 1 movement
  if(yValueA < 475) {   // joysticken har ett värde från 0 till 1023. om den är under 475 så åker paddlen upp
    if(py > 0)          //kollar så inte spelaren är över spelområdet
    {
      py -= map(yValueA, 503,0, paddlemin, paddlemax); //mappar om till paddlespeed och tar bort från paddle-y
    }
  }
  if(yValueA > 550) {   //om den är över 550 (lite över mitten) åker paddlen ner
    if(py < 49)         //kollar så inte spelaren är under spelområdet
    {       
      py += map(yValueA, 503,1023, paddlemin, paddlemax); //mappar om till paddlespeed och lägger till från paddle-y
    }
  }

  //player 2 movement: allt är samma som för spelare 1 fast för spelare 2
  if(yValueB < 475) {
    if(p2y > 0)
    {
      p2y -= map(yValueB, 503,0, paddlemin, paddlemax);
    }
  }
  if(yValueB > 550) {
    if(p2y < 49) {
      p2y += map(yValueB, 503,1023, paddlemin, paddlemax);
    }
  }
}


/*
  ballMovement()
  Sköter alla regler kring bollens rörelse och studsande samt scoring-systemet
  Inga parametrar eller returns
*/

void ballMovement(){
    if (by+yinc >= (screenHeight-br) || (by+yinc <= br)) // studsar bollen om den hade åkt över eller under spelplanen            
    {
    yinc *= -1; // gör att bollens y incrementation byter håll på skärmen
    }
  
  if (xinc <= 0 && bx <= br) //om den träffar väggen längst åt vänster när bollen åker mot vänstra väggen
  {
    bx=64; //resetta bollens x-koordinat till mitten av skärmen
    by=32; //reset y
    xinc=2; //gör bollen långsam och åker mot höger 
    yinc=0; //tar bort y movement på bollen
    p2score ++; //lägger till score för spelare 2 (spelare 2 är på höger sidan)
    Serial.println("Player 2 score:" + String(p2score));
  }

  if (xinc >=0 && bx >= 121) //om den träffar väggen längst åt höger när bollen åker mot högra väggen
  {
    bx=64;
    by=32;
    xinc=-2;
    yinc=0;
    p1score ++;
    Serial.println("Player 1 score:" + String(p1score));
  }
  
  //om bollen träffat någon paddlen
  //PLAYER 1
  if((xinc < 0) && (bx+br >= px) && (by+br >= py) && (by <= py+pHeight+br) && (bx-br <= px+pWidth)) //kollar om bollen krockar med vänstra paddlen
  {
    xinc = random(speedmin,speedmax); // ändrar riktning på bollen i slumpad speed
    if (by <= py + (3 * pHeight) / 7) { // studsar bollen uppåt om den studsar på de övre 3/7 av paddlen
      yinc = -random(1, skew + 1);
    }
    if (by >= py + (4 * pHeight) / 7){ // studsar bollen nedåt om den studsar på de nedre 3/7 av paddlen
      yinc = random(1, skew + 1);
    }
  }

  //PLAYER 2: samma pricip som för spelare 1 
  if((xinc >0) && (bx+br >= p2x) && (by+br >= p2y) && (by <= p2y+pHeight+br) && (bx-br <= p2x+pWidth))
  {
    // Reverse its x-direction
    xinc = -random(speedmin,speedmax);
    if (by <= p2y + (3 * pHeight) / 7) {
      yinc = -random(1, skew + 1);
    }
    if (by >= p2y + (4 * pHeight) / 7){
      yinc = random(1, skew + 1);
    }
  }
  
  bx += xinc; //ändrar bollens x position för varje skärmupdate med nuvarande x-incrementation
  by += yinc; //ändrar bollens y position
}

/*
  loop() -- Av någon anledning funkade inte koden om jag inte hade loopen sist i programmet --
  Körs varje update på arduinon. Kallar endast på de andra funktionerna så hade egentligen
  kunnat vara en lång loop.
*/

void loop() {
  readJoystick();
  updateOled();
  ballMovement();
}
