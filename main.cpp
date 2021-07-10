#include <iostream>
#include <fstream>
#include <bitset>
#include <map>
#include <cassert>
#include <functional>
#include <set>

using namespace std;

typedef unsigned int u32;

unsigned char M[1048576] = {0};
u32 x[32] = {0};
u32 PC = 0, NPC = 0,PC_guess = 0;
u32 mayPC = 0;
u32 IR = 0;
bool bubble = 0;


enum Operation {
    NOOP,
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
    BLT,
    BGE,
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
    SRA,
    END
};

map<Operation, string> operstringmap{
        {NOOP,"noop"},
        {LUI,"lui"},
        {AUIPC,"auipc"},
        {JAL,"jal"},
        {JALR,"jalr"},
        {LB,"lb"},
        {LH,"lh"},
        {LW,"lw"},
        {LBU,"lbu"},
        {LHU,"lhu"},
        {ADDI,"addi"},
        {SLTI,"slti"},
        {SLTIU,"sltiu"},
        {XORI,"xori"},
        {ORI,"ori"},
        {ANDI,"andi"},
        {SLLI,"slli"},
        {SRLI,"srli"},
        {SRAI,"srai"},
        {BEQ,"beq"},
        {BNE,"bne"},
        {BLT,"blt"},
        {BGE,"bge"},
        {BLTU,"bltu"},
        {BGEU,"bgeu"},
        {SB,"sb"},
        {SH,"sh"},
        {SW,"sw"},
        {ADD,"add"},
        {SLL,"sll"},
        {SLT,"slt"},
        {SLTU,"sltu"},
        {XOR,"xor"},
        {SRL,"srl"},
        {OR,"or"},
        {AND,"and"},
        {SUB,"sub"},
        {SRA,"sra"},
        {END,"end"}
};

const set<Operation> writeOp = {
        LUI,
        AUIPC,
        ADDI,
        SLTI,
        SLTIU,
        XORI,
        ORI,
        ANDI,
        SLLI,
        SRLI,
        SRAI,
        ADD,
        SLL,
        SLT,
        SLTU,
        XOR,
        SRL,
        OR,
        SUB,
        SRA,
        LB,
        LH,
        LW,
        LBU,
        LHU
};

const set<Operation> useOp = {
        LUI,
        AUIPC,
        ADDI,
        SLTI,
        SLTIU,
        XORI,
        ORI,
        ANDI,
        SLLI,
        SRLI,
        SRAI,
        ADD,
        SLL,
        SLT,
        SLTU,
        XOR,
        SRL,
        OR,
        SUB,
        SRA,
        LB,
        LH,
        LW,
        LBU,
        LHU
};



struct Saver{
    u32 rd = 0;
    u32 rs2 = 0;
    u32 B = 0;
    u32 ALUOutput = 0;
    u32 A = 0;
    u32 Imm = 0;
    u32 LMD = 0;
    u32 rs1=0;
    Operation operation = NOOP;
};
Saver ID_EX, EX_MEM, MEM_WB;




u32 writeVal (Operation op){
    switch (op) {
        case LUI:
        case AUIPC:
        case ADDI:
        case SLTI:
        case SLTIU:
        case XORI:
        case ORI:
        case ANDI:
        case SLLI:
        case SRLI:
        case SRAI:
        case ADD:
        case SLL:
        case SLT:
        case SLTU:
        case XOR:
        case SRL:
        case OR:
        case SUB:
        case SRA:
            return MEM_WB.ALUOutput;
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            return MEM_WB.LMD;
    }
    cout << "dangerous return 0"<< endl;
    return 0;
}


class Guesser {

    struct TwoBit {
        bool checker;

        bool a[2] = {0};

        bool guess() {
            return checker = a[1];
        }

        bool update(bool u) {
            assert(checker == a[1]);
            bool ret = a[1] == u;
            if (a[1] != a[0])
                a[1] = u;
            a[0] = u;
            return ret;
        }
    };

