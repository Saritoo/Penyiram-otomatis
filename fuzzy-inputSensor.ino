#include <Fuzzy.h>
#include <DHT.h>

//----------- Deklarasi Pin -----------
// Pin untuk sensor DHT
const int pinDHT = D3;  
DHT dht(pinDHT, DHT22);
// Pin untuk sensor kelembaban tanah 
const int pinSoilMoisture = A0;  
// Pin untuk mengontrol relay
const int pinRelay = D5;  

//------------ set Nilai Fuzzy ------------
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput Suhu
FuzzySet *dingin = new FuzzySet(0, 0, 15, 25);
FuzzySet *sedang = new FuzzySet(20, 25, 25, 30);
FuzzySet *panas = new FuzzySet(25, 35, 60, 60);

// FuzzyInput Kelembaban tanah
FuzzySet *kering = new FuzzySet(0, 0, 40, 60);
FuzzySet *lembab = new FuzzySet(50, 65, 65, 80);
FuzzySet *basah = new FuzzySet(70, 90, 100, 100);

// FuzzyInput Kelembaban Udara
FuzzySet *rendah = new FuzzySet(0, 0, 40, 60);
FuzzySet *biasa = new FuzzySet(50, 60, 60, 70);
FuzzySet *tinggi = new FuzzySet(60, 80, 100, 100);

// FuzzyOutput
FuzzySet *sc = new FuzzySet(0, 0, 10, 20);   // Sangat Cepat
FuzzySet *c = new FuzzySet(10, 20, 20, 30);   // Cepat
FuzzySet *normal = new FuzzySet(20, 30, 30, 40);  // Normal
FuzzySet *l = new FuzzySet(30, 40, 40, 50);   // Lama
FuzzySet *sl = new FuzzySet(40, 50, 60, 60);  // Sangat Lama

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
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
  float humidity = dht.readHumidity();
  // Membaca nilai kelembaban tanah dari sensor kelembaban tanah 
  float soilMoisture = map(analogRead(pinSoilMoisture), 0, 1023, 0, 100); //konversi nilai 
  int soilPercent = constrain(soilMoisture, 0, 100); //mengubah jadi nilai integer

//----------- Setingan input dan output fuzzy -----------
  fuzzy->setInput(1, temperature);
  fuzzy->setInput(2, humidity);
  fuzzy->setInput(3, soilPercent);
  fuzzy->fuzzify();
  float output = fuzzy->defuzzify(1);

//----------- Menampilkan hasil di Serial Monitor ------------
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" | Humidity: ");
  Serial.print(humidity);
  Serial.print(" | Soil Percent: ");
  Serial.print(soilPercent);
  Serial.print(" => Output: ");
  Serial.println(output);
  pompa(output);
  delay(1000);  
}
//----------- Fungsi Mengatur Durasi Pompa -----------
void pompa(float durasi) {
  // Mengatur waktu minimal dan maksimal pompa (dalam detik)
  float min = 5.0;
  float max = 60.0;

  // Mengonversi durasi fuzzy 
  int waktu = map(durasi, 0, 60, min, max);

  // Menyalakan pompa
  digitalWrite(pinRelay, HIGH);
  Serial.print(" | Pump Duration: ");
  Serial.println(waktu);

  // Menunggu selama durasi pompa
  delay(waktu * 1000);

  // Mematikan pompa
  digitalWrite(pinRelay, LOW);
}

