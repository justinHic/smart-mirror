# Smart Mirror

Capstone project for Embedded Systems.

## Idea

Create a mirror with an embedded display behind the glass that will display the time, date, and weather. There will also be a virtual assistant that will turn on/off an LED strip behind the mirror and fetch the user's calendar events for the day. The user will be able to activate the virtual assistant using the keyword "Marvin" and then issue a voice command.

## Process

We will be using an Arduino as our MCU as well as an ADC microphone, an Adafruit LCD screen, and an LED strip.

We will need to train a neural network to identify the keyword "Marvin". We will use the Google Speech Commands dataset. It will be a challenge to fit the neural network onto the Arduino due to the limited memory capacity.
