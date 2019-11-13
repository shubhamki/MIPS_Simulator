#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
	bitset<32>  PC;
	bool        nop;
};

struct IDStruct {
	bitset<32>  Instr;
	bool        nop;
};

struct EXStruct {
	bitset<32>  Read_data1;
	bitset<32>  Read_data2;
	bitset<16>  Imm;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        is_I_type;
	bool        rd_mem;
	bool        wrt_mem;
	bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
	bool        wrt_enable;
	bool        nop;
};

struct MEMStruct {
	bitset<32>  ALUresult;
	bitset<32>  Store_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        rd_mem;
	bool        wrt_mem;
	bool        wrt_enable;
	bool        nop;
};

struct WBStruct {
	bitset<32>  Wrt_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        wrt_enable;
	bool        nop;
};

struct stateStruct {
	IFStruct    IF;
	IDStruct    ID;
	EXStruct    EX;
	MEMStruct   MEM;
	WBStruct    WB;
};

class RF
{
public:
	bitset<32> Reg_data;
	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	bitset<32> readRF(bitset<5> Reg_addr)
	{
		Reg_data = Registers[Reg_addr.to_ulong()];
		return Reg_data;
	}

	void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
	{
		Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
	}

	void outputRF()
	{
		ofstream rfout;
		rfout.open("RFresult.txt", std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "State of RF:\t" << endl;
			for (int j = 0; j < 32; j++)
			{
				rfout << Registers[j] << endl;
			}
		}
		else cout << "Unable to open file";
		rfout.close();
	}

private:
	vector<bitset<32> >Registers;
};

