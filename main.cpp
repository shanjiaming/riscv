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
u32 PC = 0, NPC = 0;
u32 IR = 0;
u32 A = 0, B = 0;
u32 Imm = 0;
u32 ALUOutput = 0;
u32 LMD = 0;
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
enum WBClass {
    ALUWB, LMDWB, SAVE, NOWB
};
Operation operation;
WBClass wbClass;

u32 opcode;
u32 rd;
u32 func3;
u32 rs1;
u32 rs2;
u32 func7;
u32 imm11_0;
u32 s_imm11_0;
u32 imm31_12;

void initialize();

u32 sext(int code, int len) {
    return code << 32 - len >> 32 - len;
}

u32 getbit(u32 code, int ptr) {
    return code & 1 << ptr;
}

u32 getbit(u32 code, int high, int low) {
    return code >> low << (31 - high + low) >> (31 - high);
}


void IF();

void ID();

void EX();

void MEM();

void WB();


int main() {
    freopen("../bulgarian.data", "r", stdin);
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

    opcode = IR & 0b1111111u;
    rd = IR >> 7 & 0b11111u;
    func3 = IR >> 12 & 0b111u;
    rs1 = IR >> 15 & 0b11111u;
    rs2 = IR >> 20 & 0b11111u;
    func7 = IR >> 25;
    imm11_0 = IR >> 20;
    s_imm11_0 = sext(IR >> 20, 12);
    imm31_12 = IR >> 12;

    A = x[rs1];
    B = x[rs2];
    wbClass = ALUWB;

    switch (opcode) {
        case 0b0110111u:
            operation = LUI;
            Imm = IR >> 12 << 12;
            break;
        case 0b0010111u:
            operation = AUIPC;
            Imm = IR >> 12 << 12;
            break;
        case 0b1101111u:
            operation = JAL;
            Imm = sext((getbit(imm31_12, 19) | getbit(imm31_12, 18, 9) >> 9 | getbit(imm31_12, 8) << 2 |
                        getbit(imm31_12, 7, 0) << 11) << 1, 21);
            if (rd == 0) wbClass = NOWB;
            break;
        case 0b1100111u:
            operation = JALR;
            Imm = sext(IR >> 20, 12);
            if (rd == 0) wbClass = NOWB;
            break;
        case 0b0000011u:
            static const map<u32, Operation> Ifunc3Map = {{0b000u, LB},
                                                          {0b001u, LH},
                                                          {0b010u, LW},
                                                          {0b100u, LBU},
                                                          {0b101u, LHU}};
            operation = Ifunc3Map.at(func3);
            Imm = sext(IR >> 20, 12);
            wbClass = LMDWB;
            break;
        case 0b0010011u:
            static const map<u32, Operation> ifunc3Map = {{0b000u, ADDI},
                                                          {0b010u, SLTI},
                                                          {0b011u, SLTIU},
                                                          {0b100u, XORI},
                                                          {0b110u, ORI},
                                                          {0b111u, ANDI},
                                                          {0b001u, SLLI},
                                                          {0b101u, SRLI}};
            operation = ifunc3Map.at(func3);
            if (func3 == 0b101u && func7 == 0b0100000u) operation = SRAI;
            Imm = (operation == SLLI || operation == SRLI || operation == SRAI) ? rs2 : sext(IR >> 20, 12);

            break;

        case 0b1100011u:
            static const map<u32, Operation> Bfunc3Map = {{0b000u, BEQ},
                                                          {0b001u, BNE},
                                                          {0b100u, BLT},
                                                          {0b101u, BGE},
                                                          {0b110u, BLTU},
                                                          {0b111u, BGEU}};
            operation = Bfunc3Map.at(func3);
            wbClass = NOWB;
            break;

        case 0b0100011u:
            static const map<u32, Operation> Sfunc3Map = {{0b000u, SB},
                                                          {0b001u, SH},
                                                          {0b010u, SW}};
            operation = Sfunc3Map.at(func3);
            wbClass = SAVE;
            break;

        case 0b0110011u:
            switch (func7) {
                case 0b0000000u:
                    static const map<u32, Operation> R71func3Map = {{0b000u, ADD},
                                                                    {0b001u, SLL},
                                                                    {0b010u, SLT},
                                                                    {0b011u, SLTU},
                                                                    {0b100u, XOR},
                                                                    {0b101u, SRL},
                                                                    {0b110u, OR}};
                    operation = R71func3Map.at(func3);
                    break;

                case 0b0100000u:
                    static const map<u32, Operation> R72func3Map = {{0b000u, SUB},
                                                                    {0b101u, SRA}};
                    operation = R72func3Map.at(func3);
                    break;

            }
            break;

    }
}