    TwoBit group[4096] = {0};
    int rightnum = 0, wrongnum = 0;
public:
    bool guess(u32 code) {
        bool gusans = group[code & 0b111111111111u].guess();
//        cout << "guess code = " << hex << code << endl;
//        cout << "guess ans = " << gusans << endl;
        return gusans;
    }

    bool update(u32 code, bool u) {
        bool ret = group[code & 0b111111111111u].update(u);
        ++(ret?rightnum:wrongnum);
//        cout << "update code = " << hex << code << endl;
//        cout << "update u = " << u <<endl;
//        cout << "update ans = " << ret << endl;
        return ret;
    }

    void show_success_rate(){
        cout << "guess_right_num:" << rightnum << endl;
        cout << "guess_wrong_num:" << wrongnum << endl;
        cout << "total_guess_num:" << rightnum+wrongnum << endl;
        cout << "success_rate:" << double(rightnum)/(rightnum+wrongnum) << endl;
    }
};

Guesser g;

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
    IF();
    while (true) {
        WB();
        MEM();
        EX();
        ID();
        IF();
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
    if(bubble) {
        bubble = 0;
        IR = 0;
        return;
    }
    PC = NPC;
//    cout << "PC=" << hex << PC << endl;
    IR = M[PC] | M[PC + 1] << 8 | M[PC + 2] << 16 | M[PC + 3] << 24;
}

void ID() {
    if (IR == 0x0ff00513) {
        ID_EX.operation = END;
        NPC = PC;
        return;

    }
    if (IR == 0) {
        ID_EX.operation = NOOP;
        return;
    }
    NPC = PC + 4;

    u32 opcode = IR & 0b1111111u;
    ID_EX.rd = IR >> 7 & 0b11111u;
    u32 func3 = IR >> 12 & 0b111u;
    ID_EX.rs1 = IR >> 15 & 0b11111u;
    ID_EX.rs2 = IR >> 20 & 0b11111u;
    u32 func7 = IR >> 25;

    ID_EX.A = x[ID_EX.rs1];
    ID_EX.B = x[ID_EX.rs2];

    switch (opcode) {
        case 0b0110111u:
            ID_EX.operation = LUI;
            ID_EX.Imm = IR >> 12 << 12;
            break;
        case 0b0010111u:
            ID_EX.operation = AUIPC;
            ID_EX.Imm = IR >> 12 << 12;
            break;
        case 0b1101111u: {
            ID_EX.operation = JAL;
            u32 imm31_12 = IR >> 12;
            if (ID_EX.rd != 0)
                x[ID_EX.rd] = PC + 4;//不在WB做而是提前做，防止回传的麻烦。同时因为NPC与其并行，都是寄存器读写，不增加代价。
            NPC = PC + sext((getbit(imm31_12, 19) | getbit(imm31_12, 18, 9) >> 9 | getbit(imm31_12, 8) << 2 |
                             getbit(imm31_12, 7, 0) << 11) << 1, 21);
            break;
        }
        case 0b1100111u:
            ID_EX.operation = JALR;
            if (ID_EX.rd != 0)
                x[ID_EX.rd] = PC + 4;
            NPC = ((ID_EX.A + sext(IR >> 20, 12)) & ~1);
            break;
        case 0b0000011u:
            static const map<u32, Operation> Ifunc3Map = {{0b000u, LB},
                                                          {0b001u, LH},
                                                          {0b010u, LW},
                                                          {0b100u, LBU},
                                                          {0b101u, LHU}};
            ID_EX.operation = Ifunc3Map.at(func3);
            ID_EX.Imm = sext(IR >> 20, 12);
            bubble = 1;
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
            ID_EX.operation = ifunc3Map.at(func3);
            if (func3 == 0b101u && func7 == 0b0100000u) ID_EX.operation = SRAI;
            ID_EX.Imm = (ID_EX.operation == SLLI || ID_EX.operation == SRLI || ID_EX.operation == SRAI) ? ID_EX.rs2 : sext(IR >> 20, 12);
            break;

        case 0b1100011u: {
            static const map<u32, Operation> Bfunc3Map = {{0b000u, BEQ},
                                                          {0b001u, BNE},
                                                          {0b100u, BLT},
                                                          {0b101u, BGE},
                                                          {0b110u, BLTU},
                                                          {0b111u, BGEU}};
            ID_EX.operation = Bfunc3Map.at(func3);
            u32 NPCcalc = PC + sext((getbit(func7, 6) << 6 | getbit(ID_EX.rd, 0) << 11 | getbit(func7, 5, 0) << 5 |
                                     getbit(ID_EX.rd, 4, 1)),
                                    13);
            if (g.guess(PC)) {
                NPC = NPCcalc;
                mayPC = PC + 4;
            } else {
                NPC = PC + 4;
                mayPC = NPCcalc;
            }
            PC_guess = PC;
            break;
        }

        case 0b0100011u:
            static const map<u32, Operation> Sfunc3Map = {{0b000u, SB},
                                                          {0b001u, SH},
                                                          {0b010u, SW}};
            ID_EX.operation = Sfunc3Map.at(func3);
            ID_EX.Imm = func7;
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
                    ID_EX.operation = R71func3Map.at(func3);
                    break;

                case 0b0100000u:
                    static const map<u32, Operation> R72func3Map = {{0b000u, SUB},
                                                                    {0b101u, SRA}};
                    ID_EX.operation = R72func3Map.at(func3);
                    break;

            }
            break;
    }
    if(writeOp.count(MEM_WB.operation)) {
        if (ID_EX.rs1 == MEM_WB.rd) {
            ID_EX.A = writeVal(MEM_WB.operation);
        }
        if (ID_EX.rs2 == MEM_WB.rd) {
            ID_EX.B = writeVal(MEM_WB.operation);
        }
    }
    if(writeOp.count(EX_MEM.operation)) {
        if (ID_EX.rs1 == EX_MEM.rd) {
            ID_EX.A = EX_MEM.ALUOutput;
        }
        if (ID_EX.rs2 == EX_MEM.rd) {
            ID_EX.B = EX_MEM.ALUOutput;
        }
    }

}

