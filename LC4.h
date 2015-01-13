/*
 * LC4.h
 */

typedef struct {

  // PC the current value of the Program Counter register
  unsigned short int PC;

  // PSR : Processor Status Register, bit[0] = P, bit[1] = Z, bit[2] = N, bit[15] = privilege bit
  unsigned short int PSR;

  // Machine registers - all 8
  unsigned short int R[8];

  // Machine memory - all of it
  unsigned short int memory[65536];

} MachineState;

// Reset the machine state as Pennsim would do
void Reset (MachineState *theMachineState);

// Update Machine State - simulate how the state of the machine changes over a single clock cycle
int UpdateMachineState (MachineState *theMachineState);

// Note that the UpdateMachineState function must perform its action using the helper functions
// declared below which should be used to simulate the operation of portions of the datapath.
//
// Note that all of the control signals passed as arguments to these functions are represented
// as unsigned 8 bit values although none of them use more than 3 bits. You should use the lower
// bits of the fields to store the mandated control bits. Please refer to the LC4 information sheets
// on Canvas for an explanation of the control signals and their role in the datapath.


// Compute the current output of the RS port
unsigned short int RS (MachineState *theMachineState, unsigned char rsMux_CTL);

// Compute the current output of the RT port
unsigned short int RT (MachineState *theMachineState, unsigned char rtMux_CTL);

// Compute the current output of the ALUMux
unsigned short int ALUMux (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char Arith_CTL,
			   unsigned char ArithMux_CTL,
			   unsigned char LOGIC_CTL,
			   unsigned char LogicMux_CTL,
			   unsigned char SHIFT_CTL,
			   unsigned char CONST_CTL,
			   unsigned char CMP_CTL,
			   unsigned char ALUMux_CTL);

// Compute the current output of the regInputMux
unsigned short int regInputMux (MachineState *theMachineState,
				unsigned short int ALUMuxOut, // current ALUMux output
				unsigned char regInputMux_CTL);

// Compute the current output of the PCMux
unsigned short int PCMux (MachineState *theMachineState,
			  unsigned short int RSOut,
			  unsigned char PCMux_CTL);
