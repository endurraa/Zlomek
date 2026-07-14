#include <Servo.h>
#include <Stepper.h>
#include <math.h> 

// --- 1. DEFINICJE STANÓW ---
enum StanRobota {
  KALIBRACJA,
  JAZDA_SWOBODNA,
  HAMOWANIE_AWARYJNE,
  COFANIE,
  SKANOWANIE_SEKTOROWE,
  OBLICZANIE_TRASY,
  FAZA_1B_ARC,         
  FAZA_1D_KONTRA,      
  FAZA_2_STABILIZACJA  
};

StanRobota aktualnyStan = KALIBRACJA;

// --- 2. SPRZĘT ---
const int stepsPerRevolution = 2048; 
Stepper myStepper(stepsPerRevolution, A0, A2, A1, A3);
Servo headServo;

#define TRIG_PIN 11
#define ECHO_PIN 12
#define SERVO_PIN 6
#define MOTOR_PIN_A 3
#define MOTOR_PIN_B 5

// --- 3. PARAMETRY KONSTRUKCYJNE ---
const float KROKI_NA_STOP    = 4.98;   
const float SZER_ROBOTA      = 20.0;   
const float DLUG_ROBOTA      = 30.0;   
const float MS_NA_1_CM       = 80.0;   
const float MARGINES         = 10.0;   
const float DOCELOWY_SKOS    = 25.0;   
const float MNOZNIK_DYSTANSU = 1.2; 

int v1 = 125; 
int v2 = 105; 

// --- ZMIENNE OPERACYJNE ---
int kierunek = 1; 
float wyliczonaAlfa = 30.0; 
int krokiStery = 0;
unsigned long t_faza = 0;
unsigned long timerStanu = 0;
float d_straight_calc = 0;
float d_arc_calc = 0;

int servoPos = 90;
int servoDir = 6;
unsigned long servoTimer = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
  headServo.attach(SERVO_PIN);
  myStepper.setSpeed(12);
  
  headServo.write(90);
  
  unsigned long t = millis();
  while(millis() - t < 3400) myStepper.step(-10); 
  delay(500);
  t = millis();
  while(millis() - t < 1700) myStepper.step(10);  
  
  aktualnyStan = JAZDA_SWOBODNA;
  Serial.println("Zlomek naprawiony i gotowy!");
}

void loop() {
  switch (aktualnyStan) {
    case JAZDA_SWOBODNA:       obslugaJazdy(); break;
    case HAMOWANIE_AWARYJNE:   obslugaHamowania(); break;
    case COFANIE:              obslugaCofania(); break;
    case SKANOWANIE_SEKTOROWE: obslugaSkanowania(); break;
    case OBLICZANIE_TRASY:     wykonajObliczenia(); break;
    case FAZA_1B_ARC:          obslugaFazy1B(); break;
    case FAZA_1D_KONTRA:       obslugaFazy1D(); break;
    case FAZA_2_STABILIZACJA:  obslugaFazy2(); break;
  }
}

// --- LOGIKA MANEWRU ---

void wykonajObliczenia() {
  float targetY = ((SZER_ROBOTA / 2.0) + MARGINES) * MNOZNIK_DYSTANSU;
  float alfaRad = wyliczonaAlfa * (M_PI / 180.0);
  float thetaRad = DOCELOWY_SKOS * (M_PI / 180.0);
  float R = DLUG_ROBOTA / tan(alfaRad);
  
  d_arc_calc = (R * thetaRad) * MNOZNIK_DYSTANSU; 
  float y_arc = R * (1.0 - cos(thetaRad));
  float y_rem = targetY - y_arc;
  d_straight_calc = (y_rem > 0) ? (y_rem / sin(thetaRad)) : 0;
  
  krokiStery = (int)(wyliczonaAlfa * KROKI_NA_STOP);

  if (kierunek == 1) { 
    Serial.println("Manewr: PRAWO (x1.5)");
    myStepper.step(krokiStery); 
    t_faza = (unsigned long)(d_arc_calc * MS_NA_1_CM * 1.5); 
  } 
  else { 
    Serial.println("Manewr: LEWO (Standard)");
    myStepper.step(-krokiStery); 
    t_faza = (unsigned long)(d_arc_calc * MS_NA_1_CM); 
  }

  timerStanu = millis();
  aktualnyStan = FAZA_1B_ARC;
}

