#include "fake6502.h"

// Blue6502's voids // i didn't change much of Fake6502's code, it's working good

//6502 defines
#define UNDOCUMENTED //when this is defined, undocumented opcodes are handled.
//otherwise, they're simply treated as NOPs.

//flag modifier macros
#define setcarry() cpustatus |= FLAG_CARRY
#define clearcarry() cpustatus &= (~FLAG_CARRY)
#define setzero() cpustatus |= FLAG_ZERO
#define clearzero() cpustatus &= (~FLAG_ZERO)
#define setinterrupt() cpustatus |= FLAG_INTERRUPT
#define clearinterrupt() cpustatus &= (~FLAG_INTERRUPT)
#define setdecimal() cpustatus |= FLAG_DECIMAL
#define cleardecimal() cpustatus &= (~FLAG_DECIMAL)
#define setoverflow() cpustatus |= FLAG_OVERFLOW
#define clearoverflow() cpustatus &= (~FLAG_OVERFLOW)
#define setsign() cpustatus |= FLAG_SIGN
#define clearsign() cpustatus &= (~FLAG_SIGN)

#define  saveaccum( n)  ACCUMULATOR = ((uint8_t)(n) & 0x00FF);

//flag calculation macros
#define zerocalc( n)  if ((n) & 0x00FF) clearzero();  else setzero();

#define  signcalc( n)  {    if ((n) & 0x0080) setsign();    else clearsign();  }
#define carrycalc( n)  {    if ((n) & 0xFF00) setcarry();    else clearcarry();  }
#define  overflowcalc( n,  m,  o)   { if (((n) ^ (m)) & ((n) ^ (o)) & 0x0080) setoverflow();    else clearoverflow();  }

const uint8_t ticktable[256] = {
  /*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |     */
  /* 0 */      7,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    4,    4,    6,    6,  /* 0 */
  /* 1 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 1 */
  /* 2 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    4,    4,    6,    6,  /* 2 */
  /* 3 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 3 */
  /* 4 */      6,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    3,    4,    6,    6,  /* 4 */
  /* 5 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 5 */
  /* 6 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    5,    4,    6,    6,  /* 6 */
  /* 7 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 7 */
  /* 8 */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* 8 */
  /* 9 */      2,    6,    2,    6,    4,    4,    4,    4,    2,    5,    2,    5,    5,    5,    5,    5,  /* 9 */
  /* A */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* A */
  /* B */      2,    5,    2,    5,    4,    4,    4,    4,    2,    4,    2,    4,    4,    4,    4,    4,  /* B */
  /* C */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* C */
  /* D */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* D */
  /* E */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* E */
  /* F */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7   /* F */
};

uint8_t FLAG_CARRY = 0x01;
uint8_t  FLAG_ZERO = 0x02;
uint8_t  FLAG_INTERRUPT = 0x04;
uint8_t  FLAG_DECIMAL = 0x08;
uint8_t  FLAG_BREAK = 0x10;
uint8_t  FLAG_CONSTANT = 0x20;
uint8_t  FLAG_OVERFLOW = 0x40;
uint8_t  FLAG_SIGN = 0x80;

uint16_t  BASE_STACK = 0x100;

uint8_t  ACCUMULATOR = 0;
uint8_t X_REGISTER = 0;
uint8_t Y_REGISTER = 0;
uint8_t  STACK_POINTER = 0xFD;
uint8_t cpustatus;

//helper variables
uint32_t instructions = 0; //keep track of total instructions executed
int32_t clockticks6502 = 0, clockgoal6502 = 0;
uint16_t oldPROGRAM_COUNTER, ea, reladdr, value6502, result6502;
uint8_t opcode, oldcpustatus, useaccum;
uint16_t PROGRAM_COUNTER;
// end 0f Blue6502 defines and variable declarations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// globaling locals (make local variables global, to minimize push to stack time
uint16_t temp16;
uint16_t startpage;
uint16_t eahelp, eahelp2;




void reset6502() {

  PROGRAM_COUNTER = 0x0300;  // player is copied to memory at 0x300
  //PROGRAM_COUNTER = (uint16_t)read6502(0xFFFC) | ((uint16_t)read6502(0xFFFD) << 8); // reset vector not used
  ACCUMULATOR = 0;
  X_REGISTER = 0;
  Y_REGISTER = 0;
  STACK_POINTER = 0xFD;
  cpustatus |= FLAG_CONSTANT;

  instructions =0;
  //CIA_DC04 = 0;
  //CIA_DC05 = 0;
}




