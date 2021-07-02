#include <iostream>
#include <fstream>
#include <bitset>
#include <map>
#include <cassert>
#include <functional>

using namespace std;

typedef unsigned int u32;

unsigned char M[1048576] = {0};
u32 x[32] = {0};
u32 PC = 0;
u32 NPC = 0;


void initialize();

void IF();

void ID();

void EX();

void MEM();

void WB();

u32 IR;
u32 A,B;
u32 Imm;
u32 ALUOutput;

u32 opcode;
u32 rd;
u32 func3;
u32 rs1;
u32 rs2;
u32 func7;

enum Operation {
    LUI,
    AUIPC,
    JAL,
    JALR,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    BEQ,
    BNE,
    BLT, BGE,
    BLTU,
    BGEU,
    SB,
    SH,
    SW,
    ADD,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    OR,
    AND,
    SUB,
    SRA
};

Operation operation;



int main() {
   freopen("../sample.data", "r", stdin);
//    freopen("../sjmout.txt", "w", stdout);
    initialize();
    while (true) {
        IF();
        ID();
        EX();
        MEM();
        WB();
    }
}



u32 sext(int code, int len) {
    return code << 32 - len >> 32 - len;
}

u32 getbit(u32 code, int ptr) {
    return code & 1 << ptr;
}

u32 getbit(u32 code, int high, int low) {
    return code >> low << (31 - high + low) >> (31 - high);
}


void initialize() {
    u32 memptr = 0;
    string input;
    while (cin >> input) {
        if (input[0] == '@')
            memptr = stol(input.substr(1), 0, 16);
        else
            M[memptr++] = stol(input, 0, 16);
    }
}

void IF() {
    PC = NPC;
    IR = M[PC] | M[PC + 1] << 8 | M[PC + 2] << 16 | M[PC + 3] << 24;
    NPC = PC + 4;
}

void ID() {
    if (IR == 0x0ff00513) {
        cout << dec << (x[10] & 0b11111111u) << endl;
        exit(0);
    }

    A = x[rs1];
    B = x[rs2];

    static const map<u32, char> opcodeTypeMap = {{0b0110111u, 'U'},
                                                 {0b0010111u, 'u'},
                                                 {0b1101111u, 'J'},
                                                 {0b1100111u, 'j'},
                                                 {0b1100011u, 'B'},
                                                 {0b0000011u, 'I'},
                                                 {0b0100011u, 'S'},
                                                 {0b0010011u, 'i'},
                                                 {0b0110011u, 'R'},
    };
    opcode = IR & 0b1111111u;
    rd = IR >> 7 & 0b11111u;
    func3 = IR >> 12 & 0b111u;
    rs1 = IR >> 15 & 0b11111u;
    rs2 = IR >> 20 & 0b11111u;
    func7 = IR >> 25;
    char type = opcodeTypeMap.at(opcode);
    switch (type) {
        case 'U': {
            operation = LUI;
            Imm = IR >> 12 << 12;
            break;
        }
        case 'u': {
            operation = AUIPC;
            Imm = IR >> 12 << 12;
            break;
        }
        case 'J': {
            operation = JAL;
            u32 imm31_12 = IR >> 12;
            Imm = sext((getbit(imm31_12, 19) | getbit(imm31_12, 18, 9) >> 9 | getbit(imm31_12, 8) << 2 |
                        getbit(imm31_12, 7, 0) << 11) << 1, 21);
            break;
        }
        case 'j': {
            operation = JALR;
            Imm = sext(IR >> 20, 12);
            break;
        }
        case 'I': {
            static const map<u32, Operation> Ifunc3Map = {{0b000u, LB},
                                                          {0b001u, LH},
                                                          {0b010u, LW},
                                                          {0b100u, LBU},
                                                          {0b101u, LHU}};
            operation = Ifunc3Map.at(func3);
            Imm = sext(IR >> 20, 12);
            break;
        }

        case 'i': {
            static const map<u32, Operation> ifunc3Map = {{0b000u, ADDI},
                                                          {0b010u, SLTI},
                                                          {0b011u, SLTIU},
                                                          {0b100u, XORI},
                                                          {0b110u, ORI},
                                                          {0b111u, ANDI},
                                                          {0b001u, SLLI},
                                                          {0b101u, SRLI}};
            operation = ifunc3Map.at(func3);
            if (func3 == 0b101u && func7 == 0b0000000u) operation = SRAI;
            Imm =  (operation == SLLI || operation == SRLI || operation == SRAI) ? rs2 : sext(IR >> 20, 12);
            break;
        }
        case 'B': {
            static const map<u32, Operation> Bfunc3Map = {{0b000u, BEQ},
                                                          {0b001u, BNE},
                                                          {0b100u, BLT},
                                                          {0b101u, BGE},
                                                          {0b110u, BLTU},
                                                          {0b111u, BGEU}};
            operation = Bfunc3Map.at(func3);
            break;
        }
        case 'S': {
            static const map<u32, Operation> Sfunc3Map = {{0b000u, SB},
                                                          {0b001u, SH},
                                                          {0b010u, SW}};
            operation = Sfunc3Map.at(func3);
            break;
        }
        case 'R': {
            switch (func7) {
                case 0b0000000u: {
                    static const map<u32, Operation> R71func3Map = {{0b000u, ADD},
                                                                    {0b001u, SLL},
                                                                    {0b010u, SLT},
                                                                    {0b011u, SLTU},
                                                                    {0b100u, XOR},
                                                                    {0b101u, SRL},
                                                                    {0b110u, OR}};
                    operation = R71func3Map.at(func3);
                    break;
                }
                case 0b0100000u: {
                    static const map<u32, Operation> R72func3Map = {{0b000u, SUB},
                                                                    {0b101u, SRA}};
                    operation = R72func3Map.at(func3);
                    break;
                }
            }
            break;
        }
    }
}