class INSMem
{
public:
	bitset<32> Instruction;
	INSMem()
	{
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open("imem.txt");
		if (imem.is_open())
		{
			while (getline(imem, line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		imem.close();
	}

	bitset<32> readInstr(bitset<32> ReadAddress /*int Instruction*/)
	{
		string insmem;
		insmem.append(IMem[ReadAddress.to_ulong()].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
		/*if (Instruction + 4 < IMem.size())
		{
			insmem.append(IMem[Instruction].to_string());
			insmem.append(IMem[Instruction + 1].to_string());
			insmem.append(IMem[Instruction + 2].to_string());
			insmem.append(IMem[Instruction + 3].to_string());
		}*/
		this->Instruction = bitset<32>(insmem);		//read instruction memory
		return this->Instruction;
	}

private:
	vector<bitset<8> > IMem;
};

class DataMem
{
public:
	bitset<32> ReadData;
	DataMem()
	{
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		dmem.close();
	}

	bitset<32> readDataMem(bitset<32> Address)
	{
		string datamem;
		datamem.append(DMem[Address.to_ulong()].to_string());
		datamem.append(DMem[Address.to_ulong() + 1].to_string());
		datamem.append(DMem[Address.to_ulong() + 2].to_string());
		datamem.append(DMem[Address.to_ulong() + 3].to_string());
		ReadData = bitset<32>(datamem);		//read data memory
		return ReadData;
	}

	void writeDataMem(bitset<32> Address, bitset<32> WriteData)
	{
		DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
		DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
		DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
		DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
	}

	void outputDataMem()
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j < 1000; j++)
			{
				dmemout << DMem[j] << endl;
			}

		}
		else cout << "Unable to open file";
		dmemout.close();
	}

private:
	vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
	ofstream printstate;
	printstate.open("stateresult.txt", std::ios_base::app);
	if (printstate.is_open())
	{
		printstate << "State after executing cycle:\t" << cycle << endl;

		printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
		printstate << "IF.nop:\t" << state.IF.nop << endl;

		printstate << "ID.Instr:\t" << state.ID.Instr << endl;
		printstate << "ID.nop:\t" << state.ID.nop << endl;

		printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
		printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
		printstate << "EX.Imm:\t" << state.EX.Imm << endl;
		printstate << "EX.Rs:\t" << state.EX.Rs << endl;
		printstate << "EX.Rt:\t" << state.EX.Rt << endl;
		printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
		printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
		printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
		printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
		printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
		printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
		printstate << "EX.nop:\t" << state.EX.nop << endl;

		printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
		printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
		printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
		printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
		printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
		printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
		printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
		printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
		printstate << "MEM.nop:\t" << state.MEM.nop << endl;

		printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
		printstate << "WB.Rs:\t" << state.WB.Rs << endl;
		printstate << "WB.Rt:\t" << state.WB.Rt << endl;
		printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
		printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
		printstate << "WB.nop:\t" << state.WB.nop << endl;
	}
	else cout << "Unable to open file";
	printstate.close();
}
unsigned long shiftbits(bitset<32> inst, int start)
{
	unsigned long ulonginst;
	return ((inst.to_ulong()) >> start);

}
bitset<32> signextend(bitset<16> imm)
{
	string sestring;
	//if (imm[15] == 0) 
	{
		sestring = "0000000000000000" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	/*else {
		sestring = "1111111111111111" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}*/
	return (bitset<32>(sestring));

}
enum class MyEnumClass
{
	ADD,
	SUB,
	ILOAD,
	ISTORE,
	BEQ,
	HALT,
	tdefault,
};

void NOP_handler(bool& IFvar, bool& IDvar, bool& EXvar, bool& MEMvar, bool& WBvar, MyEnumClass InstType, bool HaltState)
{
	if (!HaltState)
	{
		if (InstType == MyEnumClass::ADD || InstType == MyEnumClass::SUB)
		{
			IFvar = 0;
			IDvar = 0;
			EXvar = 0;
			MEMvar = 1;
			WBvar = 0;
		}
		else if (InstType == MyEnumClass::ILOAD)
		{
			IFvar = 0;
			IDvar = 0;
			EXvar = 0;
			MEMvar = 0;
			WBvar = 0;
		}
		else if (InstType == MyEnumClass::ISTORE)
		{
			IFvar = 0;
			IDvar = 0;
			EXvar = 0;
			MEMvar = 0;
			WBvar = 1;
		}
	}
	else
	{
		if (IFvar != 1)
		{
			IFvar = 1;
		}
		WBvar = MEMvar;
		MEMvar = EXvar;
		EXvar = IDvar;
		IDvar = IFvar;
	}
}


int main()
{

	RF myRF;
	INSMem myInsMem;
	DataMem myDataMem;
	stateStruct state, newState;
	//memset(&state, 0, sizeof(state));
	//memset(&newState, 0, sizeof(newState));
	newState.IF.PC = 0;
	newState.IF.nop = 1;
	newState.ID.Instr = 00000000000000000000000000000000;
	newState.ID.nop = 1;
	newState.EX.Read_data1 = 00000000000000000000000000000000;
	newState.EX.Read_data2 = 00000000000000000000000000000000;
	newState.EX.Imm = 0000000000000000;
	newState.EX.Rs = 00000;
	newState.EX.Rt = 00000;
	newState.EX.Wrt_reg_addr = 00000;
	newState.EX.is_I_type = 0;
	newState.EX.rd_mem = 0;
	newState.EX.wrt_mem = 0;
	newState.EX.alu_op = 1;
	newState.EX.wrt_enable = 0;
	newState.EX.nop = 1;
	newState.MEM.ALUresult = 00000000000000000000000000000000;
	newState.MEM.Store_data = 00000000000000000000000000000000;
	newState.MEM.Rs = 00000;
	newState.MEM.Rt = 00000;
	newState.MEM.Wrt_reg_addr = 00000;
	newState.MEM.rd_mem = 0;
	newState.MEM.wrt_mem = 0;
	newState.MEM.wrt_enable = 0;
	newState.MEM.nop = 1;
	newState.WB.Wrt_data = 00000000000000000000000000000000;
	newState.WB.Rs = 00000;
	newState.WB.Rt = 00000;
	newState.WB.Wrt_reg_addr = 00000;
	newState.WB.wrt_enable = 0;
	newState.WB.nop = 1;
	state.ID.nop = 1;
	state.EX.nop = 1;
	state.MEM.nop = 1;
	state.WB.nop = 1;
	state.IF.nop = 0;
	state.IF.PC = 0;
	bitset<32> pc = 0;
	bitset<6> Opcode = 0;
	bitset<6> funct = 0;
	bitset<32> BranchAddress = 0;
	int cycle = 0;
	bool TriggerStall = 0;
	bool IsNotEqual = 0;
	MyEnumClass InstructionType = MyEnumClass::tdefault;
	int myinstruction = 0;
	while (1) {

		newState.WB.nop = state.MEM.nop;
		if (!state.WB.nop)
		{
			/* --------------------- WB stage --------------------- */
			//for load operation write back to RF 
			//bitset<16> imm = bitset<16>(shiftbits(newState.IF.PC, 0));
			//-------bitset<32> fetchedMem = myDataMem.readDataMem(state.MEM.ALUresult);
			//myRF.writeRF(newState.EX.Rt, fetchedMem);
			//for store operation NOP of WB stage
			//myDataMem.writeDataMem((bitset<32>(newState.EX.Rs.to_ulong() + (signextend(newState.EX.Imm)).to_ulong())), newState.EX.Read_data2);
			//newState.WB.nop = 1;
			//for add operation ALU does the sum, MEM in NO-OP and WB in RF 
			//-------bitset<32> sum = bitset<32>(state.MEM.ALUresult);
			//myRF.writeRF(newState.EX.Wrt_reg_addr, sum);
			//newState.MEM.nop = 1;
			//for sub operation
			//bitset<32> sub = bitset<32>(state.MEM.ALUresult);
			//myRF.writeRF(newState.EX.Wrt_reg_addr, sub);
			//newState.MEM.nop = 1;
			//for branch/control operation
			//---------WB stage
			newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			// either fetchedMem, sum or sub depending on the type of Instruction
			//if(cycle<8)
			//if(state.WB.wrt_enable)
			//	newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
			if (state.MEM.rd_mem)
			{
				//newState.WB.Wrt_data = fetchedMem;
				myRF.writeRF(state.MEM.Rt, newState.WB.Wrt_data);
			}
			else if (!state.MEM.wrt_mem)
			{
				//newState.WB.Wrt_data = sum;
				myRF.writeRF(state.MEM.Wrt_reg_addr, newState.WB.Wrt_data);
			}
			newState.WB.wrt_enable = state.MEM.wrt_enable;
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;

		}

		/* --------------------- MEM stage --------------------- */
		//needs attention, general case
		newState.MEM.nop = state.EX.nop;
		if (!state.MEM.nop)
		{
			//Only for Store Instruction following Store Data to be looked after
			newState.MEM.Store_data = state.EX.Read_data2;
			// for Load
			if (state.EX.rd_mem && state.EX.is_I_type)
			{
				unsigned long var = state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong();
				newState.MEM.ALUresult = bitset<32>((int)var);
				newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
				InstructionType = MyEnumClass::ILOAD;
			}
			// for Store
			else if (state.EX.is_I_type && state.EX.wrt_mem)
			{
				unsigned long var = state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong();
				newState.MEM.ALUresult = bitset<32>(var);
				//////////// seems correct logically, that the newly calculated ALU value should act as address for DMem storage 
				myDataMem.writeDataMem(newState.MEM.ALUresult, newState.MEM.Store_data);
				////////////
				InstructionType = MyEnumClass::ISTORE;

				//newState.WB.nop = 1;
			}
			// for R type Add
			else if (state.EX.alu_op)
			{
				unsigned long var = state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong();
				newState.MEM.ALUresult = bitset<32>(var);
				newState.WB.Wrt_data = state.MEM.ALUresult;
				InstructionType = MyEnumClass::ADD;
				//newState.MEM.nop = 1;
			}
			else if (!state.EX.alu_op)
			{
				unsigned long var = (state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
				state.MEM.ALUresult = bitset<32>((int)var);
				newState.WB.Wrt_data = state.MEM.ALUresult;
				InstructionType = MyEnumClass::SUB;
				//newState.MEM.nop = 1;
			}
			else
			{
			}
			newState.MEM.rd_mem = state.EX.rd_mem;
			newState.MEM.wrt_mem = state.EX.wrt_mem;
			newState.MEM.Rs = state.EX.Rs;
			newState.MEM.Rt = state.EX.Rt;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
			newState.MEM.wrt_enable = state.EX.wrt_enable;

			//Load - store Dependency
			//check for Rt match                         CURRENT STORE CHECK                                                       PREVIOUS LOAD CHECK
			if ((!newState.MEM.rd_mem && !newState.MEM.wrt_enable && newState.MEM.wrt_mem) && (state.MEM.rd_mem && !state.MEM.wrt_mem && state.MEM.wrt_enable))
			{
				if (newState.MEM.Rt == state.MEM.Wrt_reg_addr)
					newState.MEM.Store_data = newState.WB.Wrt_data;
			}
			//add - store                             CURRENT STORE CHECK                                                         PREVIOUS ADD/SUB CHECK
			if ((!newState.MEM.rd_mem && !newState.MEM.wrt_enable && newState.MEM.wrt_mem) && (state.MEM.wrt_enable && !state.MEM.wrt_mem && !state.MEM.rd_mem))
			{
				if (newState.MEM.Rt == state.MEM.Wrt_reg_addr)
					newState.MEM.Store_data = newState.WB.Wrt_data;
			}



		}

		/* --------------------- EX stage --------------------- */
		//Setting NOP to ID.nop
		newState.EX.nop = state.ID.nop;
		if (!state.EX.nop)
		{
			Opcode = bitset<6>(shiftbits(state.ID.Instr, 26));
			funct = bitset<6>(state.ID.Instr.to_ulong());
			newState.EX.alu_op = (((Opcode.to_ulong() == 0 && funct.to_ulong() != 35) || Opcode.to_ulong() == 35 || Opcode.to_ulong() == 43) ? 1 : 0);
			newState.EX.Imm = bitset<16>(shiftbits(state.ID.Instr, 0));

			newState.EX.nop = state.ID.nop;

			//Checking for I-Type Operations 
			if (Opcode.to_ulong() == 35 || Opcode.to_ulong() == 43 || Opcode.to_ulong() == 4)
				newState.EX.is_I_type = 1;
			else
				newState.EX.is_I_type = 0;

			//Checking for load operation (23)HEX => (35)INT
			if (Opcode.to_ulong() == 35)
				newState.EX.rd_mem = 1;
			else
				newState.EX.rd_mem = 0;

			//Checking for store operation (2B)HEX => (43)INT
			if (Opcode.to_ulong() == 43)
				newState.EX.wrt_mem = 1;
			else
				newState.EX.wrt_mem = 0;

			//Setting Write Enable to One if NOT Store and NOT branch
			if (Opcode.to_ulong() != 43 && Opcode.to_ulong() != 4)
				newState.EX.wrt_enable = 1;
			else
				newState.EX.wrt_enable = 0;

			//Filling Register RdReg 1 and RdReg 2
			newState.EX.Rs = bitset<5>(shiftbits(state.ID.Instr, 21));
			newState.EX.Rt = bitset<5>(shiftbits(state.ID.Instr, 16));

			//Filling the value of write register
			newState.EX.Wrt_reg_addr = (newState.EX.is_I_type == true) ? newState.EX.Rt : bitset<5>(shiftbits(state.ID.Instr, 11));

			//Filling read data 1 and Read data 2 ports
			newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
			newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

			//For branch instruction IF NOT EQUAL
			IsNotEqual = (newState.EX.Read_data1.to_ulong() != newState.EX.Read_data2.to_ulong()) ? 1 : 0;
			if (IsNotEqual && (Opcode.to_ulong() == 4))
			{
				//BranchAddress = bitset<32>((bitset<30>()).to_string<char, std::string::traits_type, std::string::allocator_type>()+ );
				BranchAddress = bitset<32>(
					state.IF.PC.to_ulong() + 4
					+ (bitset<32>((bitset<30>(shiftbits(signextend(newState.EX.Imm), 0))).to_string<char, std::string::traits_type, std::string::allocator_type>() + "00")).to_ulong());
				cout << "Branch Address" << BranchAddress << endl;
				newState.ID.nop = 1;
				newState.EX.nop = 1;
				newState.MEM.nop = 1;
				newState.WB.nop = 1;
				newState.IF.nop = 0;
				newState.EX.wrt_enable = 0;
				newState.EX.rd_mem = 0;
				newState.EX.wrt_mem = 0;
				newState.EX.alu_op = 0;

			}

			bool EXcurrentADD = (newState.EX.wrt_enable && !newState.EX.rd_mem && !newState.EX.wrt_mem);
			bool EXpreviousADD = (state.EX.wrt_enable && !state.EX.rd_mem && !state.EX.wrt_mem);
			bool EXcurrentLOAD = (newState.EX.wrt_enable && newState.EX.rd_mem && !newState.EX.wrt_mem);
			bool EXpreviousLOAD = (state.EX.wrt_enable && state.EX.rd_mem && !state.EX.wrt_mem);



			//LOAD-INDEPENDENT-ADD Instruction sequence
			if (EXcurrentADD && (newState.MEM.wrt_enable && newState.MEM.rd_mem && !newState.MEM.wrt_mem))
			{
				if (newState.EX.Rs == state.MEM.Wrt_reg_addr)
				{
					newState.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (newState.EX.Rt == state.MEM.Wrt_reg_addr)
				{
					newState.EX.Read_data2 = state.WB.Wrt_data;
				}
			}

			//ADD-INDEPENDENT-LOAD Instruction Sequence
			if (EXcurrentLOAD && (state.MEM.wrt_enable && !state.MEM.wrt_mem && !state.MEM.rd_mem))
			{
				if (newState.EX.Rs == state.MEM.Wrt_reg_addr)
				{
					newState.EX.Read_data1 = newState.WB.Wrt_data;
				}
				if (newState.EX.Rt == state.MEM.Wrt_reg_addr)
				{
					newState.EX.Read_data2 = newState.WB.Wrt_data;
				}
			}

			//ADD-LOAD Instruction Sequence
			if (EXcurrentLOAD && EXpreviousADD)
			{
				if (newState.EX.Rs == state.EX.Wrt_reg_addr)
				{
					newState.EX.Read_data1 = newState.MEM.ALUresult;
				}
				if (state.EX.Rt == newState.EX.Wrt_reg_addr)
				{
					newState.EX.Read_data2 = newState.MEM.ALUresult;
				}
			}

			//ADD-ADD Instruction sequence
			if (EXcurrentADD && EXpreviousADD)
			{
				if (state.EX.Wrt_reg_addr == newState.EX.Rs)
				{
					newState.EX.Read_data1 = newState.MEM.ALUresult;
				}
				if (state.EX.Wrt_reg_addr == newState.EX.Rt)
				{
					newState.EX.Read_data2 = newState.MEM.ALUresult;
				}
			}

			//ADD-INDEPENDENT-ADD Instruction sequence
			if (EXcurrentADD && (state.MEM.wrt_enable && !state.MEM.wrt_mem && !state.MEM.rd_mem))
			{
				if (state.MEM.Wrt_reg_addr == newState.EX.Rs)
				{
					newState.EX.Read_data1 = state.MEM.ALUresult;
				}
				if (state.MEM.Wrt_reg_addr == newState.EX.Rt)
				{
					newState.EX.Read_data2 = newState.MEM.ALUresult;
				}
			}

			//stalling for LOAD ADD sequence
			if (EXcurrentADD && EXpreviousLOAD)
			{
				if (newState.EX.Rs == state.EX.Wrt_reg_addr || newState.EX.Rt == state.EX.Wrt_reg_addr)
				{
					TriggerStall = 1;
					newState.EX.nop = 1;
				}
			}


		}

		/* --------------------- ID stage --------------------- */

		if (!state.ID.nop)
		{
			//if(state.IF.PC!=bitset<32>(0))
			//newState.ID.Instr = state.IF.PC;

		}
		newState.ID.nop = state.IF.nop;
		/* --------------------- IF stage --------------------- */
		//check for HALT
		//Handle No-OP bits effectively
		//Fetching New Instruction
		if (!TriggerStall)
		{
			if (!state.IF.nop)
			{
				newState.IF.PC = bitset<32>(myInsMem.readInstr(state.IF.PC));
				newState.ID.Instr = newState.IF.PC;

				myinstruction += 4;

				//myinstruction = bitset<32>(state.IF.PC.to_ulong() + (bitset<32>(4)).to_ulong());

				//Setting NOP of IF stage to zero
				bitset<6> haltcheck = bitset<6>(shiftbits(newState.IF.PC, 26));
				if (haltcheck == bitset<6>(string("111111")))
				{
					newState.IF.nop = 1;
				}
				else
				{
					newState.IF.nop = 0;
					newState.IF.PC = state.IF.PC.to_ulong() + 4;
				}

				if (newState.IF.PC == 4294967295)
				{
					newState.IF.PC = 32;
				}

			}
		}
		else
		{
			newState.IF = state.IF;
			newState.ID = state.ID;
			TriggerStall = 0;
		}

		//Setting the ID/RF stage Instruction


		//Setting the ALU bit to 1 for addu, lw, sw; unset for subu 

		if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
			break;

		printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
		cycle++;
		state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
		IsNotEqual = false;
	}

	myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	//system("Pause");
	return 0;
}