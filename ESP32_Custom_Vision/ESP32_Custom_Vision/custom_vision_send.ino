void custom_vision_send(String filename , camera_fb_t * fb, String Sendhost, String boundary, String Sendurl, String key){
  // Create postheader,keyheader,bodyheader
  String postheader,keyheader,bodyheader,bodyend;
  Serial.println("Preparing key header");
  keyheader  = keyheader_create(boundary);
  Serial.println("Preparing body header");
  bodyheader = bodyheader_create(boundary, filename);
  Serial.println("Preparing body end");
  bodyend    = bodyend_create(boundary);
  
  size_t allLen = keyheader.length() + fb->len + bodyheader.length() + bodyend.length();
  Serial.println("Preparing post header");
  postheader = postheader_create(boundary, Sendhost, Sendurl, allLen, key);
  Serial.println("Code: ");
  Serial.print(postheader + keyheader + bodyheader + bodyend);
  
  Serial.println("Starting connection to server");
  Serial.println(Sendhost);
  WiFiClientSecure client;
  if (!client.connect(SERVER,443)) {
    Serial.println("connection failed");
    return;   
  }
  client.print(postheader + keyheader + bodyheader);
  // client write
  client.write(fb->buf, fb->len);
  client.print(bodyend);
  delay(20);
  long tOut = millis() + TIMEOUT;
  Serial.print("waiting");
  while(client.connected() && tOut > millis()) {
    Serial.print("...");
    if (client.available()) 
    {
      String serverRes = client.readStringUntil('\r');
      Serial.print(serverRes);
      return;
    }
  }
  return;
}

String postheader_create(String boundary, String Sendhost, String Sendurl,size_t length, String key){
  String postheader;
  postheader = "POST " + Sendurl + " HTTP/1.1\r\n";
  postheader += "Host: " + Sendhost + "\r\n";
  postheader += "Prediction-Key: " + key + "\r\n";
  postheader += "Content-Type: multipart/form-data; boundary=----" + boundary + "\r\n";
  postheader += "Accept: */*\r\n";
  postheader += "Connection: keep-alive\r\n";
  postheader += "content-length: ";
  postheader += String(length);
  postheader += "\r\n";
  postheader += "\r\n";
  return postheader;
}

String keyheader_create(String boundary){
  String keyheader;
  keyheader = "------";
  keyheader += boundary;
  keyheader += "\r\n";
  keyheader += "Content-Disposition: form-data; name=\"key\"\r\n\r\n";
  return (keyheader);
}

String bodyheader_create(String boundary,String filename){
  String bodyheader;
  bodyheader = "------";
  bodyheader += boundary;
  bodyheader += "\r\n";
  bodyheader += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"" + filename + "\"\r\n";
  bodyheader += "Content-Type: Content-Type: image/jpg\r\n";
  bodyheader += "\r\n";
  bodyheader += "\r\n";
  return(bodyheader);
}
String bodyend_create(String boundary){
  String bodyEnd =  "------"+boundary+"--\r\n";
  return(bodyEnd);
}