//a few general functions used by various other functions
inline void push16(uint16_t pushval) {
  write6502(BASE_STACK + STACK_POINTER, (pushval >> 8) & 0xFF);
  write6502(BASE_STACK + ((STACK_POINTER - 1) & 0xFF), pushval & 0xFF);
  STACK_POINTER -= 2;
}

inline void push8(uint8_t pushval) {
  write6502(BASE_STACK + STACK_POINTER--, pushval);
}

inline uint16_t pull16() {

  temp16 = read6502(BASE_STACK + ((STACK_POINTER + 1) & 0xFF)) | (read6502(BASE_STACK + ((STACK_POINTER + 2) & 0xFF)) << 8);
  STACK_POINTER += 2;
  return (temp16);
}

inline uint8_t pull8() {
  return (read6502(BASE_STACK + ++STACK_POINTER));
}



//addressing mode functions, calculates effective addresses
inline void imp() { //implied
}

inline void acc() { //accumulator
  useaccum = 1;
}

inline void imm() { //immediate
  ea = PROGRAM_COUNTER++;
}

inline void zp() { //zero-page
  ea = read6502(PROGRAM_COUNTER++);
}

inline void zpx() { //zero-page,X
  ea = (read6502(PROGRAM_COUNTER++) + X_REGISTER) & 0xFF; //zero-page wraparound
}

inline void zpy() { //zero-page,Y
  ea = (read6502(PROGRAM_COUNTER++) + Y_REGISTER) & 0xFF; //zero-page wraparound
}

inline void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
  reladdr = read6502(PROGRAM_COUNTER++);
  if (reladdr & 0x80) reladdr |= 0xFF00;
}

inline void abso() { //absolute
  ea = read6502(PROGRAM_COUNTER) | (read6502(PROGRAM_COUNTER + 1) << 8);
  PROGRAM_COUNTER += 2;
}

inline void absx() { //absolute,X

  ea = (read6502(PROGRAM_COUNTER) | (read6502(PROGRAM_COUNTER + 1) << 8));
  startpage = ea & 0xFF00;
  ea += X_REGISTER;

  PROGRAM_COUNTER += 2;
}

inline void absy() { //absolute,Y

  ea = (read6502(PROGRAM_COUNTER) | (read6502(PROGRAM_COUNTER + 1) << 8));
  startpage = ea & 0xFF00;
  ea += Y_REGISTER;

  PROGRAM_COUNTER += 2;
}

inline void ind() { //indirect

  eahelp = read6502(PROGRAM_COUNTER) | (read6502(PROGRAM_COUNTER + 1) << 8);
  eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
  ea = read6502(eahelp) | (read6502(eahelp2) << 8);
  PROGRAM_COUNTER += 2;
}

inline void indx() { // (indirect,X)

  eahelp = ((read6502(PROGRAM_COUNTER++) + X_REGISTER) & 0xFF); //zero-page wraparound for table pointer
  ea = read6502(eahelp & 0x00FF) | (read6502((eahelp + 1) & 0x00FF) << 8);
}

inline void indy() { // (indirect),Y

  eahelp = read6502(PROGRAM_COUNTER++);
  eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
  ea = read6502(eahelp) | (read6502(eahelp2) << 8);
  startpage = ea & 0xFF00;
  ea += Y_REGISTER;

}

uint16_t getvalue6502() {
  if (useaccum) return (ACCUMULATOR);
  else return (read6502(ea));
}

uint16_t getvalue16() {
  return (read6502(ea) | (read6502(ea + 1) << 8));
}

inline void putvalue(uint16_t saveval) {
  if (useaccum) ACCUMULATOR = (saveval & 0x00FF);
  else write6502(ea, (saveval & 0x00FF));
}


//instruction handler functions
inline void adc() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR + value6502 + (cpustatus & FLAG_CARRY);

  carrycalc(result6502);
  zerocalc(result6502);
  overflowcalc(result6502, ACCUMULATOR, value6502);
  signcalc(result6502);
  saveaccum(result6502);
}


inline void op_and() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR & value6502;

  zerocalc(result6502);
  signcalc(result6502);

  saveaccum(result6502);
}

