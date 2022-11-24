#include "ultrasound.h"
#include "SumoEngineController.h"
#include "Button.h"
#include <BluetoothSerial.h>
#include <SSD1306.h>

// debug
#define DEBUG_SHARP 1
#define DEBUG_ULTRASOUND 1
#define DEBUG_STATE 1
#define TICK_DEBUG 500
#define TICK_DEBUG_STRATEGY 500
#define TICK_DEBUG_SHARP 500
#define TICK_DEBUG_ULTRASOUND 500
unsigned long currentTimeSharp = 0;
unsigned long currentTimeUltrasound = 0;
unsigned long currentTimeEstrategy = 0;

// configuramos el Serial Bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

// Variables y constantes para los sensores de distancia
#define PIN_ECCHO_LEFT 25
#define PIN_TRIGG_LEFT 33
#define PIN_ECCHO_RIGHT 27
#define PIN_TRIGG_RIGHT 14
#define OLD_RIVAL 70
#define RIVAL 30
#define RIVAL_MID 20
#define RIVAL_IS_CLOSE 10

//VENI VENI LONG
#define RIVAL_LONG 70
#define RIVAL_LONG_IS_CLOSE 50

float leftDistance;
float rightDistance;

// Variables y constantes para los motores
#define PIN_ENGINE_DIR_RIGHT 23 // DIR
#define PIN_ENGINE_PWM_RIGHT 22 // PWM
#define PIN_ENGINE_DIR_LEFT 18 // DIR
#define PIN_ENGINE_PWM_LEFT 19  // PWM
#define AVERAGE_SPEED 100
#define SEARCH_SPEED 80
#define MODERATE_ATTACK_SPEED 120
#define MEDIUM_SPEED 150
#define ATTACK_SPEED 255

// Pines para los botones
#define PIN_BUTTON_START 26

// variables y constantes para la pantalla oled
#define PIN_SDA 16
#define PIN_SCL 17

// instancio los objetos
SSD1306 display(0x3C, PIN_SDA, PIN_SCL); // inicializa pantalla con direccion 0x3C
EngineController *Bati = new EngineController(PIN_ENGINE_DIR_RIGHT, PIN_ENGINE_PWM_RIGHT, PIN_ENGINE_DIR_LEFT, PIN_ENGINE_PWM_LEFT);
Ultrasound *ultrasoundRight = new Ultrasound(PIN_TRIGG_RIGHT, PIN_ECCHO_RIGHT);
Ultrasound *ultrasoundLeft = new Ultrasound(PIN_TRIGG_LEFT, PIN_ECCHO_LEFT);
Button *start = new Button(PIN_BUTTON_START);

// funciones para imprimir en el serial bloutooth
void printSharp()
{
    if (millis() > currentTimeUltrasound + TICK_DEBUG_ULTRASOUND)
    {
        currentTimeSharp = millis();
        SerialBT.print("Right dist: ");
        SerialBT.print(leftDistance);
        SerialBT.print("  //  ");
        SerialBT.print("Left dist: ");
        SerialBT.println(rightDistance);
    }
}

// Funcion para la lectura de los sensores
void sensorsReading()
{
    leftDistance = ultrasoundRight->SensorRead();
    rightDistance = ultrasoundLeft->SensorRead();
}

// este enum esta aca porque es una variable que va a ser usadas en las demas maquina de estados
enum strategies
{
    SELECT_REPOSITIONING,
    SELECT_STRATEGY,
    VENI_VENI,
    FULL,
    OLD_SCHOOL,
    SEARCH_ATACK,
    VENI_VENI_LONG
};
int strategies = SELECT_REPOSITIONING;

