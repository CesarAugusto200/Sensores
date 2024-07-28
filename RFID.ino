#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9  
#define SS_PIN          10  

MFRC522 mfrc522(SS_PIN, RST_PIN);  

// Almacenar los datos de las tarjetas
struct RFIDCard {
  byte id[4];           // ID de la tarjeta (4 bytes)
  String name;         
  String requestTime;  
  String returnTime;   
};


RFIDCard cards[10];
int cardCount = 0;

void setup() {
  Serial.begin(9600);    
  while (!Serial);       
  SPI.begin();          
  mfrc522.PCD_Init();   
  delay(4);             
  mfrc522.PCD_DumpVersionToSerial(); 
  Serial.println(F("Escanea una tarjeta para iniciar el préstamo o registrar la devolución..."));
}

void loop() {
  // Verificar si hay una tarjeta nueva presente
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Seleccionar la tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Mostrar UID de la tarjeta en la consola
  Serial.print(F("UID de la tarjeta: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Verificar si la tarjeta ya está registrada
  bool found = false;
  for (int i = 0; i < cardCount; i++) {
    if (compareUID(cards[i].id, mfrc522.uid.uidByte)) {
      found = true;
      if (cards[i].returnTime == "") {
        // La tarjeta está registrada pero aún no ha sido devuelta
        Serial.print(F("Tarjeta registrada. Nombre Del Maestro: "));
        Serial.println(cards[i].name);
        Serial.print(F("Hora de solicitud: "));
        Serial.println(cards[i].requestTime);
        Serial.print(F("Hora de devolución: No registrada aún."));
        Serial.println();
        
        // Solicitar hora de devolución
        Serial.println(F("Introduce la hora de devolución (formato HH:MM):"));
        String returnTime = readSerialString(); // Leer la hora de devolución

        // Guardar la hora de devolución
        cards[i].returnTime = returnTime;

        // Mostrar todos los datos y luego limpiar los campos de hora de solicitud y devolución
        Serial.print(F("Tarjeta devuelta. Nombre: "));
        Serial.println(cards[i].name);
        Serial.print(F("Hora de solicitud: "));
        Serial.println(cards[i].requestTime);
        Serial.print(F("Hora de devolución: "));
        Serial.println(cards[i].returnTime);
        
        // Limpiar los datos de solicitud y devolución, pero mantener el nombre
        cards[i].requestTime = "";
        cards[i].returnTime = "";

        Serial.println(F("Datos de solicitud y devolución eliminados. Puede registrar un nuevo préstamo con esta tarjeta."));
      } else {
        // La tarjeta ha sido devuelta
        Serial.print(F("Tarjeta devuelta. Nombre: "));
        Serial.println(cards[i].name);
        Serial.print(F("Hora de solicitud: "));
        Serial.println(cards[i].requestTime);
        Serial.print(F("Hora de devolución: "));
        Serial.println(cards[i].returnTime);
        Serial.println();
      }
      break;
    }
  }

  // Si la tarjeta no está registrada, solicitar nombre y hora de solicitud
  if (!found) {
    if (cardCount < 10) {
      Serial.println(F("Introduce el nombre del Maestro para la tarjeta (usando el monitor serial):"));
      String name = readSerialString(); // Leer el nombre ingresado

      // Obtener hora de solicitud manualmente
      Serial.println(F("Introduce la hora de solicitud (formato HH:MM):"));
      String requestTime = readSerialString(); // Leer la hora de solicitud

      // Guardar los datos de la tarjeta
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        cards[cardCount].id[i] = mfrc522.uid.uidByte[i];
      }
      cards[cardCount].name = name;
      cards[cardCount].requestTime = requestTime;
      cards[cardCount].returnTime = ""; // Hora de devolución no está registrada aún
      cardCount++;

      Serial.print(F("Tarjeta registrada con nombre del Maestro: "));
      Serial.println(name);
      Serial.print(F("Hora de solicitud: "));
      Serial.println(requestTime);
    } else {
      Serial.println(F("Número máximo de tarjetas registradas alcanzado."));
    }
  }

  // Detener la comunicación con la tarjeta
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// Comparar UID de las tarjetas
bool compareUID(byte *uid1, byte *uid2) {
  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// Leer una cadena desde el monitor serial
String readSerialString() {
  String inputString = "";
  char receivedChar;

  while (true) {
    if (Serial.available()) {
      receivedChar = Serial.read();
      if (receivedChar == '\n') {
        break; // Fin de la entrada
      } else {
        inputString += receivedChar; // Agregar carácter a la cadena
      }
    }
  }

  inputString.trim(); 
  return inputString; 
}
