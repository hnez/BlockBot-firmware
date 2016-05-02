LD RA 64        // Load motor speed and direction Fwd 50%
LD RB 120       // Load time to drive for Two seconds


STA RA Mot1   // Set Motor 1
STA RA Mot2   // Set Motor 2

STA RB T1  // Setup timer

loop:
LDA RC T1 // Read timer value
SEQ RZ RC      // Skip next instruction if time is up
JBW loop       // Loop

STA RZ 0       // Stop Motor 1
STA RZ 1       // Stop Motor 2