void fuzzySetup () {
  // FuzzyInput suhu
  FuzzyInput *suhu = new FuzzyInput(1);

  suhu->addFuzzySet(dingin);
  suhu->addFuzzySet(sedang);
  suhu->addFuzzySet(panas);
  fuzzy->addFuzzyInput(suhu);

  // FuzzyInput lembTanah
  FuzzyInput *lembTanah = new FuzzyInput(2);

  lembTanah->addFuzzySet(kering);
  lembTanah->addFuzzySet(lembab);
  lembTanah->addFuzzySet(basah);
  fuzzy->addFuzzyInput(lembTanah);

  // FuzzyInput lembUdara
  FuzzyInput *lembUdara = new FuzzyInput(3);

  lembUdara->addFuzzySet(rendah);
  lembUdara->addFuzzySet(biasa);
  lembUdara->addFuzzySet(tinggi);
  fuzzy->addFuzzyInput(lembUdara);

  // FuzzyOutput RELAY (durasi)
  FuzzyOutput *durasi = new FuzzyOutput(1);

  durasi->addFuzzySet(sc);
  durasi->addFuzzySet(c);
  durasi->addFuzzySet(normal);
  durasi->addFuzzySet(l);
  durasi->addFuzzySet(sl);
  fuzzy->addFuzzyOutput(durasi);
  
//----------------- FuzzyRule ---------------- 1
  FuzzyRuleAntecedent *dingin_kering_1 = new FuzzyRuleAntecedent();
  dingin_kering_1->joinWithAND(dingin, kering);

  FuzzyRuleAntecedent *c_rendah = new FuzzyRuleAntecedent();
  c_rendah->joinSingle(rendah);

  FuzzyRuleAntecedent *dingin_kering_rendah_1 = new FuzzyRuleAntecedent();
  dingin_kering_rendah_1->joinWithAND(dingin_kering_1, c_rendah);

  FuzzyRuleConsequent *c_1 = new FuzzyRuleConsequent();
  c_1->addOutput(c);
//  c_t_1->addOutput(t);

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, dingin_kering_rendah_1, c_1);
  fuzzy->addFuzzyRule(fuzzyRule1);

//----------------- FuzzyRule ---------------- 2
  FuzzyRuleAntecedent *dingin_kering_2 = new FuzzyRuleAntecedent();
  dingin_kering_2->joinWithAND(dingin, kering);

  FuzzyRuleAntecedent *c_biasa = new FuzzyRuleAntecedent();
  c_biasa->joinSingle(biasa);

  FuzzyRuleAntecedent *dingin_kering_biasa_2 = new FuzzyRuleAntecedent();
  dingin_kering_biasa_2->joinWithAND(dingin_kering_2, c_biasa);

  FuzzyRuleConsequent *c_2 = new FuzzyRuleConsequent();
  c_2->addOutput(c);
  //c_2->addOutput();

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, dingin_kering_biasa_2, c_2);
  fuzzy->addFuzzyRule(fuzzyRule2);

//----------------- FuzzyRule ---------------- 3
  FuzzyRuleAntecedent *dingin_kering_3 = new FuzzyRuleAntecedent();
  dingin_kering_3->joinWithAND(dingin, kering);

  FuzzyRuleAntecedent *c_tinggi = new FuzzyRuleAntecedent();
  c_tinggi->joinSingle(tinggi);

  FuzzyRuleAntecedent *dingin_kering_tinggi_3 = new FuzzyRuleAntecedent();
  dingin_kering_tinggi_3->joinWithAND(dingin_kering_3, c_tinggi);

  FuzzyRuleConsequent *c_3 = new FuzzyRuleConsequent();
  c_3->addOutput(c);
 // c_3->addOutput(r);

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, dingin_kering_tinggi_3, c_3);
  fuzzy->addFuzzyRule(fuzzyRule3);

//----------------- FuzzyRule ---------------- 4
  FuzzyRuleAntecedent *dingin_lembab_4 = new FuzzyRuleAntecedent();
  dingin_lembab_4->joinWithAND(dingin, lembab);

  FuzzyRuleAntecedent *dingin_lembab_rendah_4 = new FuzzyRuleAntecedent();
  dingin_lembab_rendah_4->joinWithAND(dingin_lembab_4, c_rendah);

  FuzzyRuleConsequent *sc_4 = new FuzzyRuleConsequent();
  sc_4->addOutput(sc);
  //sc_4->addOutput(t);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, dingin_lembab_rendah_4, sc_4);
  fuzzy->addFuzzyRule(fuzzyRule4);

//----------------- FuzzyRule ---------------- 5
  FuzzyRuleAntecedent *dingin_lembab_5 = new FuzzyRuleAntecedent();
  dingin_lembab_5->joinWithAND(dingin, lembab);

  FuzzyRuleAntecedent *dingin_lembab_biasa_5 = new FuzzyRuleAntecedent();
  dingin_lembab_biasa_5->joinWithAND(dingin_lembab_5, c_biasa);

  FuzzyRuleConsequent *sc_5 = new FuzzyRuleConsequent();
  sc_5->addOutput(sc);
  //sc_5->addOutput();

  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, dingin_lembab_biasa_5, sc_5);
  fuzzy->addFuzzyRule(fuzzyRule5);