void obslugaFazy1B() {
  jedzDoPrzodu(v2);
  if (millis() - timerStanu > t_faza) {
    zatrzymajSie();
    if (kierunek == 1) myStepper.step(-2 * krokiStery);
    else myStepper.step(2 * krokiStery);

    t_faza = (unsigned long)((d_straight_calc * 0.5) * MS_NA_1_CM);
    timerStanu = millis();
    aktualnyStan = FAZA_1D_KONTRA;
  }
}

void obslugaFazy1D() {
  jedzDoPrzodu(v2);
  if (millis() - timerStanu > t_faza) {
    zatrzymajSie();
    if (kierunek == 1) myStepper.step(krokiStery);
    else myStepper.step(-krokiStery);

    t_faza = 400; 
    timerStanu = millis();
    aktualnyStan = FAZA_2_STABILIZACJA;
  }
}

void obslugaFazy2() {
  jedzDoPrzodu(v2);
  if (millis() - timerStanu > t_faza) {
    zatrzymajSie(); 
    headServo.write(90); 
    aktualnyStan = JAZDA_SWOBODNA; 
  }
}

void obslugaRozgladania() {
  if (millis() - servoTimer >= 25) {
    servoTimer = millis();
    servoPos += servoDir;
    if (servoPos > 125 || servoPos < 55) servoDir *= -1;
    headServo.write(servoPos);
  }
}

void obslugaJazdy() {
  jedzDoPrzodu(v1);
  obslugaRozgladania();
  if (zmierzDystans() < 45) {
    zatrzymajSie();
    aktualnyStan = HAMOWANIE_AWARYJNE; // TU BYŁ BŁĄD - POPRAWIONO
    timerStanu = millis();
  }
}

void obslugaSkanowania() {
  int najlepszeOkno = -1;
  int minKat = 180;
  for (int i = 0; i <= 12; i++) {
    int kat = 30 + (i * 10);
    headServo.write(kat);
    delay(180); 
    int d = zmierzDystans();
    if (d > 60) {
       int odchylenie = abs(90 - kat);
       if (odchylenie < minKat) { minKat = odchylenie; najlepszeOkno = kat; }
    }
  }
  if (najlepszeOkno == -1) { kierunek = 1; wyliczonaAlfa = 35; } 
  else { 
    kierunek = (najlepszeOkno < 90) ? 1 : -1; 
    wyliczonaAlfa = constrain(abs(90 - najlepszeOkno), 20, 40); 
  }
  headServo.write(90);
  aktualnyStan = OBLICZANIE_TRASY;
}

int zmierzDystans() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long d = pulseIn(ECHO_PIN, HIGH, 15000);
  return (d == 0) ? 999 : (d * 0.034 / 2);
}

void jedzDoPrzodu(int pwm) { analogWrite(MOTOR_PIN_A, pwm); digitalWrite(MOTOR_PIN_B, LOW); }
void jedzDoTylu(int pwm) { digitalWrite(MOTOR_PIN_A, LOW); analogWrite(MOTOR_PIN_B, pwm); }
void zatrzymajSie() { digitalWrite(MOTOR_PIN_A, LOW); digitalWrite(MOTOR_PIN_B, LOW); }
void obslugaHamowania() { zatrzymajSie(); if (millis() - timerStanu > 500) { aktualnyStan = COFANIE; timerStanu = millis(); } }
void obslugaCofania() { jedzDoTylu(v2); if (millis() - timerStanu > 1200) { zatrzymajSie(); aktualnyStan = SKANOWANIE_SEKTOROWE; } }