enum veniVeni
{
    STAND_BY_VENI_VENI,
    SEARCH_VENI_VENI,
    TURN_RIGHT_VENI_VENI,
    TURN_LEFT_VENI_VENI,
    MODERATE_ATTACK_VENI_VENI,
    ATTACK_VENI_VENI
};
int veniVeni = STAND_BY_VENI_VENI;
void VeniVeni()
{
    switch (veniVeni)
    {
    case STAND_BY_VENI_VENI:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy VeniVeni");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();
        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy VeniVeni");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            veniVeni = SEARCH_VENI_VENI;
        }
        break;
    }

    case SEARCH_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_RIGHT_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_LEFT_VENI_VENI:
    {
        Bati->Left(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case MODERATE_ATTACK_VENI_VENI:
    {
        Bati->Stop();
        if (rightDistance <= RIVAL_IS_CLOSE && leftDistance <= RIVAL_IS_CLOSE)
            veniVeni = ATTACK_VENI_VENI;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            veniVeni = TURN_LEFT_VENI_VENI;
        break;
    }

    case ATTACK_VENI_VENI:
    {        
        Bati->Forward(ATTACK_SPEED, ATTACK_SPEED);
      
          if (rightDistance > RIVAL && leftDistance > RIVAL)
            veniVeni = SEARCH_VENI_VENI;
          if (rightDistance <= RIVAL && leftDistance > RIVAL)
            veniVeni = TURN_RIGHT_VENI_VENI;
          if (rightDistance > RIVAL && leftDistance <= RIVAL)
            veniVeni = TURN_LEFT_VENI_VENI;

        break;
    }
    }
}


void SearchAtack()
{
    switch (veniVeni)
    {
    case STAND_BY_VENI_VENI:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy SearchAtack");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();
        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy SearchAtack");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            veniVeni = SEARCH_VENI_VENI;
        }
        break;
    }

    case SEARCH_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_RIGHT_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_LEFT_VENI_VENI:
    {
        Bati->Left(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case MODERATE_ATTACK_VENI_VENI:
    {
        Bati->Forward(AVERAGE_SPEED, AVERAGE_SPEED);
        if (rightDistance <= RIVAL_LONG_IS_CLOSE && leftDistance <= RIVAL_LONG_IS_CLOSE)
            veniVeni = ATTACK_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        break;
    }

    case ATTACK_VENI_VENI:
    {        
        Bati->Forward(ATTACK_SPEED, ATTACK_SPEED);
      
          if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
          if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
          if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;

        break;
    }
    }
}

void VeniVeniLong()
{
    switch (veniVeni)
    {
    case STAND_BY_VENI_VENI:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy VeniVeniLong");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();
        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy VeniVeni");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            veniVeni = SEARCH_VENI_VENI;
        }
        break;
    }

    case SEARCH_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_RIGHT_VENI_VENI:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case TURN_LEFT_VENI_VENI:
    {
        Bati->Left(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = MODERATE_ATTACK_VENI_VENI;
        break;
    }

    case MODERATE_ATTACK_VENI_VENI:
    {
        Bati->Stop();
        if (rightDistance <= RIVAL_LONG_IS_CLOSE && leftDistance <= RIVAL_LONG_IS_CLOSE)
            veniVeni = ATTACK_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
        if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
        if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;
        break;
    }

    case ATTACK_VENI_VENI:
    {        
        Bati->Forward(ATTACK_SPEED, ATTACK_SPEED);
      
          if (rightDistance > RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = SEARCH_VENI_VENI;
          if (rightDistance <= RIVAL_LONG && leftDistance > RIVAL_LONG)
            veniVeni = TURN_RIGHT_VENI_VENI;
          if (rightDistance > RIVAL_LONG && leftDistance <= RIVAL_LONG)
            veniVeni = TURN_LEFT_VENI_VENI;

        break;
    }
    }
}


enum full{
    STAND_BY_FULL,
    ATTACK_FULL
};
int full = STAND_BY_FULL;

void Full(){
    switch (full)
    {
    case STAND_BY_FULL:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy Full");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();

        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy Full");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            full = ATTACK_FULL;
            }
            break;
    }
    case ATTACK_FULL:
    {
        Bati->Forward(ATTACK_SPEED,ATTACK_SPEED);
        break;
    }
   
}
}
enum oldSchool{
    STAND_BY_OLD_SCHOOL,
    SEARCH_OLD_SCHOOL,
    TURN_RIGHT_OLD_SCHOOL,
    TURN_LEFT_OLD_SCHOOL,
    MODERATE_ATTACK_OLD_SCHOOL,
    ATTACK_OLD_SCHOOL
};
int oldSchool = STAND_BY_OLD_SCHOOL;