//----------------- FuzzyRule ---------------- 6
  FuzzyRuleAntecedent *dingin_lembab_6 = new FuzzyRuleAntecedent();
  dingin_lembab_6->joinWithAND(dingin, lembab);

  FuzzyRuleAntecedent *dingin_lembab_tinggi_6 = new FuzzyRuleAntecedent();
  dingin_lembab_tinggi_6->joinWithAND(dingin_lembab_6, c_tinggi);

  FuzzyRuleConsequent *sc_6 = new FuzzyRuleConsequent();
  sc_6->addOutput(sc);
  //sc_r_6->addOutput(r);

  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, dingin_lembab_tinggi_6, sc_6);
  fuzzy->addFuzzyRule(fuzzyRule6);

//----------------- FuzzyRule ---------------- 7
  FuzzyRuleAntecedent *dingin_basah_7 = new FuzzyRuleAntecedent();
  dingin_basah_7->joinWithAND(dingin, basah);

  FuzzyRuleAntecedent *dingin_basah_rendah_7 = new FuzzyRuleAntecedent();
  dingin_basah_rendah_7->joinWithAND(dingin_basah_7, c_rendah);

  FuzzyRuleConsequent *sc_7 = new FuzzyRuleConsequent();
  sc_7->addOutput(sc);
  //sc_7->addOutput();

  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, dingin_basah_rendah_7, sc_7);
  fuzzy->addFuzzyRule(fuzzyRule7);

//----------------- FuzzyRule ---------------- 8
  FuzzyRuleAntecedent *dingin_basah_8 = new FuzzyRuleAntecedent();
  dingin_basah_8->joinWithAND(dingin, basah);

  FuzzyRuleAntecedent *dingin_basah_biasa_8 = new FuzzyRuleAntecedent();
  dingin_basah_biasa_8->joinWithAND(dingin_basah_8, c_biasa);

  FuzzyRuleConsequent *sc_8 = new FuzzyRuleConsequent();
  sc_8->addOutput(sc);
  //sc_t_8->addOutput(t);

  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, dingin_basah_biasa_8, sc_8);
  fuzzy->addFuzzyRule(fuzzyRule8);

//----------------- FuzzyRule ---------------- 9
  FuzzyRuleAntecedent *dingin_basah_9 = new FuzzyRuleAntecedent();
  dingin_basah_9->joinWithAND(dingin, basah);

  FuzzyRuleAntecedent *dingin_basah_tinggi_9 = new FuzzyRuleAntecedent();
  dingin_basah_tinggi_9->joinWithAND(dingin_basah_9, c_tinggi);

  FuzzyRuleConsequent *sc_9 = new FuzzyRuleConsequent();
  sc_9->addOutput(sc);
  //sc_9->addOutput();

  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, dingin_basah_tinggi_9, sc_9);
  fuzzy->addFuzzyRule(fuzzyRule9);

//----------------- FuzzyRule ---------------- 10
  FuzzyRuleAntecedent *sedang_kering_10 = new FuzzyRuleAntecedent();
  sedang_kering_10->joinWithAND(sedang, kering);

  FuzzyRuleAntecedent *sedang_kering_rendah_10 = new FuzzyRuleAntecedent();
  sedang_kering_rendah_10->joinWithAND(sedang_kering_10, c_rendah);

  FuzzyRuleConsequent *normal_10 = new FuzzyRuleConsequent();
  normal_10->addOutput(normal);
//  normal_10->addOutput();

  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, sedang_kering_rendah_10, normal_10);
  fuzzy->addFuzzyRule(fuzzyRule10);

//----------------- FuzzyRule ---------------- 11
  FuzzyRuleAntecedent *sedang_kering_11 = new FuzzyRuleAntecedent();
  sedang_kering_11->joinWithAND(sedang, kering);

  FuzzyRuleAntecedent *sedang_kering_biasa_11 = new FuzzyRuleAntecedent();
  sedang_kering_biasa_11->joinWithAND(sedang_kering_11, c_biasa);

  FuzzyRuleConsequent *normal_11 = new FuzzyRuleConsequent();
  normal_11->addOutput(normal);
 // normal_11->addOutput(r);

  FuzzyRule *fuzzyRule11 = new FuzzyRule(11, sedang_kering_biasa_11, normal_11);
  fuzzy->addFuzzyRule(fuzzyRule11);

