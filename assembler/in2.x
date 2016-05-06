LD RA 64        // Load motor speed and direction Fwd 50%
LD RB 1         // Load bitmask

STA Mot1 RA     // Set Motor 1
STA Mot2 RA     // Set Motor 2

waitbtn:
LDA DIn RC      // Load digital input register
AND RC RB       // Mask out all uninteresting bits
SNE RZ RC       // Check if bit is set
JBW waitbtn

MOV RB RA       // Make a backup of the motor speed
NEG RB          // and invert it

STA Mot1 RB     // Drive backwards Motor 1
STA Mot2 RB     // Drive backwards Motor 2

delay:
LD RC 60        // Load driving time
STA Timer RC    // Start timer

delaylp:
LDA Timer RC    // Load timer value
SEQ RZ RC       // Check if time is up
JBW delaylp     // Loop if it is not

SNE RZ RA       // When run for the first time RA contains the motor speed
JFW stop        // When run for the second time it is set to zero below

STA Mot1 RA     // Drive forwards Motor 1
MOV RA RZ       // Indicate that motors should be stoped next time
JBW delay       // Delay again

stop:
STA Mot1 RZ     // Stop Motor 1
STA Mot2 RZ     // Stop Motor 2
