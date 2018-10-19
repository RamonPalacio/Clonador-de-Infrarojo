
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>




#if DECODE_AC
const uint8_t kTimeout = 50;
#else 
const uint8_t kTimeout = 15;
#endif

const uint16_t kMinUnknownSize = 12;


IRsend irsend(D2);  // ESP8266 GPIO pin usado por el IR Trasmisor
IRrecv irrecv(D4, 1024, kTimeout, true); // (ESP8266 GPIO pin usado por el IR Receptor , kCaptureBufferSize , kTimeout ,  true)

decode_results results; 
int Reconocido = 0;



void setup() {
  Serial.begin(115200);

  Serial.println("Esperando Puso IR por el Pin D2");


#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.enableIRIn();  // Iniciar IR Emisor
  
  irsend.begin();       // Iniciar IR Transmisor
}

 String IR_Captado = "";

void loop() {
             
              // Verificar si Hay Datos IR
              if (irrecv.decode(&results) && Reconocido == 0) {
                                      if (results.overflow)
                                        Serial.printf("WARNING: IR code is too big for buffer (>= %d). "
                                                      "This result shouldn't be trusted until this is resolved. "
                                                      "Edit & increase kCaptureBufferSize.\n",
                                                      1024);
                                      yield();  // Si el codigo demora en ejecucion Pasa a la Siguiente funcion y luego termina de ejecutar esta  
                                      IR_Captado = resultToSourceCode(&results);
                                      Serial.println(IR_Captado);                         
                                      yield();  // Si el codigo demora en ejecucion Pasa a la Siguiente funcion y luego termina de ejecutar esta
                        Reconocido = 1;
              }
              
              if (Reconocido == 1){
                delay(5000);
                Enviar_IR(IR_Captado);
                Reconocido = 0;
                IR_Captado = "";
              }
  
}



void Enviar_IR(String Codigo_IR)
{
      uint16_t rawData[200] = {0};
      String sParams[200];
      int iCount, i;

      int inicio1 = Codigo_IR.indexOf("[")+1;
      int Final1 = Codigo_IR.lastIndexOf("]", Codigo_IR.length());
      
      int inicio2 = Codigo_IR.indexOf("{")+1;
      int Final2 = Codigo_IR.lastIndexOf("}", Codigo_IR.length());
      
      String HEX_Codigo = Codigo_IR.substring(Final2+2, Codigo_IR.length());
      
      int Total = Codigo_IR.substring(inicio1, Final1).toInt();
      
      Codigo_IR = Codigo_IR.substring(inicio2, Final2);
      Serial.println( "[" + (String) Total + "]" +  "-" + "{" +  Codigo_IR + "}-" + HEX_Codigo );
      
      
      
      if (Codigo_IR.length() > 0) {
        // parse the line
        iCount = StringSplit(Codigo_IR,',',sParams,Total);
        // print the extracted paramters
        for(i=0;i<iCount;i++) {
            //Serial.println(sParams[i]);
            rawData[i] = strtoul(sParams[i].c_str(), NULL, 10);
        }
        irsend.sendRaw(rawData, Total, 38);  // Send a raw data capture at 38kHz.
        Serial.println("Enviado!");
      }
}

int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams)
{
    int iParamCount = 0;
    int iPosDelim, iPosStart = 0;

    do {
        // Searching the delimiter using indexOf()
        iPosDelim = sInput.indexOf(cDelim,iPosStart);
        if (iPosDelim > (iPosStart+1)) {
            // Adding a new parameter using substring() 
            sParams[iParamCount] = sInput.substring(iPosStart,iPosDelim);
            sParams[iParamCount].trim();
            iParamCount++;
            // Checking the number of parameters
            if (iParamCount >= iMaxParams) {
                return (iParamCount);
            }
            iPosStart = iPosDelim + 1;
        }
    } while (iPosDelim >= 0);
    
    if (iParamCount < iMaxParams) {
        // Adding the last parameter as the end of the line
        sParams[iParamCount] = sInput.substring(iPosStart);
        sParams[iParamCount].trim();
        
        iParamCount++;
    }

    return (iParamCount);
}
