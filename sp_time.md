# Susanne Paal Log-Buch

## 17.03.25, Montag

### KI

- 2,5 Stunden
- Kick-Off Besprechung von Lehrkraft:KI und Vorstellung mit Anforderung und Vision
- Teambuilding Susanne, Hüseyin
- Python Pakete Vorstellung yolo, pytesseract, opencv, pandas, pygame

---

## 18.03.25, Dienstag

### KI

- 1 Stunde
- Abhängigkeitspakete Evaluierung und Reasearching

---

## 19.03.25, Mittwoch

### MCR

- 2,5 Stunden
- Erstellung Lerntagebuch
- Einarbeitung Mosquitto Broker mit [Tutorial](https://cedalo.com/blog/how-to-install-mosquitto-mqtt-broker-on-windows/)

---

## 24.03.25, Montag

### MCR und KI

- 1,5 Stunden
- Erstellung Konzept und Projektgliederung in Arbeitspakete

---

## 28.03.25, Freitag

### MCR

- 4 Stunden
- Erstellung PlatformIO Projekt
- Import von vorhandenen Dateien zu LCD-Display und Servo
- Erstellung Konzept State_Machine.png
- Erstellung state_machine.h

---

## 31.03.25, Montag

### MCR

- 3 Stunden
- Aktualisierung der State Machine: Besprechung gewünschtes Verhalten, wenn Kennzeichen nicht erkannt werden kann. Ergebnis: Kennzeichen soll in maximal 3 Versuchen erkannt werden, falls nicht möglich, soll über einen Error wieder in den Zustand IDLE gewechselt werden
- Erstellung state_machine.cpp mit Switch Case -> Logik muss noch implementiert werden
- Recherche für Verwedung der richtigen GPIO-Pins für Servo und LCD-Display
- Fortsetzung Einarbeitung Mosquitto Broker mit ersten Tests, ob Mosquitto lokal funktioniert

Next Steps:

- Mosquitto vollständig konfigurieren
- Einarbeitung Kamera-Modul

---

## 08.04.25, Dienstag

### MCR

- 1,5 Stunden
- Recherche zu Infrarot-Sensor für Objekterkennung
- Erstellung ir_sensor.h und ir_sensor.cpp
- Einarbeitung Kamera-Modul, Problemstellung: Arbeiten mit nur einem ESP nicht mehr möglich, da nicht genug Pins vorhanden sind; Entscheidung für zusätzliche Umgebung für die ESP32-Cam, noch unklar wie genau die beiden ESPs miteinander kommunizieren werden

---

## 09.04.25, Mittwoch

### MCR

- 2 Stunden
- Versuch, zwei ESPs mit zwei Environments und zwei main.cpps in einem PlatformIO Projekt zu verwalten.
- Gespräch mit Hr. Malassa: Konzentration vorerst auf Haupt-Controller, mit Funktionalität Button, LEDs, Servo, Display und Infrarot Sensor. Wenn ESP-Cam mit eingebunden wird, soll ein separates PlatformIO Projekt erstellt werden.

---

## 16.04.25, Mittwoch

### MCR

- 4 Stunden
- Integration der State Machine in die main.cpp, da einfacher in der Umsetzung, keine Pointer notwendig
- Erste Implementierung der State Machine mit Servo, LEDs, Button, Display und Infrarot Sensor
- Display funktioniert, aber flackert
- Problem: State Machine mit IR Sensor funkioniert noch nicht richtig. State Idle nur als Startzustand, nach einmaligem Durchlauf wird direkt von Closing wieder zu Scanning gewechselt, Schranke also in Dauerschleife am öffnen und schließen, selbst wenn IR Sensor nicht verbunden ist. Rote LED nur sehr kurzzeitig am leuchten, grüne LED leuchtet nicht, hängt wahrscheinlich mit IR Sensor zusammen. Wenn IR Sensor im Code auskommentiert wird, funktioniert auch Button noch nicht immer. Übergänge zwischen States manchmal nicht möglich
- Hinzufügen von seriellen Ausgaben, in welchem State sich der Code aktuell befindet
- Aktuelle Vermutung zum Button: Beinchen des Buttons zu kurz, um mit dem Breadboard vernünftige Verbindung zu bekommen
- Lösung für Button gefunden: Button wurde direkt mit Kabeln verlötet, um Breadboard zu umgehen -> Button funktioniert jetzt

Next Steps:

- Änderung State Machine: Aktuell bleibt State Machine in einem State hängen oder läuft in Dauerschleife an Idle vorbei
- State Machine soll nur mit Button und LEDs funktionieren, danach Einbindung Display und IR Sensor
- Display soll nur aktualisiert werden, wenn neuer State erreicht wird, nicht bei jedem Loop Durchlauf, so soll flackern verhindert werden

## 23.04.25, Mittwoch

### MCR

- 4 Stunden
- Test auf Funktionalität Button -> Funktioniert weiterhin
- Test auf Funktionalität LEDs -> Funktioniert, nach richtigem Anstecken auf Breadboard von Ground
- Test auf Funktionalität Servo -> Funktioniert
- Test auf Funktionalität Display -> Funktioniert, ohne State Machine auch kein dauerndes Flackern im Display
- Display flackert nur, wenn Servo sich bewegt
  Entscheidung: Display bleibt bei 5V, Servo läuft auf 3,3V, dadurch etwas langsamer, aber Display flackert weniger
- Test auf Funkionalität IR Sensor -> Funktioniert, nach Änderung der ir_sensor.cpp, dass digitalRead() LOW zurück gibt, wenn ein Objekt erkannt worden ist
- Hinzufügung WLAN-Anbindung über <WiFi.h> mit Auslagerung der Anmeldedaten in wifi_config.h
- Integrierung Anzeige aktuelle Zeit über die integrierte Funktion configTime() auf Display, dadurch muss für die Zeitanzeige keine weitere Bibliothek verwendet werden
- Hinzufügen Funktion updateLEDs() zur Prüfung und Anzeige freier Parkplätze über die LEDs

## 24.04.25, Donnerstag

### MCR

- 5 Stunden
- Versuch Anpassung WLAN-Anbindung mit System-Umgebungsvariablen:
  Hat nicht funkioniert, keine WLAN Verbindung möglich, obwohl Umgebungsvariablen in platformio.ini hinterlegt waren
  Problem mit .env Datei nicht lösen können
  Rollback zur WLAN Anbindung über Anmeldedaten in wifi_config.h
- Einrichten lokaler MQTT Server, allerdings nicht als App in Windows Firewall gelistet, daher Erstellung manueller Regel in erweiterten Firewall-Einstellungen notwendig
- Anpassen der State Machine, entfernen der States, die nicht für den ESP benötigt werden, sondern KI-Teil sind (Checking_CSV, ...), um Logik für ESP zu vereinfachen
- Besprechung MQTT Nachrichten, Verwendung des JSON-Formates, Absprache, wann Nachrichten mit welchem Inhalt gesendet werden sollen
- Implementierung State Machine -> Nicht funktionsfähig, Übergänge werden nicht korrekt ausgeführt, Aktionen für Display und Servo werden nicht getriggert

Next Steps:

- Testen der MQTT-Kommunikation zwischen ESP und Laptop
- State Machine weiter bearbeiten, Debugging hinzufügen

## 25.04.25, Freitag

### MCR

- 6 Stunden
- Testen MQTT-Kommunikation: Verbindung von ESP zu MQTT Broker nicht möglich, obwohl ESP und Broker im selben Netzwerk verbunden sind und MQTT Dienst auf Laptop ausgeführt wird
  Problem: mosquitto_sub mit localhost funktioniert, mit IP des Laptops als Host keine Verbindung möglich
  Lösung: In mosquitto.conf war als Listener nur Port 1883 hinterlegt, keine IP-Adresse, weshalb nur auf localhost gehört wurde. Anpassung: 0.0.0.0 für Listener hinzugefügt, damit Mosquitto auf allen Netzwerkschnittstellen hört, da IP-Adressen dynamisch vergeben werden. Test mit manuellem Neustart von Mosquitto erfolgreich.
  Problem: Mosquitto Broker lässt sich als Dienst starten, aber beendet sich sofort selbst wieder. Erstellung einer .log Datei ergab: Fehler beim Öffnen der passwordfile, die von Mosquitto automatisch erstellt wurde.
  Lösung: Anpassung der Dateiberechtigungen der passwordfile, um den Zugriff für Mosquitto zu ermöglichen. Mosquitto lässt sich als Dienst jetzt wieder starten.
- Weiterentwicklung State Machine: Hinzufügen Debugging der Zustandsübergänge und Aktionen (LCD, Servo).
  Fortschritt: Zustandsübergänge für Button und Wartezeiten innerhalb der States funktionieren.
  Problem IR: Wenn Sensor Objekt erkennt, muss Timeout für IR Sensor integriert werden, da sonst State Machine nie vollständig durchlaufen werden kann
  Problem Button: States werden erfolgreich durchlaufen, aber Aktionen innerhalb der States werden nicht ausgeführt (keine Servo-Bewegung, Zeit bleibt auf Display in Nicht-STATE-IDLE-Zuständen stehen)

Next Steps:

- Tests State Machine mit Servo und Display separat auskommentiert
- Hinzufügen weiterer Debug-Ausgaben vor und nach den switch- und in jedem case-Block
- Wenn Button funktionsfähig wird, als nächstes IR Sensor
- WLAN Anpassungen nochmal recherchieren und versuchen

## 26.04.25, Samstag

### MCR

- 6 Stunden
- Weiterentwicklung State Machine:
- Ausfahrt durch Button jetzt funktionsfähig.
  Problem war: Zeitliche Abfolge von Zustandsänderungen und Aktionen falsch, Zustand wurde bereits durch loop() wieder überschrieben, bevor Aktionen durchgeführt werden konnten.
- Einfahrt durch IR Sensor jetzt funktionsfähig.
  Problem: ESP ist abgestürtzt, da MQTT Nachricht nicht richtig verarbeitet werden konnte, Anführungszeichen in JSON Strings wurden nicht richtig erkannt
  Lösung: Anführungszeichen werden durch Backslashes jetzt korrekt erkannt und verarbeitet
  Problem: Ausfahrt durch Button nicht mehr möglich
  Lösung: Nochmaliges Anpassen der Zustandsmaschine und Button-Klasse, dass Button-Druck wieder zuverlässig erkannt wird
- Anpassung des Zählers für belegte Parkplätze, da bei Einfahrten Zähler erst nach MQTT Nachricht aktualisiert worden ist
- Hinzufügen neuer Funktion für zweizeilige Anzeigen auf dem Display
- Hinzufügen neuer State FULL für die Anzeige "Parkhaus voll", wenn keine Plätze mehr frei sind, aber der IR Sensor ein Objekt erkennt

Next Steps:

- Testen gesamtes Parkhaus
- Anpassen der JSON-Strings am Montag für die MQTT Nachrichten, damit KI damit arbeiten kann
- WLAN Anpassungen nochmal recherchieren und versuchen
- Parkhaus Modell bauen

## 27.04.25, Sonntag

### MCR

- 2 Stunden
- Anpassung State Machine: MQTT Timeout im State SCANNING läuft jetzt auch zuerst in den State ERROR, bevor wieder zu IDLE gewechselt wird
- Überarbeitung State Machine Diagramm, Anpassung an aktuellen Code
- Hinzufügung von Build-Flags in platformio.ini für WLAN- und MQTT-Zugangsdaten - nicht funktionsfähig

## 28.04.25, Montag

### MCR

- 3 Stunden
- Fertigstellung Auslagerung der WLAN- und MQTT-Zugangsdaten
  Lösung: Build-Flags nicht nur in Anführungszeichen setzen, sonder auch richtig escapen (\"WLAN-SSID\")
- Gemeinsame Überarbeitung der MQTT-Topics und -Nachrichten, das der ESP korrekt mit der KI kommunizieren kann
- Umsetzung der MQTT-Anpassungen im MCR-Code
- Testen gesamtes Parkhaus zusätzlich zur MQTT-Kommunikation

## 29.04.25

### MCR

- 4 Stunden
- Aufbau Parkhaus-Modell
- Testen gesamtes Projekt im Modell