void EX(){
    
    static const map<Operation, function<void()>> operMap
            =
            {
                    {LUI,   []() { x[rd] = Imm; }},
                    {AUIPC, []() { x[rd] = PC + Imm; }},
                    {JAL,   []() {
                        if (rd != 0)
                            x[rd] = PC + 4;//注意这个+4,是指下一条指令的地址，这是是执行完语句再改，考虑与流水的关系，等等。
                        NPC += Imm - 4;
                    }},
                    {JALR,  []() {
                        int t = PC + 4;
                        NPC = ((A + Imm) & ~1);
                        if (rd != 0)x[rd] = t;
                    }},
                    {LB,    []() {
                        int addr = A + Imm;
                        x[rd] = sext(M[addr], 8);
                    }},
                    {LH,    []() {
                        int addr = A + Imm;
                        x[rd] = sext(M[addr] | M[addr + 1] << 8, 16);
                    }},
                    {LW,    []() {
                        int addr = A + Imm;
                        x[rd] = M[addr] | M[addr + 1] << 8 | M[addr + 2] << 16 | M[addr + 3] << 24;

                    }},
                    {LBU,   []() {
                        int addr = A + Imm;
                        x[rd] = M[addr];

                    }},
                    {LHU,   []() {
                        int addr = A + Imm;
                        x[rd] = M[addr] | M[addr + 1] << 8;

                    }},
                    {ADDI,  []() {
                        x[rd] = A + Imm;
                    }},
                    {SLTI,  []() {
                        x[rd] = (int(A) < int(Imm));
                    }},
                    {SLTIU, []() {
                        x[rd] = (A < Imm);
                    }},
                    {XORI,  []() {
                        x[rd] = A ^ Imm;
                    }},
                    {ORI,   []() {
                        x[rd] = A | Imm;
                    }},
                    {ANDI,  []() {
                        x[rd] = A & Imm;
                    }},
                    {SLLI,  []() {
                        x[rd] = A << Imm;
                    }},
                    {SRLI,  []() {
                        x[rd] = (A >> Imm);
                    }},
                    {SRAI,  []() {
                        x[rd] = (int(A) >> Imm);
                    }},
                    {BEQ,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (A == B) NPC += s_offset;
                    }},
                    {BNE,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (A != B)
                            NPC += s_offset;
                    }},
                    {BLT,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                    }},
                    {BGE,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (int(A) >= int(B)) NPC += s_offset;
                    }},
                    {BLTU,  []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (A < B) NPC += s_offset;
                    }},
                    {BGEU,  []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (A >= B) NPC += s_offset;
                    }},
                    {SB,    []() {
                        int addr = A + sext(func7 << 5 | rd, 12);
                        M[addr] = B;

                    }},
                    {SH,    []() {
                        int addr = A + sext(func7 << 5 | rd, 12);
                        M[addr] = B;
                        M[addr + 1] = B >> 8;
                    }},
                    {SW,    []() {
                        int addr = A + sext(func7 << 5 | rd, 12);
                        M[addr] = B;
                        M[addr + 1] = B >> 8;
                        M[addr + 2] = B >> 16;
                        M[addr + 3] = B >> 24;
                    }},
                    {ADD,   []() {
                        x[rd] = A + B;
                    }},
                    {SLL,   []() {
                        x[rd] = A << B;
                    }},
                    {SLT,   []() {
                        x[rd] = (int(A) < int(B));
                    }},
                    {SLTU,  []() {
                        x[rd] = (A < B);
                    }},
                    {XOR,   []() {
                        x[rd] = A ^ B;
                    }},
                    {SRL,   []() {
                        x[rd] = (A >> B);
                    }},
                    {OR,    []() {
                        x[rd] = A | B;
                    }},
                    {AND,   []() {
                        x[rd] = A & B;
                    }},
                    {SUB,   []() {
                        x[rd] = A - B;
                    }},
                    {SRA,   []() {
                        x[rd] = (int(A) >> int(B));
                    }}
            };
    operMap.at(operation)();
}

void MEM(){}

void WB(){}


