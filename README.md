# Steppermidi
Simple way to play midi on a stepper motor

## Needed Hardware
- ESP32 or similar
- A4988 stepper motor driver
- stepper motor
- power supply for stepper motor

# What does it do?
The ESP32 is using the Midi library to listen to Midi data coming in through the serial port (i.e. USB connected to your PC)
It will then drive the stepper motor to produce the note required by the Midi data.

## Features
- Logs on to your wifi and produces some debugging output on telnet port (since serial is occupied with MIDI data)
- Calculates a lookup table for intervals needed to play a certain note on the fly at setup
- Accepts Midi files streamed from your PC
- blinks LED

## Constraints
There is only one channel supported. If you have a midi file with multiple channel, you need to filter out the channel beforehand and stream that to the ESP

# How to do it

## On the PC
### Prepare your MIDI file.
You can use timidity to quieten unwanted tracks and output the result to a file
```timidity -OmM -o imperialmarch_trumpet.mid -Q 1,2,3,4,6,7,8,9,10,11,12,13,14 Downloads/imperialmarch.mid```

### Prepare the MIDI out via serial
- Download ttymidi from https://code.launchpad.net/~ttymidi-team/ttymidi/trunk (using bzr)
- build it (just “make” in the directory)

Use ttymidi to create an ALSA port which is bound to a Serial Device /dev/ttyUSB0

```./ttymidi -v -s /dev/ttyUSB0 -n myweirdcontroller```

### Play MIDI via Serial
Check which ports are available to play to 
```
    pmidi -l
     Port     Client name                       Port name
     14:0     Midi Through                      Midi Through Port-0
    128:1     myweirdcontroller                 MIDI in
``` 

Use pmidi to play a midi file to that ALSA port (decisive pointer on that gem. It’s about using timidity as ALSA input. Just substitute with ttymidi as above)

```pmidi -p128:1 imperialmarch_trumpet.mid```