//----------------- FuzzyRule ---------------- 12
  FuzzyRuleAntecedent *sedang_kering_12 = new FuzzyRuleAntecedent();
  sedang_kering_12->joinWithAND(sedang, kering);

  FuzzyRuleAntecedent *sedang_kering_tinggi_12 = new FuzzyRuleAntecedent();
  sedang_kering_tinggi_12->joinWithAND(sedang_kering_12, c_tinggi);

  FuzzyRuleConsequent *normal_12 = new FuzzyRuleConsequent();
  normal_12->addOutput(normal);
//  normal_12->addOutput();

  FuzzyRule *fuzzyRule12 = new FuzzyRule(12, sedang_kering_tinggi_12, normal_12);
  fuzzy->addFuzzyRule(fuzzyRule12);

//----------------- FuzzyRule ---------------- 13
  FuzzyRuleAntecedent *sedang_lembab_13 = new FuzzyRuleAntecedent();
  sedang_lembab_13->joinWithAND(sedang, lembab);
  
  FuzzyRuleAntecedent *sedang_lembab_rendah_13 = new FuzzyRuleAntecedent();
  sedang_lembab_rendah_13->joinWithAND(sedang_lembab_13, c_rendah);

  FuzzyRuleConsequent *sc_13 = new FuzzyRuleConsequent();
  sc_13->addOutput(sc);
//  sc_13->addOutput();

  FuzzyRule *fuzzyRule13 = new FuzzyRule(13, sedang_lembab_rendah_13, sc_13);
  fuzzy->addFuzzyRule(fuzzyRule13);

//----------------- FuzzyRule ---------------- 14
  FuzzyRuleAntecedent *sedang_lembab_14 = new FuzzyRuleAntecedent();
  sedang_lembab_14->joinWithAND(sedang, lembab);

  FuzzyRuleAntecedent *sedang_lembab_biasa_14 = new FuzzyRuleAntecedent();
  sedang_lembab_biasa_14->joinWithAND(sedang_lembab_14, c_biasa );

  FuzzyRuleConsequent *sc_14 = new FuzzyRuleConsequent();
  sc_14->addOutput(sc);
//  sc_14->addOutput();

  FuzzyRule *fuzzyRule14 = new FuzzyRule(14, sedang_lembab_biasa_14, sc_14);
  fuzzy->addFuzzyRule(fuzzyRule14);

//----------------- FuzzyRule ---------------- 15
  FuzzyRuleAntecedent *sedang_lembab_15 = new FuzzyRuleAntecedent();
  sedang_lembab_15->joinWithAND(sedang, lembab);

  FuzzyRuleAntecedent *sedang_lembab_tinggi_15 = new FuzzyRuleAntecedent();
  sedang_lembab_tinggi_15->joinWithAND(sedang_lembab_15, c_tinggi);

  FuzzyRuleConsequent *sc_15 = new FuzzyRuleConsequent();
  sc_15->addOutput(sc);
//  sc_15->addOutput();

  FuzzyRule *fuzzyRule15 = new FuzzyRule(15, sedang_lembab_tinggi_15, sc_15);
  fuzzy->addFuzzyRule(fuzzyRule15);

//----------------- FuzzyRule ---------------- 16
  FuzzyRuleAntecedent *sedang_basah_16 = new FuzzyRuleAntecedent();
  sedang_basah_16->joinWithAND(sedang, basah);

  FuzzyRuleAntecedent *sedang_basah_rendah_16 = new FuzzyRuleAntecedent();
  sedang_basah_rendah_16->joinWithAND(sedang_basah_16, c_rendah);

  FuzzyRuleConsequent *sc_16 = new FuzzyRuleConsequent();
  sc_16->addOutput(sc);
//  sc_16->addOutput();

  FuzzyRule *fuzzyRule16 = new FuzzyRule(16, sedang_basah_rendah_16, sc_16);
  fuzzy->addFuzzyRule(fuzzyRule16);