void OldSchool(){
    switch (oldSchool)
    {
    case STAND_BY_OLD_SCHOOL:
    {
        Bati->Stop();
        display.clear();
        display.drawString(19, 0, "Strategy Old school");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 28, "Press Star()");
        display.display();
        if (start->GetIsPress())
        {
            display.clear();
            display.drawString(19, 0, "Strategy Old school");
            display.drawString(0, 9, "---------------------");
            display.drawString(0, 28, "Iniciando en 5");
            display.display();
            delay(5000);
            oldSchool = SEARCH_OLD_SCHOOL;
            }
            break;
    }
    case SEARCH_OLD_SCHOOL:
    {
       Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            oldSchool = TURN_RIGHT_OLD_SCHOOL;
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            oldSchool = TURN_LEFT_OLD_SCHOOL;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            oldSchool = MODERATE_ATTACK_OLD_SCHOOL;
        break;
    }
    case TURN_RIGHT_OLD_SCHOOL:
    {
        Bati->Right(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            oldSchool = TURN_LEFT_OLD_SCHOOL;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            oldSchool = SEARCH_OLD_SCHOOL;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            oldSchool = MODERATE_ATTACK_OLD_SCHOOL;
        break;
    }
    case TURN_LEFT_OLD_SCHOOL:
    {
        Bati->Left(SEARCH_SPEED, SEARCH_SPEED);
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            oldSchool = TURN_LEFT_OLD_SCHOOL;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            oldSchool = SEARCH_OLD_SCHOOL;
        if (rightDistance <= RIVAL && leftDistance <= RIVAL)
            oldSchool = MODERATE_ATTACK_OLD_SCHOOL;
        break;
    }
    case MODERATE_ATTACK_OLD_SCHOOL:
    {
        if (rightDistance <= RIVAL_MID && leftDistance <= RIVAL_MID)
            oldSchool = ATTACK_OLD_SCHOOL;
        if (rightDistance > RIVAL && leftDistance > RIVAL)
            oldSchool = SEARCH_OLD_SCHOOL;
        if (rightDistance <= RIVAL && leftDistance > RIVAL)
            oldSchool = TURN_RIGHT_OLD_SCHOOL;
        if (rightDistance > RIVAL && leftDistance <= RIVAL)
            oldSchool = TURN_LEFT_OLD_SCHOOL;
        break;
    }
    case ATTACK_OLD_SCHOOL:
    {        
        Bati->Forward(ATTACK_SPEED, ATTACK_SPEED);
        if (rightDistance <=RIVAL_MID || leftDistance <=RIVAL_MID)
        {
          if (rightDistance > RIVAL && leftDistance > RIVAL)
            oldSchool = SEARCH_OLD_SCHOOL;
          if (rightDistance <= RIVAL && leftDistance > RIVAL)
            oldSchool = TURN_RIGHT_OLD_SCHOOL;
          if (rightDistance > RIVAL && leftDistance <= RIVAL)
            oldSchool = TURN_LEFT_OLD_SCHOOL;
        }
        
        break;
    }

        
    }
}

enum reposittioningMenu
{
    REPOSITIONING_MENU,
    TURN_45_MENU,
    TURN_90_MENU,
    TURN_135_MENU
};
int reposittioningMenu = REPOSITIONING_MENU;

void ReposittioningMenu()
{
    switch (reposittioningMenu)
    {
    case REPOSITIONING_MENU:
    {
        display.clear();
        display.drawString(19, 0, "Select Turn");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 18, "Rotate 90");
        display.drawString(0, 28, "Rotate 180");
        display.display();
        
        if (start->GetIsPress())
        {
            strategies = SELECT_STRATEGY;
        }
        break;
    }
    
    }
}

