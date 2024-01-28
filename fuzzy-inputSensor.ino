#include <Fuzzy.h>
#include <DHT.h>

//----------- Deklarasi Pin -----------
// Pin untuk sensor DHT
const int pinDHT = D3;  
DHT dht(pinDHT, DHT22);
// Pin untuk sensor kelembaban tanah 
const int pinSoilMoisture = A0; 
// Variabel Global untuk kelembaban
static int previousSoilPercent = 0;
// Pin untuk mengontrol relay
const int pinRelay = D5;  
//Variabel untuk menyimpan status pompa
static bool pumpActive = false; 


//------------ set Nilai Fuzzy ------------
Fuzzy *fuzzy = new Fuzzy();

// Fungsi keanggotaan Suhu Input
FuzzySet *cold = new FuzzySet(-10,-10, -10, 3);
FuzzySet *cool = new FuzzySet(0, 3, 12, 15);
FuzzySet *normal = new FuzzySet(12, 15, 24, 27);
FuzzySet *warm = new FuzzySet(24, 27, 36, 39);
FuzzySet *hot = new FuzzySet(36, 50, 50,50);

// Fungsi keanggotaan Kelembaban Input
FuzzySet *dry = new FuzzySet(0,0,0, 20);
FuzzySet *moist = new FuzzySet(10, 20, 40, 50);
FuzzySet *wet = new FuzzySet(40, 70,70,70);

// Fungsi keanggotaan Lama Menyiram Output
FuzzySet *off = new FuzzySet(0,0,0, 20);
FuzzySet *cepat = new FuzzySet(0, 20,20, 28);
FuzzySet *biasa = new FuzzySet(20, 28, 40, 48);
FuzzySet *lama = new FuzzySet(40, 60,60,60);

void setup() {
  Serial.begin(9600);
  // randomSeed(analogRead(0));
  pinMode(pinRelay, OUTPUT);
  dht.begin();
  fuzzySetup();
}