inline void asl() {
  value6502 = getvalue6502();
  result6502 = value6502 << 1;

  carrycalc(result6502);
  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void bcc() {
  if ((cpustatus & FLAG_CARRY) == 0) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void bcs() {
  if ((cpustatus & FLAG_CARRY) == FLAG_CARRY) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void beq() {
  if ((cpustatus & FLAG_ZERO) == FLAG_ZERO) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void op_bit() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR & value6502;

  zerocalc(result6502);
  cpustatus = (cpustatus & 0x3F) | (value6502 & 0xC0);
}

inline void bmi() {
  if ((cpustatus & FLAG_SIGN) == FLAG_SIGN) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void bne() {
  if ((cpustatus & FLAG_ZERO) == 0) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void bpl() {
  if ((cpustatus & FLAG_SIGN) == 0) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void brk() {
  PROGRAM_COUNTER++;
  push16(PROGRAM_COUNTER); //push next instruction address onto stack
  push8(cpustatus | FLAG_BREAK); //push CPU cpustatus to stack
  setinterrupt(); //set interrupt flag
  PROGRAM_COUNTER = read6502(0xFFFE) | (read6502(0xFFFF) << 8);
}

inline void bvc() {
  if ((cpustatus & FLAG_OVERFLOW) == 0) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void bvs() {
  if ((cpustatus & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
    oldPROGRAM_COUNTER = PROGRAM_COUNTER;
    PROGRAM_COUNTER += reladdr;
    if ((oldPROGRAM_COUNTER & 0xFF00) != (PROGRAM_COUNTER & 0xFF00)) clockticks6502 += 2; //check if jump crossed a page boundary
    else clockticks6502++;
  }
}

inline void clc() {
  clearcarry();
}

inline void cld() {
  cleardecimal();
}

inline void cli() {
  clearinterrupt();
}

inline void clv() {
  clearoverflow();
}

inline void cmp() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR - value6502;

  if (ACCUMULATOR >= (value6502 & 0x00FF)) setcarry();
  else clearcarry();
  if (ACCUMULATOR == (value6502 & 0x00FF)) setzero();
  else clearzero();
  signcalc(result6502);
}

inline void cpx() {
  value6502 = getvalue6502();
  result6502 = X_REGISTER - value6502;

  if (X_REGISTER >= (value6502 & 0x00FF)) setcarry();
  else clearcarry();
  if (X_REGISTER == (value6502 & 0x00FF)) setzero();
  else clearzero();
  signcalc(result6502);
}

inline void cpy() {
  value6502 = getvalue6502();
  result6502 = Y_REGISTER - value6502;

  if (Y_REGISTER >= (value6502 & 0x00FF)) setcarry();
  else clearcarry();
  if (Y_REGISTER == (value6502 & 0x00FF)) setzero();
  else clearzero();
  signcalc(result6502);
}

inline void dec() {
  value6502 = getvalue6502();
  result6502 = value6502 - 1;

  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void dex() {
  X_REGISTER--;

  zerocalc(X_REGISTER);
  signcalc(X_REGISTER);
}

inline void dey() {
  Y_REGISTER--;

  zerocalc(Y_REGISTER);
  signcalc(Y_REGISTER);
}

inline void eor() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR ^ value6502;

  zerocalc(result6502);
  signcalc(result6502);

  saveaccum(result6502);
}

inline void inc() {
  value6502 = getvalue6502();
  result6502 = value6502 + 1;

  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void inx() {
  X_REGISTER++;

  zerocalc(X_REGISTER);
  signcalc(X_REGISTER);
}

inline void iny() {
  Y_REGISTER++;

  zerocalc(Y_REGISTER);
  signcalc(Y_REGISTER);
}

inline void jmp() {
  PROGRAM_COUNTER = ea;
}

inline void jsr() {
  push16(PROGRAM_COUNTER - 1);
  PROGRAM_COUNTER = ea;
}

inline void lda() {
  value6502 = getvalue6502();
  ACCUMULATOR = (value6502 & 0x00FF);

  zerocalc(ACCUMULATOR);
  signcalc(ACCUMULATOR);
}

inline void ldx() {
  value6502 = getvalue6502();
  X_REGISTER = (value6502 & 0x00FF);

  zerocalc(X_REGISTER);
  signcalc(X_REGISTER);
}

inline void ldy() {
  value6502 = getvalue6502();
  Y_REGISTER = (value6502 & 0x00FF);

  zerocalc(Y_REGISTER);
  signcalc(Y_REGISTER);
}

inline void lsr() {
  value6502 = getvalue6502();
  result6502 = value6502 >> 1;

  if (value6502 & 1) setcarry();
  else clearcarry();
  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void nop() {
}

inline void ora() {
  value6502 = getvalue6502();
  result6502 = ACCUMULATOR | value6502;

  zerocalc(result6502);
  signcalc(result6502);

  saveaccum(result6502);
}

inline void pha() {
  push8(ACCUMULATOR);
}

inline void php() {
  push8(cpustatus | FLAG_BREAK);
}

inline void pla() {
  ACCUMULATOR = pull8();

  zerocalc(ACCUMULATOR);
  signcalc(ACCUMULATOR);
}

inline void plp() {
  cpustatus = pull8() | FLAG_CONSTANT;
}

inline void rol() {
  value6502 = getvalue6502();
  result6502 = (value6502 << 1) | (cpustatus & FLAG_CARRY);

  carrycalc(result6502);
  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void ror() {
  value6502 = getvalue6502();
  result6502 = (value6502 >> 1) | ((cpustatus & FLAG_CARRY) << 7);

  if (value6502 & 1) setcarry();
  else clearcarry();
  zerocalc(result6502);
  signcalc(result6502);

  putvalue(result6502);
}

inline void rti() {
  cpustatus = pull8();
  value6502 = pull16();
  PROGRAM_COUNTER = value6502;
}

inline void rts() {
  value6502 = pull16();
  PROGRAM_COUNTER = value6502 + 1;
}

inline void sbc() {
  value6502 = getvalue6502() ^ 0x00FF;
  result6502 = ACCUMULATOR + value6502 + (cpustatus & FLAG_CARRY);

  carrycalc(result6502);
  zerocalc(result6502);
  overflowcalc(result6502, ACCUMULATOR, value6502);
  signcalc(result6502);



  saveaccum(result6502);
}


inline void sec() {
  setcarry();
}

inline void sed() {
  setdecimal();
}

inline void sei() {
  setinterrupt();
}

inline void sta() {
  putvalue(ACCUMULATOR);
}

inline void stx() {
  putvalue(X_REGISTER);
}

inline void sty() {
  putvalue(Y_REGISTER);
}

inline void tax() {
  X_REGISTER = ACCUMULATOR;

  zerocalc(X_REGISTER);
  signcalc(X_REGISTER);
}

inline void tay() {
  Y_REGISTER = ACCUMULATOR;

  zerocalc(Y_REGISTER);
  signcalc(Y_REGISTER);
}

inline void tsx() {
  X_REGISTER = STACK_POINTER;

  zerocalc(X_REGISTER);
  signcalc(X_REGISTER);
}

inline void txa() {
  ACCUMULATOR = X_REGISTER;

  zerocalc(ACCUMULATOR);
  signcalc(ACCUMULATOR);
}

inline void txs() {
  STACK_POINTER = X_REGISTER;
}

inline void tya() {
  ACCUMULATOR = Y_REGISTER;

  zerocalc(ACCUMULATOR);
  signcalc(ACCUMULATOR);
}

//undocumented instructions
#ifdef UNDOCUMENTED
inline void lax() {
  lda();
  ldx();
}

inline void sax() {
  sta();
  stx();
  putvalue(ACCUMULATOR & X_REGISTER);
}

inline void dcp() {
  dec();
  cmp();
}

inline void isb() {
  inc();
  sbc();
}

inline void slo() {
  asl();
  ora();
}

inline void rla() {
  rol();
  op_and();
}

inline void sre() {
  lsr();
  eor();
}

inline void rra() {
  ror();
  adc();
}
#else
#define lax nop
#define sax nop
#define dcp nop
#define isb nop
#define slo nop
#define rla nop
#define sre nop
#define rra nop
#endif


void exec6502() {
  opcode = read6502(PROGRAM_COUNTER++);

  cpustatus |= FLAG_CONSTANT;

  useaccum = 0;

  switch (opcode) {
    case 0x0: // _brk
      imp();  // adr mode
      brk();  // cmd
      break;
    case 0x1: 
      indx();
      ora();
      break;
    case 0x5: 
      zp();
      ora();
      break;
    case 0x6:
      zp();
      asl();
      break;
    case 0x8:
      imp();
      php();
      break;
    case 0x9:
      imm();
      ora();
      break;
    case 0xA:
      acc();
      asl();
      break;
    case 0xD:
      abso();
      ora();
      break;
    case 0xE:
      abso();
      asl();
      break;
    case 0x10:
      rel();
      bpl();
      break;
    case 0x11:
      indy();
      ora();
      break;
    case 0x15:
      zpx();
      ora();
      break;
    case 0x16:
      zpx();
      asl();
      break;
    case 0x18:
      imp();
      clc();
      break;
    case 0x19:
      absy();
      ora();
      break;
    case 0x1D:
      absx();
      ora();
      break;
    case 0x1E:
      absx();
      asl();
      break;
    case 0x20:
      abso();
      jsr();
      break;
    case 0x21:
      indx();
      op_and();
      break;
    case 0x24:
      zp();
      op_bit();
      break;
    case 0x25:
      zp();
      op_and();
      break;
    case 0x26:
      zp();
      rol();
      break;
    case 0x28:
      imp();
      plp();
      break;
    case 0x29:
      imm();
      op_and();
      break;
    case 0x2A:
      acc();
      rol();
      break;
    case 0x2C:
      abso();
      op_bit();
      break;
    case 0x2D:
      abso();
      op_and();
      break;
    case 0x2E:
      abso();
      rol();
      break;
    case 0x30:
      rel();
      bmi();
      break;
    case 0x31:
      indy();
      op_and();
      break;
    case 0x35:
      zpx();
      op_and();
      break;
    case 0x36:
      zpx();
      rol();
      break;
    case 0x38:
      imp();
      sec();
      break;
    case 0x39:
      absy();
      op_and();
      break;
    case 0x3D:
      absx();
      op_and();
      break;
    case 0x3E:
      absx();
      rol();
      break;
    case 0x40:
      imp();
      rti();
      break;
    case 0x41:
      indx();
      eor();
      break;
    case 0x45:
      zp();
      eor();
      break;
    case 0x46:
      zp();
      lsr();
      break;
    case 0x48:
      imp();
      pha();
      break;
    case 0x49:
      imm();
      eor();
      break;
    case 0x4A:
      acc();
      lsr();
      break;
    case 0x4C:
      abso();
      jmp();
      break;
    case 0x4D:
      abso();
      eor();
      break;
    case 0x4E:
      abso();
      lsr();
      break;
    case 0x50:
      rel();
      bvc();
      break;
    case 0x51:
      indy();
      eor();
      break;
    case 0x55:
      zpx();
      eor();
      break;
    case 0x56:
      zpx();
      lsr();
      break;
    case 0x58:
      imp();
      cli();
      break;
    case 0x59:
      absy();
      eor();
      break;
    case 0x5D:
      absx();
      eor();
      break;
    case 0x5E:
      absx();
      lsr();
      break;
    case 0x60:
      imp();
      rts();
      break;
    case 0x61:
      indx();
      adc();
      break;
    case 0x65:
      zp();
      adc();
      break;
    case 0x66:
      zp();
      ror();
      break;
    case 0x68:
      imp();
      pla();
      break;
    case 0x69:
      imm();
      adc();
      break;
    case 0x6A:
      acc();
      ror();
      break;
    case 0x6C:
      ind();
      jmp();
      break;
    case 0x6D:
      abso();
      adc();
      break;
    case 0x6E:
      abso();
      ror();
      break;
    case 0x70:
      rel();
      bvs();
      break;
    case 0x71:
      indy();
      adc();
      break;
    case 0x75:
      zpx();
      adc();
      break;
    case 0x76:
      zpx();
      ror();
      break;
    case 0x78:
      imp();
      sei();
      break;
    case 0x79:
      absy();
      adc();
      break;
    case 0x7D:
      absx();
      adc();
      break;
    case 0x7E:
      absx();
      ror();
      break;
    case 0x81:
      indx();
      sta();
      break;
    case 0x84:
      zp();
      sty();
      break;
    case 0x85:
      zp();
      sta();
      break;
    case 0x86:
      zp();
      stx();
      break;
    case 0x88:
      imp();
      dey();
      break;
    case 0x8A:
      imp();
      txa();
      break;
    case 0x8C:
      abso();
      sty();
      break;
    case 0x8D:
      abso();
      sta();
      break;
    case 0x8E:
      abso();
      stx();
      break;
    case 0x90:
      rel();
      bcc();
      break;
    case 0x91:
      indy();
      sta();
      break;
    case 0x94:
      zpx();
      sty();
      break;
    case 0x95:
      zpx();
      sta();
      break;
    case 0x96:
      zpy();
      stx();
      break;
    case 0x98:
      imp();
      tya();
      break;
    case 0x99:
      absy();
      sta();
      break;
    case 0x9A:
      imp();
      txs();
      break;
    case 0x9D:
      absx();
      sta();
      break;
    case 0xA0:
      imm();
      ldy();
      break;
    case 0xA1:
      indx();
      lda();
      break;
    case 0xA2:
      imm();
      ldx();
      break;
    case 0xA4:
      zp();
      ldy();
      break;
    case 0xA5:
      zp();
      lda();
      break;
    case 0xA6:
      zp();
      ldx();
      break;
    case 0xA8:
      imp();
      tay();
      break;
    case 0xA9:
      imm();
      lda();
      break;
    case 0xAA:
      imp();
      tax();
      break;
    case 0xAC:
      abso();
      ldy();
      break;
    case 0xAD:
      abso();
      lda();
      break;
    case 0xAE:
      abso();
      ldx();
      break;
    case 0xB0:
      rel();
      bcs();
      break;
    case 0xB1:
      indy();
      lda();
      break;
    case 0xB4:
      zpx();
      ldy();
      break;
    case 0xB5:
      zpx();
      lda();
      break;
    case 0xB6:
      zpy();
      ldx();
      break;
    case 0xB8:
      imp();
      clv();
      break;
    case 0xB9:
      absy();
      lda();
      break;
    case 0xBA:
      imp();
      tsx();
      break;
    case 0xBC:
      absx();
      ldy();
      break;
    case 0xBD:
      absx();
      lda();
      break;
    case 0xBE:
      absy();
      ldx();
      break;
    case 0xC0:
      imm();
      cpy();
      break;
    case 0xC1:
      indx();
      cmp();
      break;
    case 0xC4:
      zp();
      cpy();
      break;
    case 0xC5:
      zp();
      cmp();
      break;
    case 0xC6:
      zp();
      dec();
      break;
    case 0xC8:
      imp();
      iny();
      break;
    case 0xC9:
      imm();
      cmp();
      break;
    case 0xCA:
      imp();
      dex();
      break;
    case 0xCC:
      abso();
      cpy();
      break;
    case 0xCD:
      abso();
      cmp();
      break;
    case 0xCE:
      abso();
      dec();
      break;
    case 0xD0:
      rel();
      bne();
      break;
    case 0xD1:
      indy();
      cmp();
      break;
    case 0xD5:
      zpx();
      cmp();
      break;
    case 0xD6:
      zpx();
      dec();
      break;
    case 0xD8:
      imp();
      cld();
      break;
    case 0xD9:
      absy();
      cmp();
      break;
    case 0xDD:
      absx();
      cmp();
      break;
    case 0xDE:
      absx();
      dec();
      break;
    case 0xE0:
      imm();
      cpx();
      break;
    case 0xE1:
      indx();
      sbc();
      break;
    case 0xE4:
      zp();
      cpx();
      break;
    case 0xE5:
      zp();
      sbc();
      break;
    case 0xE6:
      zp();
      inc();
      break;
    case 0xE8:
      imp();
      inx();
      break;
    case 0xE9:
      imm();
      sbc();
      break;
    case 0xEB:
      imm();
      sbc();
      break;
    case 0xEC:
      abso();
      cpx();
      break;
    case 0xED:
      abso();
      sbc();
      break;
    case 0xEE:
      abso();
      inc();
      break;
    case 0xF0:
      rel();
      beq();
      break;
    case 0xF1:
      indy();
      sbc();
      break;
    case 0xF5:
      zpx();
      sbc();
      break;
    case 0xF6:
      zpx();
      inc();
      break;
    case 0xF8:
      imp();
      sed();
      break;
    case 0xF9:
      absy();
      sbc();
      break;
    case 0xFD:
      absx();
      sbc();
      break;
    case 0xFE:
      absx();
      inc();
      break;
  }

  instructions++;

}


inline uint16_t getpc() {
  return (PROGRAM_COUNTER);
}

inline uint8_t getop() {
  return (opcode);
}

// end of Blue6502's voids