//----------------- FuzzyRule ---------------- 17
  FuzzyRuleAntecedent *sedang_basah_17 = new FuzzyRuleAntecedent();
  sedang_basah_17->joinWithAND(sedang, basah);

  FuzzyRuleAntecedent *sedang_basah_biasa_17 = new FuzzyRuleAntecedent();
  sedang_basah_biasa_17->joinWithAND(sedang_basah_17, c_biasa);

  FuzzyRuleConsequent *sc_17 = new FuzzyRuleConsequent();
  sc_17->addOutput(sc);
//  sc_r_17->addOutput(r);

  FuzzyRule *fuzzyRule17 = new FuzzyRule(17, sedang_basah_biasa_17, sc_17);
  fuzzy->addFuzzyRule(fuzzyRule17);

//----------------- FuzzyRule ---------------- 18
  FuzzyRuleAntecedent *sedang_basah_18 = new FuzzyRuleAntecedent();
  sedang_basah_18->joinWithAND(sedang, basah);

  FuzzyRuleAntecedent *sedang_basah_tinggi_18 = new FuzzyRuleAntecedent();
  sedang_basah_tinggi_18->joinWithAND(sedang_basah_18, c_tinggi);

  FuzzyRuleConsequent *sc_18 = new FuzzyRuleConsequent();
  sc_18->addOutput(sc);
//  sc_18->addOutput();

  FuzzyRule *fuzzyRule18 = new FuzzyRule(18, sedang_basah_tinggi_18, sc_18);
  fuzzy->addFuzzyRule(fuzzyRule18);

//----------------- FuzzyRule ---------------- 19
  FuzzyRuleAntecedent *panas_kering_19 = new FuzzyRuleAntecedent();
  panas_kering_19->joinWithAND(panas, kering);

  FuzzyRuleAntecedent *panas_kering_rendah_19 = new FuzzyRuleAntecedent();
  panas_kering_rendah_19->joinWithAND(panas_kering_19, c_rendah);

  FuzzyRuleConsequent *s_r_19 = new FuzzyRuleConsequent();
  s_r_19->addOutput(sl);
//  s_r_19->addOutput(r);

  FuzzyRule *fuzzyRule19 = new FuzzyRule(19, panas_kering_rendah_19, s_r_19);
  fuzzy->addFuzzyRule(fuzzyRule19);

//----------------- FuzzyRule ---------------- 20
  FuzzyRuleAntecedent *panas_kering_20 = new FuzzyRuleAntecedent();
  panas_kering_20->joinWithAND(panas, kering);

  FuzzyRuleAntecedent *panas_kering_biasa_20 = new FuzzyRuleAntecedent();
  panas_kering_biasa_20->joinWithAND(panas_kering_20, c_biasa);

  FuzzyRuleConsequent *s_20 = new FuzzyRuleConsequent();
  s_20->addOutput(sl);
//  s_20->addOutput();

  FuzzyRule *fuzzyRule20 = new FuzzyRule(20, panas_kering_biasa_20, s_20);
  fuzzy->addFuzzyRule(fuzzyRule20);

//----------------- FuzzyRule ---------------- 21
  FuzzyRuleAntecedent *panas_kering_21 = new FuzzyRuleAntecedent();
  panas_kering_21->joinWithAND(panas, kering);

  FuzzyRuleAntecedent *panas_kering_tinggi_21 = new FuzzyRuleAntecedent();
  panas_kering_tinggi_21->joinWithAND(panas_kering_21, c_tinggi);

  FuzzyRuleConsequent *s_21 = new FuzzyRuleConsequent();
  s_21->addOutput(sl);
//  s_21->addOutput();

  FuzzyRule *fuzzyRule21 = new FuzzyRule(21, panas_kering_tinggi_21, s_21);
  fuzzy->addFuzzyRule(fuzzyRule21);

//----------------- FuzzyRule ---------------- 22
  FuzzyRuleAntecedent *panas_lembab_22 = new FuzzyRuleAntecedent();
  panas_lembab_22->joinWithAND(panas, lembab);

  FuzzyRuleAntecedent *panas_lembab_rendah_22 = new FuzzyRuleAntecedent();
  panas_lembab_rendah_22->joinWithAND(panas_lembab_22, c_rendah);

  FuzzyRuleConsequent *l_22 = new FuzzyRuleConsequent();
  l_22->addOutput(l);