void EX() {
    static const map<Operation, function<void()>> operMap
            =
            {
                    {LUI,   []() { ALUOutput = Imm; }},
                    {AUIPC, []() { ALUOutput = PC + Imm; }},
                    {JAL,   []() {
                        if (wbClass == ALUWB)
                            ALUOutput = PC + 4;//注意这个+4,是指下一条指令的地址，这是是执行完语句再改，考虑与流水的关系，等等。
                        NPC += Imm - 4;
                    }},
                    {JALR,  []() {
                        int t = PC + 4;
                        NPC = ((A + Imm) & ~1);
                        if (wbClass == ALUWB)ALUOutput = t;
                    }},
                    {LB,    []() {
                        ALUOutput = A + Imm;
                    }},
                    {LH,    []() {
                        ALUOutput = A + Imm;
                    }},
                    {LW,    []() {
                        ALUOutput = A + Imm;

                    }},
                    {LBU,   []() {
                        ALUOutput = A + Imm;

                    }},
                    {LHU,   []() {
                        ALUOutput = A + Imm;

                    }},
                    {ADDI,  []() {
                        ALUOutput = A + Imm;
                    }},
                    {SLTI,  []() {
                        ALUOutput = (int(A) < int(Imm));
                    }},
                    {SLTIU, []() {
                        ALUOutput = (A < Imm);
                    }},
                    {XORI,  []() {
                        ALUOutput = A ^ Imm;
                    }},
                    {ORI,   []() {
                        ALUOutput = A | Imm;
                    }},
                    {ANDI,  []() {
                        ALUOutput = A & Imm;
                    }},
                    {SLLI,  []() {
                        ALUOutput = A << Imm;
                    }},
                    {SRLI,  []() {
                        ALUOutput = (A >> Imm);
                    }},
                    {SRAI,  []() {
                        ALUOutput = (int(A) >> Imm);
                    }},
                    {BEQ,   []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;
                    }},
                    {BNE,   []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;
                    }},
                    {BLT,   []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;

                    }},
                    {BGE,   []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;
                    }},
                    {BLTU,  []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;
                    }},
                    {BGEU,  []() {
                        ALUOutput = sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                          getbit(rd, 4, 1)),
                                         13) - 4;
                    }},
                    {SB,    []() {
                        ALUOutput = A + sext(func7 << 5 | rd, 12);
                    }},
                    {SH,    []() {
                        ALUOutput = A + sext(func7 << 5 | rd, 12);
                    }},
                    {SW,    []() {
                        ALUOutput = A + sext(func7 << 5 | rd, 12);
                    }},
                    {ADD,   []() {
                        ALUOutput = A + B;
                    }},
                    {SLL,   []() {
                        ALUOutput = A << B;
                    }},
                    {SLT,   []() {
                        ALUOutput = (int(A) < int(B));
                    }},
                    {SLTU,  []() {
                        ALUOutput = (A < B);
                    }},
                    {XOR,   []() {
                        ALUOutput = A ^ B;
                    }},
                    {SRL,   []() {
                        ALUOutput = (A >> B);
                    }},
                    {OR,    []() {
                        ALUOutput = A | B;
                    }},
                    {AND,   []() {
                        ALUOutput = A & B;
                    }},
                    {SUB,   []() {
                        ALUOutput = A - B;
                    }},
                    {SRA,   []() {
                        ALUOutput = (int(A) >> int(B));
                    }}
            };
    operMap.at(operation)();
}


void MEM() {
    switch (operation) {
        case LB:
            LMD = sext(M[ALUOutput], 8);
            break;
        case LH:
            LMD = sext(M[ALUOutput] | M[ALUOutput + 1] << 8, 16);
            break;
        case LW:
            LMD = M[ALUOutput] | M[ALUOutput + 1] << 8 | M[ALUOutput + 2] << 16 | M[ALUOutput + 3] << 24;
            break;
        case LBU:
            LMD = M[ALUOutput];
            break;
        case LHU:
            LMD = M[ALUOutput] | M[ALUOutput + 1] << 8;
            break;
        case SB :
            M[ALUOutput] = B;
            break;
        case SH:
            M[ALUOutput] = B;
            M[ALUOutput + 1] = B >> 8;
            break;
        case SW:
            M[ALUOutput] = B;
            M[ALUOutput + 1] = B >> 8;
            M[ALUOutput + 2] = B >> 16;
            M[ALUOutput + 3] = B >> 24;
            break;
        case BEQ:
            if (A == B) NPC += ALUOutput;
            break;
        case BNE:
            if (A != B) NPC += ALUOutput;
            break;
        case BLT:
            if (int(A) < int(B)) NPC += ALUOutput;
            break;
        case BGE:
            if (int(A) >= int(B)) NPC += ALUOutput;
            break;
        case BLTU:
            if (A < B) NPC += ALUOutput;
            break;
        case BGEU:
            if (A >= B) NPC += ALUOutput;
            break;
    }
}

void WB() {
    switch (wbClass) {
        case ALUWB:
            x[rd] = ALUOutput;
            break;
        case LMDWB:
            x[rd] = LMD;
            break;
    }
}

