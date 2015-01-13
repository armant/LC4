/*
 * LC4.c
 */

#include <stdlib.h>

#include "LC4.h"

// Reset the machine state as Pennsim would do
void Reset (MachineState *theMachineState) {
  int i;
  printf("RESET\n");
  theMachineState -> PC = 0x8200;
  theMachineState -> PSR = 0x8002;

  for (i = 0; i < 8; i++) {
  	theMachineState -> R[i] = 0;
  }

  for (i = 0; i < 65536; i++) {
  	theMachineState -> memory[i] = 0;
  }

}

signed short int passedNZP (MachineState *theMachineState) {
  
  if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 1) {
    printf("BRp IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 2) {
    printf("BRz IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 3) {
    printf("BRzp IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 4) {
    printf("BRn IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 5) {
    printf("BRnp IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 6) {
    printf("BRnz IMM9\n");
  } else if (((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7) == 7) {
    printf("BRnzp IMM9\n");
  }

  return theMachineState -> PSR & 
         ((theMachineState -> memory[theMachineState -> PC] >> 9) & 0x7);
}

void updateNZP(MachineState *theMachineState, signed short int value) {
  theMachineState -> PSR = theMachineState -> PSR & 0x8000;

  if (value < 0) {
    theMachineState -> PSR += 4;
    printf("N is true\n");
  } else if (value == 0) {
    theMachineState -> PSR += 2;
    printf("Z is true\n");
  } else {
    theMachineState -> PSR += 1;
    printf("P is true\n");
  }

}

void saveResult(MachineState *theMachineState, unsigned short int regInputOut) {
  unsigned short int index;

  index = ((theMachineState -> memory[theMachineState -> PC]) >> 9) & 0x7;
  printf("Saved in R[%d] %x\n", index, regInputOut);
  theMachineState -> R[index] = regInputOut;
}

// Update Machine State - simulate how the state of the machine changes over a single clock cycle
int UpdateMachineState (MachineState *theMachineState) {
  unsigned short int instr, opcode, subopcode, ALUout, regInputOut;
  unsigned char ch1, ch2;
  signed short int temp;

  instr = theMachineState -> memory[theMachineState -> PC];

  if (theMachineState -> PC == 0x80FF) { // Instruction for EXIT
    printf("PC is 80FF. Exiting LC4.\n");
    return 2;
  }

  if ((theMachineState -> PC >= 0x2000) & (theMachineState -> PC <= 0x7FFF)) {
    printf("ERROR: trying to interpret user data as code. Exiting LC4.\n");
    return 1;
  }

  if (theMachineState -> PC >= 0xA000) {
    printf("ERROR: trying to interpret OS data as code. Exiting LC4.\n");
    return 1;
  }

  if ((theMachineState -> PC >= 0x8000) & (theMachineState -> PC <= 0x9FFF) &
    (theMachineState -> PSR >> 15 == 0)) {
    printf("ERROR: trying to execute OS code in user mode. Exiting LC4.\n");
    return 1;
  }

  printf("Current command is %x \n", instr);
  opcode = instr >> 12;

  if (opcode == 0) { // NOP/BR
    subopcode = (instr >> 9) & 0x7;

    if (subopcode == 0) { // NOP
      printf("NOP\n", subopcode);
      theMachineState -> PC = PCMux(theMachineState, 0, 1);
    } else { // BR
      theMachineState -> PC = PCMux(theMachineState, 0, 0);
    }

  } else if (opcode == 1) { // Arithmetic
    subopcode = (instr >> 3) & 0x7;

    if (subopcode == 0) { // ADD
      printf("ADD R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);

      ch1 = 'A';
      ch2 = 'R';
    } else if (subopcode == 1) { // MUL
      printf("MUL R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);

      ch1 = 'M';
      ch2 = 'R';
    } else if (subopcode == 2) { // SUB
      printf("SUB R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);

      ch1 = 'S';
      ch2 = 'R';
    } else if (subopcode == 3) { // DIV
      printf("DIV R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);

      ch1 = 'D';
      ch2 = 'R';
    } else if (subopcode >= 4) { // ADD IMM
      temp = instr & 31;
      temp <<= 11;
      temp >>= 11;

      printf("ADD R%d, R%d, #%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          temp);

      ch1 = 'A';
      ch2 = '5';
    }

    ALUout = ALUMux(instr, RS(theMachineState, 0), RT(theMachineState, 0), 
          ch1, ch2, '_', '_', '_', '_', '_', 'A');
    regInputOut = regInputMux(theMachineState, ALUout, 0);
    saveResult(theMachineState, regInputOut);
    
    if (subopcode == 1 | subopcode == 3) {
      updateNZP(theMachineState, regInputOut);
    } else {
      updateNZP(theMachineState, (signed short int) regInputOut);
    }

    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 2) { // CMP
    printf("Current opcode is %d and the operation is CMP\n", opcode);
    subopcode = (instr >> 7) & 0x3;
    printf("Current subopcode is %d\n", subopcode);
    ch1 = (unsigned char) subopcode;
    ALUout = ALUMux(instr, RS(theMachineState, 2), 
        RT(theMachineState, 0), 
          '_', '_', '_', '_', '_', '_', ch1, 'M');

    updateNZP(theMachineState, (signed short int) ALUout);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 4) { // JSR
    subopcode = (instr >> 11) & 0x1;
    regInputOut = (signed short int) regInputMux(theMachineState, 0, 2);
    updateNZP(theMachineState, regInputOut);

    if (subopcode == 1) { // JSR IMM
      printf("JSR IMM\n");
      theMachineState -> PC = PCMux(theMachineState, 0, 5);
    } else if (subopcode == 0) { // JSRR
      printf("JSRR R%d\n", (instr >> 6) & 0x7);
      theMachineState -> PC = PCMux(theMachineState, RS(theMachineState, 0), 3);
    }

    theMachineState -> R[7] = regInputOut;
  } else if (opcode == 5) { // Logical
    subopcode = (instr >> 3) & 0x7;

    if (subopcode == 0) { // AND
      printf("AND R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);
      ch1 = 'A';
      ch2 = 'R';
    } else if (subopcode == 1) { // NOT
      printf("NOT R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7);
      ch1 = 'N';
      ch2 = '_';
    } else if (subopcode == 2) { // OR
      printf("OR R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);
      ch1 = 'O';
      ch2 = 'R';
    } else if (subopcode == 3) { // XOR
      printf("XOR R%d, R%d, R%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          instr & 0x7);
      ch1 = 'X';
      ch2 = 'R';
    } else if (subopcode >= 4) { // AND IMM
      temp = instr & 31;
      temp <<= 11;
      temp >>= 11;

      printf("AND R%d, R%d, #%d\n", (instr >> 9) & 0x7, (instr >> 6) & 0x7, 
          temp);
      ch1 = 'A';
      ch2 = '5';
    }

    ALUout = ALUMux(instr, RS(theMachineState, 0), RT(theMachineState, 0), 
          '_', '_', ch1, ch2, '_', '_', '_', 'L');
    regInputOut = regInputMux(theMachineState, ALUout, 0);
    saveResult(theMachineState, regInputOut);
    updateNZP(theMachineState, (signed short int) regInputOut);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 6) { // LDR
    printf("Current opcode is %d and the operation is LDR\n", opcode);
    ALUout = (signed short int) ALUMux(instr, RS(theMachineState, 0), 0, 
          'A', '6', '_', '_', '_', '_', '_', 'A');
    printf("In LDR ALUout is %x\n", ALUout);

    if (ALUout <= 0x1FFF) {
      printf("ERROR: trying to interpret user code as data. Exiting LC4.\n");
      return 1;
    }

    if ((ALUout >= 0x8000) & (ALUout <= 0x9FFF)) {
      printf("ERROR: trying to interpret OS code as data. Exiting LC4.\n");
      return 1;
    }

    if ((ALUout >= 0xA000) & (theMachineState -> PSR >> 15 == 0)) {
      printf("ERROR: trying to access OS data in user mode. Exiting LC4.\n");
      return 1;
    }

    regInputOut = (signed short int) regInputMux(theMachineState, ALUout, 1);
    printf("In LDR regInputOut is %x\n", regInputOut);
    saveResult(theMachineState, regInputOut);
    updateNZP(theMachineState, regInputOut);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 7) { // STR
    printf("Current opcode is %d and the operation is STR\n", opcode);
    ALUout = (signed short int) ALUMux(instr, RS(theMachineState, 0), 0, 
          'A', '6', '_', '_', '_', '_', '_', 'A');

    if (ALUout <= 0x1FFF) {
      printf("ERROR: trying to interpret user code as data. Exiting LC4.\n");
      return 1;
    }

    if ((ALUout >= 0x8000) & (ALUout <= 0x9FFF)) {
      printf("ERROR: trying to interpret OS code as data. Exiting LC4.\n");
      return 1;
    }

    if ((ALUout >= 0xA000) & (theMachineState -> PSR >> 15 == 0)) {
      printf("ERROR: trying to access OS data in user mode. Exiting LC4.\n");
      return 1;
    }

    theMachineState -> memory[ALUout] = RT(theMachineState, 1);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 8) { // RTI
    printf("Current opcode is %d and the operation is RTI\n", opcode);
    theMachineState -> PC = RS(theMachineState, 1);
    theMachineState -> PSR = theMachineState -> PSR & 0x7;
  } else if (opcode == 9) { // CONST
    printf("Current opcode is %d and the operation is CONST\n", opcode);
    ALUout = ALUMux(instr, 0, 0, '_', '_', '_', '_', '_', 'C', '_', 'N');
    regInputOut = (signed short int) regInputMux(theMachineState, ALUout, 0);
    printf("In CONST, regInputOut is %x\n", regInputOut);
    saveResult(theMachineState, regInputOut);
    updateNZP(theMachineState,  regInputOut);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 10) { // Shift/MOD
    printf("Current opcode is %d and the operation is Shift/MOD\n", opcode);
    subopcode = (instr >> 4) & 0x3;
    printf("Current subopcode is %d\n", subopcode);

    /* For reference:
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
         unsigned char ALUMux_CTL) {


}
*/

    if (subopcode < 3) { // Shift
      printf("Current subopcode is %d and the operation is Shift\n", subopcode);
      
      if (subopcode == 0) { // SLL
        printf("Current subopcode is %d and the operation is SLL\n", subopcode);
        ch1 = 'L';
      } else if (subopcode == 1) { // SRA
        printf("Current subopcode is %d and the operation is SRA\n", subopcode);
        ch1 = 'A';
      } else if (subopcode == 2) { // SRL
        printf("Current subopcode is %d and the operation is SRL\n", subopcode);
        ch1 = 'R';
      }
      
      ALUout = ALUMux(instr, RS(theMachineState, 0), RT(theMachineState, 0), 
          '_', '_', '_', '_', ch1, '_', '_', 'S');
      regInputOut = regInputMux(theMachineState, ALUout, 0);
      saveResult(theMachineState, regInputOut);
      updateNZP(theMachineState, (signed short int) regInputOut);
      theMachineState -> PC = PCMux(theMachineState, 0, 1);
    } else if (subopcode == 3) { // MOD
      printf("Current subopcode is %d and the operation is MOD\n", subopcode);
      ALUout = ALUMux(instr, RS(theMachineState, 0), RT(theMachineState, 0), 
          'O', 'R', '_', '_', '_', '_', '_', 'A');
      regInputOut = regInputMux(theMachineState, ALUout, 0);
      saveResult(theMachineState, regInputOut);
      updateNZP(theMachineState, regInputOut);
      theMachineState -> PC = PCMux(theMachineState, 0, 1);
    }

  } else if (opcode == 12) { // JMP
    printf("Current opcode is %d and the operation is JMP\n", opcode);
    subopcode = (instr >> 11) & 0x1;
    printf("Current subopcode is %d\n", subopcode);

    if (subopcode == 0) { // JMPR
      printf("Current subopcode is %d and the operation is JMPR\n", subopcode);
      theMachineState -> PC = PCMux(theMachineState, RS(theMachineState, 0), 3);
    } else if (subopcode == 1) { // JMP IMM
      printf("Current subopcode is %d and the operation is JMP IMM\n", subopcode);
      theMachineState -> PC = PCMux(theMachineState, 0, 2);
    }

  } else if (opcode == 13) { // HICONST
    printf("Current opcode is %d and the operation is HICONST\n", opcode);
    ALUout = ALUMux(instr, RS(theMachineState, 2), 0, '_', '_', '_', '_', '_', 'H', '_', 'N');
    regInputOut = regInputMux(theMachineState, ALUout, 0);
    printf("In HICONST, regInputOut is %x\n", regInputOut);
    saveResult(theMachineState, regInputOut);
    updateNZP(theMachineState, regInputOut);
    theMachineState -> PC = PCMux(theMachineState, 0, 1);
  } else if (opcode == 15) { // TRAP
    printf("Current opcode is %d and the operation is TRAP\n", opcode);
    theMachineState -> R[7] = theMachineState -> PC + 1;
    theMachineState -> PC = PCMux(theMachineState, 0, 4);
    theMachineState -> PSR = theMachineState -> PSR | 0x8000;
  } else { // Error
    printf("Current opcode is %d and it is wrong\n", opcode);
    return 1;
  }

  return 0;
}

// Note that the UpdateMachineState function must perform its action using the helper functions
// declared below which should be used to simulate the operation of portions of the datapath.
//
// Note that all of the control signals passed as arguments to these functions are represented
// as unsigned 8 bit values although none of them use more than 3 bits. You should use the lower
// bits of the fields to store the mandated control bits. Please refer to the LC4 information sheets
// on Canvas for an explanation of the control signals and their role in the datapath.


// Compute the current output of the RS port
unsigned short int RS (MachineState *theMachineState, unsigned char rsMux_CTL) {
  unsigned short int instr;

  instr = theMachineState -> memory[theMachineState -> PC];
  
  if (rsMux_CTL == 0) {
    printf("Index of R is %d\n", (instr >> 6) & 0x7);
    return theMachineState -> R[(instr >> 6) & 0x7];
  } else if (rsMux_CTL == 1) {
    return theMachineState -> R[0x07];
  } else if (rsMux_CTL == 2) {
    return theMachineState -> R[(instr >> 9) & 0x7];
  }

}

// Compute the current output of the RT port
unsigned short int RT (MachineState *theMachineState, unsigned char rtMux_CTL) {
  unsigned short int instr;

  instr = theMachineState -> memory[theMachineState -> PC];
  
  if (rtMux_CTL == 0) {
    return theMachineState -> R[instr & 0x7];
  } else if (rtMux_CTL == 1) {
    return theMachineState -> R[(instr >> 9) & 0x7];
  }

}

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
			   unsigned char ALUMux_CTL) {
  signed short int op2, tempS;
  unsigned short int op2U, tempU;

  printf("in ALUMux\n");

  if (ALUMux_CTL == 'A') { // Arithmetic
    printf("ALU key is %c - arithmetic operations, ALUMux is %c\n", Arith_CTL, ALUMux_CTL);

    if (ArithMux_CTL == 'R') { // RT
      op2 = (signed short int) RTOut;
    } else if (ArithMux_CTL == '5') { // IMM5
      op2 = INSN & 31;
      op2 = op2 << 11;
      op2 = op2 >> 11;
    } else if (ArithMux_CTL == '6') { // IMM6
      op2 = INSN & 63;
      op2 = op2 << 10;
      op2 = op2 >> 10;
    }

    printf("Operand 1 is %d, operand 2 is %d\n", (signed short int) RSOut, op2);

    if (Arith_CTL == 'A') { // Add
      printf("Add\n");
      return (signed short int) RSOut + op2;
    } else if (Arith_CTL == 'M') { // Multiply
      printf("Multiply\n");
      return (signed short int) RSOut * op2;
    } else if (Arith_CTL == 'S') { // Subtract
      printf("Subtract\n");
      return (signed short int) RSOut - op2;
    } else if (Arith_CTL == 'D') { // Divide
      printf("Divide\n");
      
      if (RTOut == 0) {
        return 0;
      }

      return RSOut / (unsigned short int) op2;
    } else if (Arith_CTL == 'O') { // Modulus
      printf("Modulus\n");

      if (RTOut == 0) {
        return 0;
      }

      return ((RSOut % RTOut) + RTOut) % RTOut;
    }

  } else if (ALUMux_CTL == 'L') { // Logic
    printf("ALU key is %c - logic operations, ALUMux is %c\n", LOGIC_CTL, ALUMux_CTL);

    if (LogicMux_CTL == 'R') { // RT
      op2 = RTOut;
    } else if (LogicMux_CTL == '5') { // IMM5
      op2 = INSN & 31;
      op2 = op2 << 11;
      op2 = op2 >> 11;
    }

    printf("Operand 1 is %d, operand 2 is %d\n", RSOut, op2);

    if (LOGIC_CTL == 'A') { // AND
      printf("AND\n");
      return RSOut & op2;
    } else if (LOGIC_CTL == 'N') { // NOT
      printf("NOT\n");
      return ~RSOut;
    } else if (LOGIC_CTL == 'O') { // OR
      printf("OR\n");
      return RSOut | op2;
    } else if (LOGIC_CTL == 'X') { // XOR
      printf("XOR\n");
      return RSOut ^ RTOut;
    }

  } else if (ALUMux_CTL == 'S') { // Shift
    printf("ALU key is %c - shift operations, ALUMux is %c\n", SHIFT_CTL, ALUMux_CTL);
    op2U = INSN & 0xF;
    printf("Operand 1 is %d, operand 2 is %d\n", RSOut, op2U);

    if (SHIFT_CTL == 'L') { // SLL
      printf("SLL\n");
      return RSOut << op2U;
    } else if (SHIFT_CTL == 'A') { // SRA
      printf("SRA\n");
      tempS = (signed short int) RSOut;
      tempS = tempS >> op2U;
      tempU = (unsigned short int) tempS;
      return tempU;
    } else if (SHIFT_CTL == 'R') { // SRL
      printf("SRL\n");
      return RSOut >> op2U;
    }

  } else if (ALUMux_CTL == 'N') { // Constant
    printf("ALU key is %c - constant operations, ALUMux is %c\n", CONST_CTL, ALUMux_CTL);

    if (CONST_CTL == 'C') { // CONST
      printf("CONST\n");
      op2 = INSN & 0x1FF;
      op2 = op2 << 7;
      op2 = op2 >> 7;
      printf("op2 is %d\n", op2);
      return op2;
    } else if (CONST_CTL == 'H') { // HICONST
      printf("HICONST\n");
      return (RSOut & 0xFF) | ((INSN & 0xFF) << 8);
    }

  } else if (ALUMux_CTL == 'M') { // Compare
    printf("ALU key is %d - comparison operations, ALUMux is %c\n", CMP_CTL, ALUMux_CTL);

    if (CMP_CTL == 0) { // CMP
      printf("CMP\n");
      return (signed short int) RSOut - (signed short int) RTOut;
    } else if (CMP_CTL == 1) { // CMPU
      printf("CMPU\n");
      return RSOut - RTOut;
    } else if (CMP_CTL == 2) { // CMPI
      printf("CMPI\n");
      op2 = INSN & 0x7F;      
      printf("op2 is %d\n", op2);
      op2 = op2 << 9;      
      printf("op2 is %d\n", op2);
      op2 = op2 >> 9;
      printf("op2 is %d\n", op2);
      printf("RSOut is %d\n", (signed short int) RSOut);
      printf("result is %d\n", (signed short int) RSOut - op2);
      return (signed short int) RSOut - op2;
    } else if (CMP_CTL == 3) { // CMPIU
      printf("CMPIU\n");
      return RSOut - (INSN & 0x7F);
    }

  }

}

// Compute the current output of the regInputMux
unsigned short int regInputMux (MachineState *theMachineState,
				unsigned short int ALUMuxOut, // current ALUMux output
				unsigned char regInputMux_CTL) {

  if (regInputMux_CTL == 0) {
    return ALUMuxOut;
  } else if (regInputMux_CTL == 1) {
    return theMachineState -> memory[ALUMuxOut];
  } else if (regInputMux_CTL == 2) {
    return theMachineState -> PC + 1;
  }

}

// Compute the current output of the PCMux
unsigned short int PCMux (MachineState *theMachineState,
			  unsigned short int RSOut,
			  unsigned char PCMux_CTL) {
  unsigned short int instr, next;
  signed short int result;

  instr = theMachineState -> memory[theMachineState -> PC];
  next = theMachineState -> PC + 1;
  if (PCMux_CTL == 0) {

    if (passedNZP(theMachineState) > 0) {
      result = (instr & 0x1FF);
      result = result << 7;
      result = result >> 7;
      return next + result;
    } else {
      return next;
    }

  } else if (PCMux_CTL == 1) {
    return next;
  } else if (PCMux_CTL == 2) {
    result = (instr & 0x7FF);
    result = result << 5;
    result = result >> 5;
    return next + result;
  } else if (PCMux_CTL == 3) {
    return RSOut;
  } else if (PCMux_CTL == 4) {
    printf("In PCMux exiting the program command, the value is %x\n", 0x8000 | (instr & 0xFF));
    return 0x8000 | (instr & 0xFF);
  } else if (PCMux_CTL == 5) {
    result = instr & 0x7FF;
    result = result << 4;
    return ((theMachineState -> PC & 0x8000) | result);
  }

}

/*
int main() {
  printf("started the program\n");
  MachineState *state;

  state = malloc(sizeof(*state));
  printf("initialized state\n");
  Reset(state);
  printf("executed the reset\n");
  state -> memory[state -> PC] = 0x0023;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0823;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0C23;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0A23;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0423;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0623;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0223;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x0E23;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x1587;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x1508;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x1510;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x1519;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x1534;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x2005;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x2085;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x2345;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x23A5;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x4F67;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x4567;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x5587;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x5508;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x5510;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x5518;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x5534;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x6789;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x789A;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x89AB;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x9ABC;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xABCD;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xABDD;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xABED;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xABFD;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xCDEF;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xC0EF;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xDEF0;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0xF012;
  UpdateMachineState(state);
  state -> memory[state -> PC] = 0x3456;
  printf("%d\n", UpdateMachineState(state));
  state -> memory[state -> PC] = 0xBCDE;
  printf("%d\n", UpdateMachineState(state));
  state -> memory[state -> PC] = 0xEF01;
  printf("%d\n", UpdateMachineState(state));
  printf("RS(EF01, 0): %d\n", RS(state, 0));
  printf("RS(EF01, 1): %d\n", RS(state, 1));
  printf("RS(EF01, 2): %d\n", RS(state, 2));
  printf("RT(EF01, 0): %d\n", RT(state, 0));
  printf("RT(EF01, 1): %d\n", RT(state, 1));
  printf("regInputMux(EF01, state -> PC, 0): %x\n", regInputMux(state, state -> PC, 0));
  printf("regInputMux(EF01, state -> PC, 1): %x\n", regInputMux(state, state -> PC, 1));
  printf("regInputMux(EF01, state -> PC, 2): %x\n", regInputMux(state, state -> PC, 2));
  printf("passedNZP(EF01): %d\n", passedNZP(state));
  state -> memory[state -> PC] = 0xEB01;
  printf("passedNZP(EB01): %d\n", passedNZP(state));
  printf("PCMux(EB01, 123, 0): %x\n", PCMux(state, 123, 0));
  state -> memory[state -> PC] = 0xEE01;
  printf("PCMux(EE01, 123, 0): %x\n", PCMux(state, 123, 0));
  printf("PCMux(EE01, 123, 1): %x\n", PCMux(state, 123, 1));
  state -> memory[state -> PC] = 0xE801;
  printf("PCMux(E801, 123, 2): %x\n", PCMux(state, 123, 2));
  printf("PCMux(E801, 123, 3): %d\n", PCMux(state, 123, 3));
  state -> memory[state -> PC] = 0x8000;
  printf("PCMux(8000, 123, 4): %x\n", PCMux(state, 123, 4));
  printf("PCMux(8000, 123, 5): %x\n", PCMux(state, 123, 5));
  free(state);
  return 0;
}
*/