//  _t_22->addOutput(t);

  FuzzyRule *fuzzyRule22 = new FuzzyRule(22, panas_lembab_rendah_22, l_22);
  fuzzy->addFuzzyRule(fuzzyRule22);

//----------------- FuzzyRule ---------------- 23
  FuzzyRuleAntecedent *panas_lembab_23 = new FuzzyRuleAntecedent();
  panas_lembab_23->joinWithAND(panas, lembab);

  FuzzyRuleAntecedent *panas_lembab_biasa_23 = new FuzzyRuleAntecedent();
  panas_lembab_biasa_23->joinWithAND(panas_lembab_23, c_biasa);

  FuzzyRuleConsequent *l_23 = new FuzzyRuleConsequent();
  l_23->addOutput(l);
//  _r_23->addOutput(r);

  FuzzyRule *fuzzyRule23 = new FuzzyRule(23, panas_lembab_biasa_23, l_23);
  fuzzy->addFuzzyRule(fuzzyRule23);

//----------------- FuzzyRule ---------------- 24
  FuzzyRuleAntecedent *panas_lembab_24 = new FuzzyRuleAntecedent();
  panas_lembab_24->joinWithAND(panas, lembab);

  FuzzyRuleAntecedent *panas_lembab_tinggi_24 = new FuzzyRuleAntecedent();
  panas_lembab_tinggi_24->joinWithAND(panas_lembab_24, c_tinggi);

  FuzzyRuleConsequent *_24 = new FuzzyRuleConsequent();
  _24->addOutput(l);
//  _24->addOutput();

  FuzzyRule *fuzzyRule24 = new FuzzyRule(24, panas_lembab_tinggi_24, _24);
  fuzzy->addFuzzyRule(fuzzyRule24);

//----------------- FuzzyRule ---------------- 25
  FuzzyRuleAntecedent *panas_basah_25 = new FuzzyRuleAntecedent();
  panas_basah_25->joinWithAND(panas, basah);

  FuzzyRuleAntecedent *panas_basah_rendah_25 = new FuzzyRuleAntecedent();
  panas_basah_rendah_25->joinWithAND(panas_basah_25, c_rendah);

  FuzzyRuleConsequent *normal_25 = new FuzzyRuleConsequent();
  normal_25->addOutput(normal);
//  normal_25->addOutput();

  FuzzyRule *fuzzyRule25 = new FuzzyRule(25, panas_basah_rendah_25, normal_25);
  fuzzy->addFuzzyRule(fuzzyRule25);

//----------------- FuzzyRule ---------------- 26
  FuzzyRuleAntecedent *panas_basah_26 = new FuzzyRuleAntecedent();
  panas_basah_26->joinWithAND(panas, basah);

  FuzzyRuleAntecedent *panas_basah_biasa_26 = new FuzzyRuleAntecedent();
  panas_basah_biasa_26->joinWithAND(panas_basah_26, c_biasa);

  FuzzyRuleConsequent *normal_26 = new FuzzyRuleConsequent();
  normal_26->addOutput(normal);
//  normal_t_26->addOutput(t);

  FuzzyRule *fuzzyRule26 = new FuzzyRule(26, panas_basah_biasa_26, normal_26);
  fuzzy->addFuzzyRule(fuzzyRule26);

//----------------- FuzzyRule ---------------- 27
  FuzzyRuleAntecedent *panas_basah_27 = new FuzzyRuleAntecedent();
  panas_basah_27->joinWithAND(panas, basah);

  FuzzyRuleAntecedent *panas_basah_tinggi_27 = new FuzzyRuleAntecedent();
  panas_basah_tinggi_27->joinWithAND(panas_basah_27, c_tinggi);

  FuzzyRuleConsequent *normal_27 = new FuzzyRuleConsequent();
  normal_27->addOutput(normal);
//  normal_27->addOutput(r);

  FuzzyRule *fuzzyRule27 = new FuzzyRule(27, panas_basah_tinggi_27, normal_27);
  fuzzy->addFuzzyRule(fuzzyRule27);

  }