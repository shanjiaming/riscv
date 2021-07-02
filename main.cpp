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

u32 opcode;
u32 rd;
u32 func3;
u32 rs1;
u32 rs2;
u32 func7;
u32 imm11_0;
u32 s_imm11_0;
u32 imm31_12;

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
//    freopen("../multiarray.data", "r", stdin);
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
    imm11_0 = IR >> 20;
    s_imm11_0 = sext(imm11_0, 12);
    imm31_12 = IR >> 12;
    char type = opcodeTypeMap.at(opcode);
    switch (type) {
        case 'U': {
            operation = LUI;
            break;
        }
        case 'u': {
            operation = AUIPC;
            break;
        }
        case 'J': {
            operation = JAL;
            break;
        }
        case 'j': {
            operation = JALR;
            break;
        }
        case 'I': {
            static const map<u32, Operation> Ifunc3Map = {{0b000u, LB},
                                                          {0b001u, LH},
                                                          {0b010u, LW},
                                                          {0b100u, LBU},
                                                          {0b101u, LHU}};
            operation = Ifunc3Map.at(func3);
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
                default: {
                    //cout << "error";
                    exit(0);
                }
            }

            break;
        }
        default: {
            //cout << "error";
            exit(0);
        }
    }

}

void EX(){
    static const map<Operation, function<void()>> operMap
            =
            {
                    {LUI,   []() { x[rd] = imm31_12 << 12; }},
                    {AUIPC, []() { x[rd] = PC + (imm31_12 << 12); }},
                    {JAL,   []() {
                        u32 offset = (getbit(imm31_12, 19) | getbit(imm31_12, 18, 9) >> 9 | getbit(imm31_12, 8) << 2 |
                                      getbit(imm31_12, 7, 0) << 11) << 1;
                        if (rd != 0)
                            x[rd] = PC + 4;//注意这个+4,是指下一条指令的地址，这是是执行完语句再改，考虑与流水的关系，等等。
                        NPC += sext(offset, 21) - 4;
                    }},
                    {JALR,  []() {
                        int t = PC + 4;
                        NPC = ((x[rs1] + s_imm11_0) & ~1);
                        if (rd != 0)x[rd] = t;
                    }},
                    {LB,    []() {
                        int addr = x[rs1] + s_imm11_0;
                        x[rd] = sext(M[addr], 8);
                    }},
                    {LH,    []() {
                        int addr = x[rs1] + s_imm11_0;
                        x[rd] = sext(M[addr] | M[addr + 1] << 8, 16);
                    }},
                    {LW,    []() {
                        int addr = x[rs1] + s_imm11_0;
                        x[rd] = M[addr] | M[addr + 1] << 8 | M[addr + 2] << 16 | M[addr + 3] << 24;

                    }},
                    {LBU,   []() {
                        int addr = x[rs1] + s_imm11_0;
                        x[rd] = M[addr];

                    }},
                    {LHU,   []() {
                        int addr = x[rs1] + s_imm11_0;
                        x[rd] = M[addr] | M[addr + 1] << 8;

                    }},
                    {ADDI,  []() {
                        x[rd] = x[rs1] + s_imm11_0;
                    }},
                    {SLTI,  []() {
                        x[rd] = (int(x[rs1]) < int(s_imm11_0));
                    }},
                    {SLTIU, []() {
                        x[rd] = (x[rs1] < s_imm11_0);
                    }},
                    {XORI,  []() {
                        x[rd] = x[rs1] ^ s_imm11_0;
                    }},
                    {ORI,   []() {
                        x[rd] = x[rs1] | s_imm11_0;
                    }},
                    {ANDI,  []() {
                        x[rd] = x[rs1] & s_imm11_0;
                    }},
                    {SLLI,  []() {
                        x[rd] = x[rs1] << rs2;
                    }},
                    {SRLI,  []() {
                        x[rd] = (x[rs1] >> rs2);
                    }},
                    {SRAI,  []() {
                        x[rd] = (int(x[rs1]) >> rs2);
                    }},
                    {BEQ,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (x[rs1] == x[rs2]) NPC += s_offset;
                    }},
                    {BNE,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (x[rs1] != x[rs2])
                            NPC += s_offset;
                    }},
                    {BLT,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (int(x[rs1]) < int(x[rs2])) NPC += s_offset;

                    }},
                    {BGE,   []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (int(x[rs1]) >= int(x[rs2])) NPC += s_offset;
                    }},
                    {BLTU,  []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (x[rs1] < x[rs2]) NPC += s_offset;
                    }},
                    {BGEU,  []() {
                        u32 s_offset =
                                sext((getbit(func7, 6) << 6 | getbit(rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                      getbit(rd, 4, 1)),
                                     13) - 4;
                        if (x[rs1] >= x[rs2]) NPC += s_offset;
                    }},
                    {SB,    []() {
                        int addr = x[rs1] + sext(func7 << 5 | rd, 12);
                        M[addr] = x[rs2];

                    }},
                    {SH,    []() {
                        int addr = x[rs1] + sext(func7 << 5 | rd, 12);
                        M[addr] = x[rs2];
                        M[addr + 1] = x[rs2] >> 8;
                    }},
                    {SW,    []() {
                        int addr = x[rs1] + sext(func7 << 5 | rd, 12);
                        M[addr] = x[rs2];
                        M[addr + 1] = x[rs2] >> 8;
                        M[addr + 2] = x[rs2] >> 16;
                        M[addr + 3] = x[rs2] >> 24;
                    }},
                    {ADD,   []() {
                        x[rd] = x[rs1] + x[rs2];
                    }},
                    {SLL,   []() {
                        x[rd] = x[rs1] << x[rs2];
                    }},
                    {SLT,   []() {
                        x[rd] = (int(x[rs1]) < int(x[rs2]));
                    }},
                    {SLTU,  []() {
                        x[rd] = (x[rs1] < x[rs2]);
                    }},
                    {XOR,   []() {
                        x[rd] = x[rs1] ^ x[rs2];
                    }},
                    {SRL,   []() {
                        x[rd] = (x[rs1] >> x[rs2]);
                    }},
                    {OR,    []() {
                        x[rd] = x[rs1] | x[rs2];
                    }},
                    {AND,   []() {
                        x[rd] = x[rs1] & x[rs2];
                    }},
                    {SUB,   []() {
                        x[rd] = x[rs1] - x[rs2];
                    }},
                    {SRA,   []() {
                        x[rd] = (int(x[rs1]) >> int(x[rs2]));
                    }}
            };
    operMap.at(operation)();
}

void MEM(){}

void WB(){}