void EX() {
    EX_MEM = ID_EX;

    static const map<Operation, function<bool(u32, u32)>> Bmap = {
            {BEQ, [](u32 a, u32 b){return a==b;}},
            {BNE, [](u32 a, u32 b){return a!=b;}},
            {BLT, [](u32 a, u32 b){return int(a)<int(b);}},
            {BGE, [](u32 a, u32 b){return int(a)>=int(b);}},
            {BLTU, [](u32 a, u32 b){return a<b;}},
            {BGEU, [](u32 a, u32 b){return a>=b;}}
    };
    //这里由于逆序性，不必也无法复制rs1和rs2。
    //得先LMD再ALUOutput短路，因为最近的总是被最先修改的。


    switch (ID_EX.operation) {
        case LUI: {
            EX_MEM.ALUOutput = EX_MEM.Imm;
            break;
        }
        case AUIPC: {
            EX_MEM.ALUOutput = PC + EX_MEM.Imm;
            break;
        }

        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU: {
            EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.Imm;
            break;
        }
        case ADDI: {
            EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.Imm;
            break;
        }
        case SLTI: {
            EX_MEM.ALUOutput = (int(EX_MEM.A) < int(EX_MEM.Imm));
            break;
        }
        case SLTIU: {
            EX_MEM.ALUOutput = (EX_MEM.A < EX_MEM.Imm);
            break;
        }
        case XORI: {
            EX_MEM.ALUOutput = EX_MEM.A ^ EX_MEM.Imm;
            break;
        }
        case ORI: {
            EX_MEM.ALUOutput = EX_MEM.A | EX_MEM.Imm;
            break;
        }
        case ANDI: {
            EX_MEM.ALUOutput = EX_MEM.A & EX_MEM.Imm;
            break;
        }
        case SLLI: {
            EX_MEM.ALUOutput = EX_MEM.A << EX_MEM.Imm;
            break;
        }
        case SRLI: {
            EX_MEM.ALUOutput = (EX_MEM.A >> EX_MEM.Imm);
            break;
        }
        case SRAI: {
            EX_MEM.ALUOutput = (int(EX_MEM.A) >> EX_MEM.Imm);
            break;
        }
        case SB:
        case SH:
        case SW: {
            EX_MEM.ALUOutput = EX_MEM.A + sext(EX_MEM.Imm << 5 | EX_MEM.rd, 12);
            break;
        }
        case BEQ:
        case BNE:
        case BLT:
        case BGE:
        case BLTU:
        case BGEU:
            if (g.update(PC_guess, Bmap.at(EX_MEM.operation)(EX_MEM.A,EX_MEM.B))) break;
            NPC = mayPC;
            IR = 0;//停一回合，气泡
            break;
        case ADD: {
            EX_MEM.ALUOutput = EX_MEM.A + EX_MEM.B;
            break;
        }
        case SLL: {
            EX_MEM.ALUOutput = EX_MEM.A << EX_MEM.B;
            break;
        }
        case SLT: {
            EX_MEM.ALUOutput = (int(EX_MEM.A) < int(EX_MEM.B));
            break;
        }
        case SLTU: {
            EX_MEM.ALUOutput = (EX_MEM.A < EX_MEM.B);
            break;
        }
        case XOR: {
            EX_MEM.ALUOutput = EX_MEM.A ^ EX_MEM.B;
            break;
        }
        case SRL: {
            EX_MEM.ALUOutput = (EX_MEM.A >> EX_MEM.B);
            break;
        }
        case OR: {
            EX_MEM.ALUOutput = EX_MEM.A | EX_MEM.B;
            break;
        }
        case AND: {
            EX_MEM.ALUOutput = EX_MEM.A & EX_MEM.B;
            break;
        }
        case SUB: {
            EX_MEM.ALUOutput = EX_MEM.A - EX_MEM.B;
            break;
        }
        case SRA: {
            EX_MEM.ALUOutput = (int(EX_MEM.A) >> int(EX_MEM.B));
            break;
        }
    }
//    if(ID_EX.operation != NOOP)cout << operstringmap[ID_EX.operation]<<endl;
}