enum menu
{
    MAIN_MENU,
    VENI_VENI_MENU,
    FULL_MENU,
    OLD_SCHOOL_MENU,
    SEARCH_ATACK_MENU,
    VENI_VENI_LONG_MENU,
};
int menu = MAIN_MENU;


int menu_cont = VENI_VENI;
String isSelected(int menu_cont,int strategy){
  bool isActive = menu_cont == strategy ;
  return isActive ? "  <--- ":" ";
}

bool press_anterior= true;
int cont_press = 0;
void Menu()
{
    switch (menu)
    {
    case MAIN_MENU:
    {
        display.clear();
        display.drawString(19, 0, "Select strategy");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "VeniVeni"+isSelected(menu_cont,VENI_VENI_MENU) );
        display.drawString(0, 28, "Full"+isSelected(menu_cont,FULL_MENU));
        display.drawString(0, 37, "Old School"+isSelected(menu_cont,OLD_SCHOOL_MENU));
         display.drawString(0, 46, "SearchAtack "+isSelected(menu_cont,SEARCH_ATACK_MENU) );
        display.drawString(0, 55, "VeniVeniLong "+isSelected(menu_cont,VENI_VENI_LONG_MENU) );
          

        display.display();
        bool press_actual =start->ReadValue();
        if (start->GetIsPress() && press_anterior !=press_actual) {
          menu_cont++;
          cont_press=0;
          if(menu_cont>VENI_VENI_LONG_MENU){
            menu_cont=VENI_VENI_MENU;
          }
          //menu = VENI_VENI_MENU;
        }
        
        press_anterior=press_actual;

        if( press_anterior ==press_actual && press_actual ==false){
          cont_press++;
        }


        if(cont_press>6){
            menu=menu_cont;
        }

        break;
    }

    case VENI_VENI_MENU:
    {
        display.clear();
        display.drawString(19, 0, "Select strategy");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "VeniVeni");
        display.display();
        if (start->GetIsPress()) strategies = VENI_VENI;
        break;
    }
    case FULL_MENU:
    {
        display.clear();
        display.drawString(19, 0, "Select strategy");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "Full");
        display.display();
        if (start->GetIsPress()) strategies = FULL;
        break;
    }
        case VENI_VENI_LONG_MENU:
    {
         display.clear();
        display.drawString(19, 0, "Select strategy");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "VeniVeniLong");
        display.display();
        if (start->GetIsPress()) strategies = VENI_VENI_LONG;
        break;
    }

            case SEARCH_ATACK_MENU:
    {
         display.clear();
        display.drawString(19, 0, "Select search_ataque_menu");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "search atack menu");
        display.display();
        if (start->GetIsPress()) strategies = SEARCH_ATACK;
        break;
    }
    case OLD_SCHOOL_MENU:
    {
        display.clear();
        display.drawString(19, 0, "Select strategy");
        display.drawString(0, 9, "---------------------");
        display.drawString(0, 19, "Old School");
        display.display();
        if (start->GetIsPress()) strategies = OLD_SCHOOL;
        break;
    }
    }
}

void Strategies()
{   
    switch (strategies)
    {
    case SELECT_REPOSITIONING:
    {
        ReposittioningMenu();
        break;
    }

    case SELECT_STRATEGY: 
    {
        Menu();
        break;
    }

    case VENI_VENI:
    {
        VeniVeni();
        break;
    }

    case VENI_VENI_LONG:{
       VeniVeniLong();
        break;
    }

      case SEARCH_ATACK:{
       SearchAtack();
        break;
    }

    case FULL:
    {
        Full();
        break;
    }

    case OLD_SCHOOL:
    {
        OldSchool();
        break;
    }
    }
}

void setup()
{
    SerialBT.begin("Bati");
    Serial.begin(115200);
    Serial.println("HOLIS");
    // Bati->DisabledMotorLeft();
    // Bati->DisabledMotorRight();
    start->SetFlanco(LOW);
    display.init();
}

void loop()
{
    sensorsReading();
    Strategies();
    //Full();
    //OldSchool();
    if (DEBUG_ULTRASOUND) printSharp();
}