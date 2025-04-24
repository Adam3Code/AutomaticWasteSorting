
  void handleRoot() {
  String html = "<!DOCTYPE html>"
                "<html lang='da'>"
                "<head>"
                  "<meta charset='UTF-8'>"
                  "<meta http-equiv='refresh' content='2'>" // Auto-refresh hver 2. sekund
                  "<title>Recycle</title>"
                  "<style>"
                    "body { background-color: #121212; color: #FFA500; font-family: Arial, sans-serif; text-align: center; padding: 40px; }"
                    "h1 { font-size: 2.5em; }"
                    ".counter { font-size: 2em; margin: 20px; }"
                    ".warning { color: red; font-size: 1.5em; margin-top: 20px; }"
                  "</style>"
                "</head>"
                "<body>"
                  "<h1>Trashcan status</h1>"
                  "<div class='counter'>Items in trash can: <span id='countDisplay'>?</span></div>"
                  "<div id='warning' class='warning' style='display: none;'>⚠️ Empty trash can!</div>"

                  "<script>";

  // Indsæt den aktuelle skraldeTæller værdi fra Arduino
  html += "let count = " + String(foodwaste) + ";";
  
  html += "document.getElementById('countDisplay').innerText = count;";
  html += "if (count >= 10) { document.getElementById('warning').style.display = 'block'; }";

  html += "</script>"
         "</body>"
         "</html>";

  server.send(200, "text/html", html);
}
