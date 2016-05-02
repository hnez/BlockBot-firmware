LD RA           // Load motor speed and direction
64              // Fwd 50%
LD RB           // Load time to drive for
120             // Two seconds


STA Mot1 RA    // Set Motor 1
STA Mot2 RA    // Set Motor 2

STA Timer RB   // Setup timer

loop:
LDA Timer RC  // Read timer value
SEQ RZ RC      // Skip next instruction if time is up
JBW loop       // Loop

STA Mot1 RZ       // Stop Motor 1
STA Mot2 RZ       // Stop Motor 2
