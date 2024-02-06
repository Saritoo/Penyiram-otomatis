#include <Fuzzy.h>
#include <DHT.h>

//----------- Deklarasi Pin -----------
const int pinDHT = D3;
const int pinSoilMoisture = A0;
const int pinRelay = D5;

DHT dht(pinDHT, DHT11);
//------------ set Nilai Fuzzy ------------
Fuzzy *fuzzy = new Fuzzy();

// Fungsi keanggotaan Suhu Input
FuzzySet *cold = new FuzzySet(-10,-10, -10, 3);
FuzzySet *cool = new FuzzySet(0, 3, 12, 15);
FuzzySet *normal = new FuzzySet(12, 15, 24, 27);
FuzzySet *warm = new FuzzySet(24, 27, 36, 39);
FuzzySet *hot = new FuzzySet(36, 50, 50,50);

// Fungsi keanggotaan Kelembaban Input
FuzzySet *veryDry = new FuzzySet(0,0,0, 15);
FuzzySet *dry = new FuzzySet(10,20,20, 30);
FuzzySet *moist = new FuzzySet(10, 20, 40, 50);
FuzzySet *wet = new FuzzySet(40, 55,55,70);
FuzzySet *veryWet = new FuzzySet(40, 40,40,70);

// Fungsi keanggotaan Lama Menyiram Output
FuzzySet *off = new FuzzySet(0,0,0, 0);
FuzzySet *cepat = new FuzzySet(0, 20,20, 28);
FuzzySet *biasa = new FuzzySet(20, 28, 40, 48);
FuzzySet *lama = new FuzzySet(40, 60,60,60);

void setup() {
  Serial.begin(9600);
  pinMode(pinRelay, OUTPUT);
  dht.begin();
  fuzzySetup();
}

void loop() {
  float suhu = dht.readTemperature();
  float soilMoisture = map(analogRead(pinSoilMoisture), 1000, 4095, 0, 100);

  fuzzy->setInput(1, suhu);
  fuzzy->setInput(2, soilMoisture);
  fuzzy->fuzzify();
  float durasi = fuzzy->defuzzify(1);

  if (soilMoisture < 50) {
    pompaOn(durasi);
  } else {
    pompaOff();
  }

  displayData(suhu, soilMoisture, durasi);

  delay(1000);
}

void pompaOn(float duration) {
  digitalWrite(pinRelay, HIGH);
  delay(duration * 1000);
  digitalWrite(pinRelay, LOW);
}

void pompaOff() {
  digitalWrite(pinRelay, LOW);
}

void displayData(float temp, float soil, float duration) {
  Serial.println("------------------------------");
  Serial.print("Suhu : ");
  Serial.print(temp);
  Serial.println(" *C");
  Serial.print("Kelembaban :  ");
  Serial.print(soil);
  Serial.println(" %");
  Serial.print("Durasi :  ");
  Serial.print(duration);
  Serial.println(" Detik");
  Serial.println("------------------------------");
}


