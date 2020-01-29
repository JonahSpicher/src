# Coin Acceptor Blinking Light
Using a coin acceptor, toggles an LED on the Elecanisms board on and off when a coin is fed through the slot. 
### Requirements
Follow the instructions in [this document](http://elecanisms.olin.edu/handouts/1.1_BuildTools.pdf) to get all build tools ready. 

Next, set up the bootloader as described [here](https://github.com/JonahSpicher/src/blob/master/bootloader/README.txt).
### Instructions
1. Power the Elecanisms board with a 12V barrel jack and connect the board to your laptop. 
2. Connect the DC12V line on the coin acceptor to the Vin pin on the board. Connect the COIN line to pin D6 on the board, and connect the GND line to the board's GND.
3. Configure the coin acceptor to output one pulse for each penny that it accepts according to the instructions [here](http://cdn.sparkfun.com/datasheets/Components/General/3.jpg), specifically the Setting Process for Parameters and Sampling sections.
4. Compile blink.c using SCons and run the bootloader to write blink.hex to the baord (ensure the bootloader is running on the board by resetting the board while pressing SW1).
5. Click "Disconnect and Run" in the bootloader menu and the code should be functioning. 
