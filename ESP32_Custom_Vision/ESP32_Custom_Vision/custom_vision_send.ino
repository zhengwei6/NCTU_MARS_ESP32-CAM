/***********************************************************************
 *  ESP32 Custom Vision function
 *
 *  Author : Zheng Wei Wu(2019/09/21)
 * 
 *  input: 
 *        String filename : SD card file name
 *        String Sendhost : Custom Vision Server
 *        String Sendurl  : url of your project
 *        String key      : key of your project
 *  output : 
 *        String response
 ***********************************************************************/
String custom_vision_send(String filename, String Sendhost, String Sendurl, String key){
  String response = "";
  // Create postheader,keyheader,bodyheader
  WiFiClientSecure *client = new WiFiClientSecure;
  HTTPClient https;
  Serial.print("[HTTPS] begin...\n");

  // set up SSL client
  if (!client->connect(SERVER,443)) {
    Serial.println("connection failed");
    return response;   
  }else{
    Serial.println("Client connect success!");
  }

  // set up https and send file
  if (https.begin(*client,"https://westus2.api.cognitive.microsoft.com/customvision/v3.0/Prediction/58a8698a-abfb-4732-bfbb-ef5199b4ed0b/detect/iterations/Iteration1/image")) {
     Serial.println("begin sucess");
     https.addHeader("Accept", "application/json");
     https.addHeader("Content-Type", "application/octet-stream");
     https.addHeader("Prediction-Key", key);
     fs::FS &fs = SD_MMC; 
     File file = fs.open(filename, "r");
     https.sendRequest("POST", &file, file.size());
     delay(20);
     long tOut = millis() + TIMEOUT;
     Serial.print("waiting");
     while(client->connected() && tOut > millis()) {
       Serial.print("...");
       if (client->available()) 
       {
          response = client->readStringUntil('\r');
          Serial.print(response);
           https.end();
           file.close();
        }
     }
     https.end();
     file.close();
  }else{
    Serial.println("http client begin error");
  }
  
  return(response);
}