void fuzzySetup () {
  // FuzzyInput Suhu
  FuzzyInput *suhu = new FuzzyInput(1);

  suhu->addFuzzySet(cold);
  suhu->addFuzzySet(cool);
  suhu->addFuzzySet(normal);
  suhu->addFuzzySet(warm);
  suhu->addFuzzySet(hot);
  fuzzy->addFuzzyInput(suhu);

  // FuzzyInput Kelembaban Tanah
  FuzzyInput *kelembaban = new FuzzyInput(2);

  kelembaban->addFuzzySet(veryDry);
  kelembaban->addFuzzySet(dry);
  kelembaban->addFuzzySet(moist);
  kelembaban->addFuzzySet(wet);
  kelembaban->addFuzzySet(veryWet);
  fuzzy->addFuzzyInput(kelembaban);

  // FuzzyOutput RELAY (Lama Menyiram)
  FuzzyOutput *durasi = new FuzzyOutput(1);

  durasi->addFuzzySet(off);
  durasi->addFuzzySet(cepat);
  durasi->addFuzzySet(biasa);
  durasi->addFuzzySet(lama);
  fuzzy->addFuzzyOutput(durasi);

  
//----------------- FuzzyRule ---------------- 
// FuzzyRule 1
FuzzyRuleAntecedent *rule1Antecedent = new FuzzyRuleAntecedent();
rule1Antecedent->joinWithAND(veryDry, cold);

FuzzyRuleConsequent *rule1Consequent = new FuzzyRuleConsequent();
rule1Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule1 = new FuzzyRule(1, rule1Antecedent, rule1Consequent);
fuzzy->addFuzzyRule(fuzzyRule1);

// FuzzyRule 2
FuzzyRuleAntecedent *rule2Antecedent = new FuzzyRuleAntecedent();
rule2Antecedent->joinWithAND(veryDry, cool);

FuzzyRuleConsequent *rule2Consequent = new FuzzyRuleConsequent();
rule2Consequent->addOutput(lama);

FuzzyRule *fuzzyRule2 = new FuzzyRule(2, rule2Antecedent, rule2Consequent);
fuzzy->addFuzzyRule(fuzzyRule2);

// FuzzyRule 3
FuzzyRuleAntecedent *rule3Antecedent = new FuzzyRuleAntecedent();
rule3Antecedent->joinWithAND(veryDry, normal);

FuzzyRuleConsequent *rule3Consequent = new FuzzyRuleConsequent();
rule3Consequent->addOutput(lama);

FuzzyRule *fuzzyRule3 = new FuzzyRule(3, rule3Antecedent, rule3Consequent);
fuzzy->addFuzzyRule(fuzzyRule3);

// FuzzyRule 4
FuzzyRuleAntecedent *rule4Antecedent = new FuzzyRuleAntecedent();
rule4Antecedent->joinWithAND(veryDry, warm);

FuzzyRuleConsequent *rule4Consequent = new FuzzyRuleConsequent();
rule4Consequent->addOutput(lama);

FuzzyRule *fuzzyRule4 = new FuzzyRule(4, rule4Antecedent, rule4Consequent);
fuzzy->addFuzzyRule(fuzzyRule4);

// FuzzyRule 5
FuzzyRuleAntecedent *rule5Antecedent = new FuzzyRuleAntecedent();
rule5Antecedent->joinWithAND(veryDry, hot);

FuzzyRuleConsequent *rule5Consequent = new FuzzyRuleConsequent();
rule5Consequent->addOutput(lama);

FuzzyRule *fuzzyRule5 = new FuzzyRule(5, rule5Antecedent, rule5Consequent);
fuzzy->addFuzzyRule(fuzzyRule5);

// FuzzyRule 6
FuzzyRuleAntecedent *rule6Antecedent = new FuzzyRuleAntecedent();
rule6Antecedent->joinWithAND(dry, cold);

FuzzyRuleConsequent *rule6Consequent = new FuzzyRuleConsequent();
rule6Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule6 = new FuzzyRule(6, rule6Antecedent, rule6Consequent);
fuzzy->addFuzzyRule(fuzzyRule6);

// FuzzyRule 7
FuzzyRuleAntecedent *rule7Antecedent = new FuzzyRuleAntecedent();
rule7Antecedent->joinWithAND(dry, cool);

FuzzyRuleConsequent *rule7Consequent = new FuzzyRuleConsequent();
rule7Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule7 = new FuzzyRule(7, rule7Antecedent, rule7Consequent);
fuzzy->addFuzzyRule(fuzzyRule7);

// FuzzyRule 8
FuzzyRuleAntecedent *rule8Antecedent = new FuzzyRuleAntecedent();
rule8Antecedent->joinWithAND(dry, normal);

FuzzyRuleConsequent *rule8Consequent = new FuzzyRuleConsequent();
rule8Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule8 = new FuzzyRule(8, rule8Antecedent, rule8Consequent);
fuzzy->addFuzzyRule(fuzzyRule8);

// FuzzyRule 9
FuzzyRuleAntecedent *rule9Antecedent = new FuzzyRuleAntecedent();
rule9Antecedent->joinWithAND(dry, warm);

FuzzyRuleConsequent *rule9Consequent = new FuzzyRuleConsequent();
rule9Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule9 = new FuzzyRule(9, rule9Antecedent, rule9Consequent);
fuzzy->addFuzzyRule(fuzzyRule9);

// FuzzyRule 10
FuzzyRuleAntecedent *rule10Antecedent = new FuzzyRuleAntecedent();
rule10Antecedent->joinWithAND(dry, hot);

FuzzyRuleConsequent *rule10Consequent = new FuzzyRuleConsequent();
rule10Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule10 = new FuzzyRule(10, rule10Antecedent, rule10Consequent);
fuzzy->addFuzzyRule(fuzzyRule10);

// FuzzyRule 11
FuzzyRuleAntecedent *rule11Antecedent = new FuzzyRuleAntecedent();
rule11Antecedent->joinWithAND(moist, cold);

FuzzyRuleConsequent *rule11Consequent = new FuzzyRuleConsequent();
rule11Consequent->addOutput(off);

FuzzyRule *fuzzyRule11 = new FuzzyRule(11, rule11Antecedent, rule11Consequent);
fuzzy->addFuzzyRule(fuzzyRule11);

// FuzzyRule 12
FuzzyRuleAntecedent *rule12Antecedent = new FuzzyRuleAntecedent();
rule12Antecedent->joinWithAND(moist, cool);

FuzzyRuleConsequent *rule12Consequent = new FuzzyRuleConsequent();
rule12Consequent->addOutput(off);

FuzzyRule *fuzzyRule12 = new FuzzyRule(12, rule12Antecedent, rule12Consequent);
fuzzy->addFuzzyRule(fuzzyRule12);

// FuzzyRule 13
FuzzyRuleAntecedent *rule13Antecedent = new FuzzyRuleAntecedent();
rule13Antecedent->joinWithAND(moist, normal);

FuzzyRuleConsequent *rule13Consequent = new FuzzyRuleConsequent();
rule13Consequent->addOutput(off);

FuzzyRule *fuzzyRule13 = new FuzzyRule(13, rule13Antecedent, rule13Consequent);
fuzzy->addFuzzyRule(fuzzyRule13);

// FuzzyRule 14
FuzzyRuleAntecedent *rule14Antecedent = new FuzzyRuleAntecedent();
rule14Antecedent->joinWithAND(moist, warm);

FuzzyRuleConsequent *rule14Consequent = new FuzzyRuleConsequent();
rule14Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule14 = new FuzzyRule(14, rule14Antecedent, rule14Consequent);
fuzzy->addFuzzyRule(fuzzyRule14);

// FuzzyRule 15
FuzzyRuleAntecedent *rule15Antecedent = new FuzzyRuleAntecedent();
rule15Antecedent->joinWithAND(moist, hot);

FuzzyRuleConsequent *rule15Consequent = new FuzzyRuleConsequent();
rule15Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule15 = new FuzzyRule(15, rule15Antecedent, rule15Consequent);
fuzzy->addFuzzyRule(fuzzyRule15);

// FuzzyRule 16
FuzzyRuleAntecedent *rule16Antecedent = new FuzzyRuleAntecedent();
rule16Antecedent->joinWithAND(wet, cold);

FuzzyRuleConsequent *rule16Consequent = new FuzzyRuleConsequent();
rule16Consequent->addOutput(off);

FuzzyRule *fuzzyRule16 = new FuzzyRule(16, rule16Antecedent, rule16Consequent);
fuzzy->addFuzzyRule(fuzzyRule16);

// FuzzyRule 17
FuzzyRuleAntecedent *rule17Antecedent = new FuzzyRuleAntecedent();
rule17Antecedent->joinWithAND(wet, cool);

FuzzyRuleConsequent *rule17Consequent = new FuzzyRuleConsequent();
rule17Consequent->addOutput(off);

FuzzyRule *fuzzyRule17 = new FuzzyRule(17, rule17Antecedent, rule17Consequent);
fuzzy->addFuzzyRule(fuzzyRule17);

// FuzzyRule 18
FuzzyRuleAntecedent *rule18Antecedent = new FuzzyRuleAntecedent();
rule18Antecedent->joinWithAND(wet, normal);

FuzzyRuleConsequent *rule18Consequent = new FuzzyRuleConsequent();
rule18Consequent->addOutput(off);

FuzzyRule *fuzzyRule18 = new FuzzyRule(18, rule18Antecedent, rule18Consequent);
fuzzy->addFuzzyRule(fuzzyRule18);

// FuzzyRule 19
FuzzyRuleAntecedent *rule19Antecedent = new FuzzyRuleAntecedent();
rule19Antecedent->joinWithAND(wet, warm);

FuzzyRuleConsequent *rule19Consequent = new FuzzyRuleConsequent();
rule19Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule19 = new FuzzyRule(19, rule19Antecedent, rule19Consequent);
fuzzy->addFuzzyRule(fuzzyRule19);

// FuzzyRule 20
FuzzyRuleAntecedent *rule20Antecedent = new FuzzyRuleAntecedent();
rule20Antecedent->joinWithAND(wet, hot);

FuzzyRuleConsequent *rule20Consequent = new FuzzyRuleConsequent();
rule20Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule20 = new FuzzyRule(20, rule20Antecedent, rule20Consequent);
fuzzy->addFuzzyRule(fuzzyRule20);

// FuzzyRule 21
FuzzyRuleAntecedent *rule21Antecedent = new FuzzyRuleAntecedent();
rule21Antecedent->joinWithAND(veryWet, cold);

FuzzyRuleConsequent *rule21Consequent = new FuzzyRuleConsequent();
rule21Consequent->addOutput(off);

FuzzyRule *fuzzyRule21 = new FuzzyRule(21, rule21Antecedent, rule21Consequent);
fuzzy->addFuzzyRule(fuzzyRule21);

// FuzzyRule 22
FuzzyRuleAntecedent *rule22Antecedent = new FuzzyRuleAntecedent();
rule22Antecedent->joinWithAND(veryWet, cool);

FuzzyRuleConsequent *rule22Consequent = new FuzzyRuleConsequent();
rule22Consequent->addOutput(off);

FuzzyRule *fuzzyRule22 = new FuzzyRule(22, rule22Antecedent, rule22Consequent);
fuzzy->addFuzzyRule(fuzzyRule22);

// FuzzyRule 23
FuzzyRuleAntecedent *rule23Antecedent = new FuzzyRuleAntecedent();
rule23Antecedent->joinWithAND(veryWet, normal);

FuzzyRuleConsequent *rule23Consequent = new FuzzyRuleConsequent();
rule23Consequent->addOutput(off);

FuzzyRule *fuzzyRule23 = new FuzzyRule(23, rule23Antecedent, rule23Consequent);
fuzzy->addFuzzyRule(fuzzyRule23);

// FuzzyRule 24
FuzzyRuleAntecedent *rule24Antecedent = new FuzzyRuleAntecedent();
rule24Antecedent->joinWithAND(veryWet, warm);

FuzzyRuleConsequent *rule24Consequent = new FuzzyRuleConsequent();
rule24Consequent->addOutput(off);

FuzzyRule *fuzzyRule24 = new FuzzyRule(24, rule24Antecedent, rule24Consequent);
fuzzy->addFuzzyRule(fuzzyRule24);

// FuzzyRule 25
FuzzyRuleAntecedent *rule25Antecedent = new FuzzyRuleAntecedent();
rule25Antecedent->joinWithAND(veryWet, hot);

FuzzyRuleConsequent *rule25Consequent = new FuzzyRuleConsequent();
rule25Consequent->addOutput(off);

FuzzyRule *fuzzyRule25 = new FuzzyRule(25, rule25Antecedent, rule25Consequent);
fuzzy->addFuzzyRule(fuzzyRule25);
}