void MEM() {
    MEM_WB = EX_MEM;

    switch (MEM_WB.operation) {
        case LB:
            MEM_WB.LMD = sext(M[MEM_WB.ALUOutput], 8);
            break;
        case LH:
            MEM_WB.LMD = sext(M[MEM_WB.ALUOutput] | M[MEM_WB.ALUOutput + 1] << 8, 16);
            break;
        case LW:
            MEM_WB.LMD = M[MEM_WB.ALUOutput] | M[MEM_WB.ALUOutput + 1] << 8 | M[MEM_WB.ALUOutput + 2] << 16 | M[MEM_WB.ALUOutput + 3] << 24;
            break;
        case LBU:
            MEM_WB.LMD = M[MEM_WB.ALUOutput];
            break;
        case LHU:
            MEM_WB.LMD = M[MEM_WB.ALUOutput] | M[MEM_WB.ALUOutput + 1] << 8;
            break;
        case SB :
            M[MEM_WB.ALUOutput] = MEM_WB.B;
            break;
        case SH:
            M[MEM_WB.ALUOutput] = MEM_WB.B;
            M[MEM_WB.ALUOutput + 1] = MEM_WB.B >> 8;
            break;
        case SW:
            M[MEM_WB.ALUOutput] = MEM_WB.B;
            M[MEM_WB.ALUOutput + 1] = MEM_WB.B >> 8;
            M[MEM_WB.ALUOutput + 2] = MEM_WB.B >> 16;
            M[MEM_WB.ALUOutput + 3] = MEM_WB.B >> 24;
            break;

    }
}


void WB() {

    if (useOp.count(MEM_WB.operation))
            x[MEM_WB.rd] = writeVal(MEM_WB.operation);
            if(MEM_WB.operation == END) {
                cout << dec << (x[10] & 0b11111111u) << endl;
//        g.show_success_rate();
                exit(0);
            }

}

