import controlP5.*;
import processing.serial.*;

Serial port;

ControlP5 cp5;

float[]sensorValues;

int buttonWidth = 80;
int buttonHeight = 80;
String[] buttonNames = {"valve_1_button", "valve_2_button", "valve_3_button", "valve_4_button", "exhaust_button", "pump_button"};
String[] buttonLabels = {"Valve 1", "Valve 2", "Valve 3", "Valve 4", "Exhaust", "Pump"};
int[][] buttonPositions = {{50, 150}, {50, 50}, {150, 50}, {150, 150}, {280, 50}, {280, 150}};
Toggle[] buttons = new Toggle[buttonLabels.length];

void setup() {
   
  size(400, 700);
  port = new Serial(this, "COM6", 9600);
  cp5 = new ControlP5(this);

  // Create the buttons
  for (int i = 0; i < buttonNames.length; i++) {
      Toggle toggleButton = cp5.addToggle(buttonNames[i]);
      toggleButton.setPosition(buttonPositions[i][0], buttonPositions[i][1]);
      toggleButton.setSize(buttonWidth, buttonHeight);
      toggleButton.setColorBackground(color(200));
      toggleButton.setColorActive(color(20, 200, 20));
      toggleButton.setColorLabel(color(0));
      toggleButton.setLabel(buttonLabels[i]);
      toggleButton.getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
      buttons[i] = toggleButton;
  }
  
  sensorValues = new float[4];

}

void draw() {
  background(255);
  
  // Display the bar graph
  drawBarGraph();
  
  // Draw other components
  fill(200);
  stroke(200);
  rect(115, 240, 50, 15);
  fill(0);
  textAlign(CENTER, CENTER);
  text("Front", 115 + 50/2, 240 + 15/2);
}

void serialEvent(Serial p) {
  String input = p.readStringUntil('\n');
  if (input != null) {
    println("Received from Arduino: " + input);
    String[] inputValues = input.split(",");
    for (int i = 0; i < sensorValues.length; i++) {
      sensorValues[i] = Float.parseFloat(inputValues[i]);
    }
  }
}

void drawBarGraph() {
  float barWidth = (width - 100) / sensorValues.length; // Width of each bar
  float maxValue = 5; // Change to represent the highest psi value
  String[] sensorLabels = {"Sensor 1", "Sensor 2", "Sensor 3", "Sensor 4", "Voltage"};

  for (int i = 0; i < sensorValues.length; i++) {
    float barHeight = map(sensorValues[i], 0, maxValue, 0, height - 200); // Scale bar height based on data value
    float x = (i * (barWidth + 5)) + 40; // X position of the bar
    float y = (height - 20) - barHeight; // Y position of the bar
    
    fill(100);
    
    // Draw the bar
    rect(x, y, barWidth, barHeight);
    
    // Optional: Draw labels
    fill(0);
    textAlign(CENTER, CENTER);
    text(sensorLabels[i], x + barWidth / 2, y - 30);
    text(sensorValues[i], x + barWidth / 2, y - 10);
  }
}

void valve_1_button() {
  port.write('a');
}

void valve_2_button() {
  port.write('b');
}

void valve_3_button() {
  port.write('c');
}

void valve_4_button() {
  port.write('d');
}

void exhaust_button() {
  port.write('e');
}

void pump_button() {
  port.write('f');
}

void dispose() {
  cp5.remove("valve_1_button");
  cp5.remove("valve_2_button");
  cp5.remove("valve_3_button");
  cp5.remove("valve_4_button");
  cp5.remove("exhaust_button");
  cp5.remove("pump_button");
  super.dispose();
}
