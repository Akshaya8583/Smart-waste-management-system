#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

/**
 * Initialize.
 */ 
#define DT A0
#define SCK A1
#define sw 9
 
long sample=0;
float val=0;
long count=0;
int points =0; 
int acc=0;
int sense =0;
const int metal_input=8;
int isCard =0;
unsigned long readCount(void){
  unsigned long Count;
  unsigned char i;
  pinMode(DT, OUTPUT);
  digitalWrite(DT,HIGH);
  digitalWrite(SCK,LOW);
  Count=0;
  pinMode(DT, INPUT);
  while(digitalRead(DT));
  for (i=0;i<24;i++)
  {
  digitalWrite(SCK,HIGH);
  Count=Count<<1;
  digitalWrite(SCK,LOW);
  if(digitalRead(DT))
  Count++;
  }
  digitalWrite(SCK,HIGH);
  Count=Count^0x800000;
  digitalWrite(SCK,LOW);
  return(Count);
}
 
void setup(){
  pinMode(SCK, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  pinMode(metal_input, INPUT);
  lcd.begin(16, 2);
  lcd.print(" Weight");
  lcd.setCursor(0,1);
  lcd.print(" Measurement ");
  delay(1000);
  lcd.clear();
  calibrate();
  Serial.begin(115200); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
  Serial.print(F("Using key (for A and B):"));
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.clear();
}
byte i=0; 
byte sector         = 1;
byte blockAddr      = 5;
byte dataBlock[]    = {
    0x00, 0x00, 0x00, 0x00, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x00, //  5,  6,   7,  8,
    0x00, 0x00, 0x00, 0x00, //  9, 10, 255, 11,
    0x00, 0x00, 0x00, 0x00  // 12, 13, 14, 15
};

int previous =0;
int num =0;
byte weight_measure(){    //this function calculates the difference in weight added and calculates points
  count= readCount();
  int w=(((count-sample)/val)-2*((count-sample)/val));
  points = (w-previous)/10;
  previous =w;
  lcd.setCursor(0,0);
  lcd.print(points);
  lcd.print("POINTS GAINED");
  lcd.setCursor(0,1);
  lcd.print("SCAN TO ADD");
  
  if(digitalRead(sw)==0){
  val=0;
  sample=0;
  w=0;
  count=0;
  calibrate();
  }
}
int k=0;
void loop(){

  if(k==0){   //output of the metal sensor is taken as the input
    sense = digitalRead(metal_input);
    delay(3000);
  }
  if(sense ==1 ){ //measurinf the weight when the metal is added(detected)
    weight_measure();
    delay(10000);
    acc = acc + points;   //accumulator variable to add many wastes and collective points are added with one scan
    i=0;
    while(isCard == 0){   //loops till the card is scanned
      card();           
      sense = digitalRead(metal_input); //if another waste is added instead of  scanning points are added to accumulator
      delay(3000);
      if(sense ==1 ){ 
        isCard=1;   //if card is scanned wait in loop till new waste is added.
        k=1;
      } 
    }
    isCard = 0;
  }
  
}

void card(){    //This function is to read and write into the rfid card
    //data is written into the 1 section 5 block 1 element
   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent()){
        return;
    }

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }

    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7

 
    byte trailerBlock   = 7;
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Show the whole sector as it currently is
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    // Read data from the block
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();

    
    if(i==0){
      dataBlock[0]=buffer[0]+acc;   //adding points and displaying the current total of the card
      lcd.clear();
      lcd.print("POINTS ADDED");
      lcd.setCursor(0,1);
      lcd.print("Status: ");
      lcd.print(dataBlock[0]);
      acc=0;
      i++;
      isCard =1;
      k=0;
    }

    Serial.println();

    // Authenticate using key B
    Serial.println(F("Authenticating again using key B..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Write data to the block
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr); 
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();

    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == dataBlock[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
        Serial.println(F("Success :-)"));
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();

    // Dump the sector data
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}
void calibrate(){
  lcd.clear();
  lcd.print("Calibrating...");
  lcd.setCursor(0,1);
  lcd.print("Please Wait...");
  for(int i=0;i<100;i++)
  {
  count=readCount();
  sample+=count;
  }
  sample/=100;
  lcd.clear();
  lcd.print("Put 100g & wait");
  count=0;
  while(count<1000)
  {
  count=readCount();
  count=sample-count;
  }
  lcd.clear();
  lcd.print("Please Wait....");
  delay(2000);
  for(int i=0;i<100;i++)
  {
  count=readCount();
  val+=sample-count;
  }
  val=val/100.0;
  val=val/100.0; // put here your calibrating weight
  lcd.clear();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}