void loop() {
// //----------- Membaca nilai dari Random -----------
//   int temperature = random(0, 60);
//   int humidity = random(0, 100);
//   int soilPercent = random(0, 100);

//----------- Membaca nilai dari Sensor -----------
    // Membaca nilai suhu dan kelembaban dari sensor DHT
  float temperature = dht.readTemperature();
  // float humidity = dht.readHumidity();
  // Membaca nilai kelembaban tanah dari sensor kelembaban tanah 
  float soilMoisture = map(analogRead(pinSoilMoisture), 0, 1300, 100, 0); //konversi nilai 
  int soilPercent = constrain(soilMoisture, 100, 0); //mengubah jadi nilai integer

//----------- Setingan input dan output fuzzy -----------
  fuzzy->setInput(1, temperature);
  // fuzzy->setInput(2, humidity);
  fuzzy->setInput(2, soilPercent);
  fuzzy->fuzzify();
  float output = fuzzy->defuzzify(1);

//----------- Menampilkan hasil di Serial Monitor ------------
  Serial.print("Temperature: ");
  Serial.print(temperature);
  // Serial.print(" | Humidity: ");
  // Serial.print(humidity);
  Serial.print(" | Soil Percent: ");
  Serial.print(soilPercent);
  Serial.print(" => Output: ");
  Serial.println(output);


  // Mengecek kondisi untuk mengatur pompa
  if (soilPercent < 50) {
    // Kelembaban masih rendah, nyalakan pompa dan perbarui status
    pompa(output);  // mengaktifkan fuzzy untuk mengatur durasi pompa
    pumpActive = true;
    previousSoilPercent = soilPercent;
    Serial.println(" | pompa active");
  } else if (pumpActive && soilPercent >= previousSoilPercent) {
    // Jika pompa aktif dan kelembaban meningkat atau tetap sama, matikan pompa
    digitalWrite(pinRelay, LOW);
    pumpActive = false;
    Serial.println(" | Pump inactive");
    
  } else {
    digitalWrite(pinRelay, LOW);
    Serial.println(" | Pump inactive");
  }
  delay(1000);
}
//----------- Fungsi Mengatur Durasi Pompa -----------
void pompa(float durasi) {
  float min = 5.0;
  float max = 60.0;
  int waktu = map(durasi, 0, 60, min, max);

  digitalWrite(pinRelay, HIGH);
  Serial.print(" | Pump Duration: ");
  Serial.println(waktu);

  delay(waktu * 1000);

  digitalWrite(pinRelay, LOW);
  previousSoilPercent = analogRead(pinSoilMoisture);
  Serial.print(" | Soil Percent: ");
  Serial.print(previousSoilPercent);
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

kelembaban->addFuzzySet(dry);
kelembaban->addFuzzySet(moist);
kelembaban->addFuzzySet(wet);
fuzzy->addFuzzyInput(kelembaban);

// FuzzyOutput RELAY (Lama Menyiram)
FuzzyOutput *durasi = new FuzzyOutput(1);

durasi->addFuzzySet(off);
durasi->addFuzzySet(cepat);
durasi->addFuzzySet(biasa);
durasi->addFuzzySet(lama);
fuzzy->addFuzzyOutput(durasi);
  
//----------------- FuzzyRule ---------------- 1
  // FuzzyRule 1
FuzzyRuleAntecedent *rule1Antecedent = new FuzzyRuleAntecedent();
rule1Antecedent->joinWithAND(dry, cold);

FuzzyRuleConsequent *rule1Consequent = new FuzzyRuleConsequent();
rule1Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule1 = new FuzzyRule(1, rule1Antecedent, rule1Consequent);
fuzzy->addFuzzyRule(fuzzyRule1);

// FuzzyRule 2
FuzzyRuleAntecedent *rule2Antecedent = new FuzzyRuleAntecedent();
rule2Antecedent->joinWithAND(dry, cool);

FuzzyRuleConsequent *rule2Consequent = new FuzzyRuleConsequent();
rule2Consequent->addOutput(lama);

FuzzyRule *fuzzyRule2 = new FuzzyRule(2, rule2Antecedent, rule2Consequent);
fuzzy->addFuzzyRule(fuzzyRule2);

// FuzzyRule 3
FuzzyRuleAntecedent *rule3Antecedent = new FuzzyRuleAntecedent();
rule3Antecedent->joinWithAND(dry, normal);

FuzzyRuleConsequent *rule3Consequent = new FuzzyRuleConsequent();
rule3Consequent->addOutput(lama);

FuzzyRule *fuzzyRule3 = new FuzzyRule(3, rule3Antecedent, rule3Consequent);
fuzzy->addFuzzyRule(fuzzyRule3);

// FuzzyRule 4
FuzzyRuleAntecedent *rule4Antecedent = new FuzzyRuleAntecedent();
rule4Antecedent->joinWithAND(dry, warm);

FuzzyRuleConsequent *rule4Consequent = new FuzzyRuleConsequent();
rule4Consequent->addOutput(lama);

FuzzyRule *fuzzyRule4 = new FuzzyRule(4, rule4Antecedent, rule4Consequent);
fuzzy->addFuzzyRule(fuzzyRule4);

// FuzzyRule 5
FuzzyRuleAntecedent *rule5Antecedent = new FuzzyRuleAntecedent();
rule5Antecedent->joinWithAND(dry, hot);

FuzzyRuleConsequent *rule5Consequent = new FuzzyRuleConsequent();
rule5Consequent->addOutput(lama);

FuzzyRule *fuzzyRule5 = new FuzzyRule(5, rule5Antecedent, rule5Consequent);
fuzzy->addFuzzyRule(fuzzyRule5);

// FuzzyRule 6
FuzzyRuleAntecedent *rule6Antecedent = new FuzzyRuleAntecedent();
rule6Antecedent->joinWithAND(moist, cold);

FuzzyRuleConsequent *rule6Consequent = new FuzzyRuleConsequent();
rule6Consequent->addOutput(off);

FuzzyRule *fuzzyRule6 = new FuzzyRule(6, rule6Antecedent, rule6Consequent);
fuzzy->addFuzzyRule(fuzzyRule6);

// FuzzyRule 7
FuzzyRuleAntecedent *rule7Antecedent = new FuzzyRuleAntecedent();
rule7Antecedent->joinWithAND(moist, cool);

FuzzyRuleConsequent *rule7Consequent = new FuzzyRuleConsequent();
rule7Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule7 = new FuzzyRule(7, rule7Antecedent, rule7Consequent);
fuzzy->addFuzzyRule(fuzzyRule7);

// FuzzyRule 8
FuzzyRuleAntecedent *rule8Antecedent = new FuzzyRuleAntecedent();
rule8Antecedent->joinWithAND(moist, normal);

FuzzyRuleConsequent *rule8Consequent = new FuzzyRuleConsequent();
rule8Consequent->addOutput(cepat);

FuzzyRule *fuzzyRule8 = new FuzzyRule(8, rule8Antecedent, rule8Consequent);
fuzzy->addFuzzyRule(fuzzyRule8);

// FuzzyRule 9
FuzzyRuleAntecedent *rule9Antecedent = new FuzzyRuleAntecedent();
rule9Antecedent->joinWithAND(moist, warm);

FuzzyRuleConsequent *rule9Consequent = new FuzzyRuleConsequent();
rule9Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule9 = new FuzzyRule(9, rule9Antecedent, rule9Consequent);
fuzzy->addFuzzyRule(fuzzyRule9);

// FuzzyRule 10
FuzzyRuleAntecedent *rule10Antecedent = new FuzzyRuleAntecedent();
rule10Antecedent->joinWithAND(moist, hot);

FuzzyRuleConsequent *rule10Consequent = new FuzzyRuleConsequent();
rule10Consequent->addOutput(biasa);

FuzzyRule *fuzzyRule10 = new FuzzyRule(10, rule10Antecedent, rule10Consequent);
fuzzy->addFuzzyRule(fuzzyRule10);

// FuzzyRule 11
FuzzyRuleAntecedent *rule11Antecedent = new FuzzyRuleAntecedent();
rule11Antecedent->joinWithAND(wet, cold);

FuzzyRuleConsequent *rule11Consequent = new FuzzyRuleConsequent();
rule11Consequent->addOutput(off);

FuzzyRule *fuzzyRule11 = new FuzzyRule(11, rule11Antecedent, rule11Consequent);
fuzzy->addFuzzyRule(fuzzyRule11);

// FuzzyRule 12
FuzzyRuleAntecedent *rule12Antecedent = new FuzzyRuleAntecedent();
rule12Antecedent->joinWithAND(wet, cool);

FuzzyRuleConsequent *rule12Consequent = new FuzzyRuleConsequent();
rule12Consequent->addOutput(off);

FuzzyRule *fuzzyRule12 = new FuzzyRule(12, rule12Antecedent, rule12Consequent);
fuzzy->addFuzzyRule(fuzzyRule12);

// FuzzyRule 13
FuzzyRuleAntecedent *rule13Antecedent = new FuzzyRuleAntecedent();
rule13Antecedent->joinWithAND(wet, normal);

FuzzyRuleConsequent *rule13Consequent = new FuzzyRuleConsequent();
rule13Consequent->addOutput(off);

FuzzyRule *fuzzyRule13 = new FuzzyRule(13, rule13Antecedent, rule13Consequent);
fuzzy->addFuzzyRule(fuzzyRule13);

// FuzzyRule 14
FuzzyRuleAntecedent *rule14Antecedent = new FuzzyRuleAntecedent();
rule14Antecedent->joinWithAND(wet, warm);

FuzzyRuleConsequent *rule14Consequent = new FuzzyRuleConsequent();
rule14Consequent->addOutput(off);

FuzzyRule *fuzzyRule14 = new FuzzyRule(14, rule14Antecedent, rule14Consequent);
fuzzy->addFuzzyRule(fuzzyRule14);

// FuzzyRule 15
FuzzyRuleAntecedent *rule15Antecedent = new FuzzyRuleAntecedent();
rule15Antecedent->joinWithAND(wet, hot);

FuzzyRuleConsequent *rule15Consequent = new FuzzyRuleConsequent();
rule15Consequent->addOutput(off);

FuzzyRule *fuzzyRule15 = new FuzzyRule(15, rule15Antecedent, rule15Consequent);
fuzzy->addFuzzyRule(fuzzyRule15);
}