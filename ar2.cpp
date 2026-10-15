const int LED_F3 = 13;
const int LED_F2 = 12;
const int LED_F1 = 11;
const int LED_F0 = 10;

String mem[100];

const int REG_PC = 0;
const int REG_W = 1;
const int REG_X = 2;
const int REG_Y = 3;
const int memStart = 4;

enum State{
	WAIT_LOAD,
	WAIT_CONF,
	EXEC,
	FINISH
};

State stateNow = WAIT_LOAD;

int instructionsCount = 0;


int charToHex(char c){
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

char hexToChar(int val){
	val &= 0x0F;
	if(val < 10) return '0' + val;
	return 'A' + (val-10);
}

void updateLedState(int hex){
	digitalWrite(LED_F3, (hex >> 3) & 0x01);
	digitalWrite(LED_F2, (hex >> 2) & 0x01);
	digitalWrite(LED_F1, (hex >> 1) & 0x01);
	digitalWrite(LED_F0, (hex >> 0) & 0x01);
}

int ULA(int A, int B, int S){
	int ret;

	switch(S & 0x0F){
		case 0x0: ret = ~A;       	     break; // nA     A'
		case 0x1: ret = ~(A | B); 	     break; // AoBn   (A+B)'
		case 0x2: ret = ~A & B;   	     break; // nAeB   A' . B
		case 0x3: ret = 0;                   break; // zeroL  0
		case 0x4: ret = ~(A & B);            break; // AeBn   (A.B)'
	        case 0x5: ret = ~B;                  break; // nB     B'
		case 0x6: ret = (~A & B) | (A & ~B); break; // AxB    A'.B + A.B' {a xor b}
		case 0x7: ret = A & ~B;              break; // AenB   A.B'
		case 0x8: ret = ~A | B;              break; // nAoB   A'+B
		case 0x9: ret = A&B | ~A&~B;         break; // AxBn   A.B + A'.B'
		case 0xA: ret = B;                   break; // copiaB B
		case 0xB: ret = A & B;               break; // AeB    A.B
		case 0xC: ret = 1;                   break; // umL    1
		case 0xD: ret = A | ~B;              break; // AonB   A+B'
		case 0xE: ret = A | B;               break; // AoB    A+B
                case 0xF: ret = A;                   break; // copiaA A
		default: ret=0;                      break;
	}

	return (ret & 0x0F);
}

void load(String input){
	input.trim();
	if(input.length()==0) return;

	mem[REG_PC] = String(memStart);
	mem[REG_W] = "0";
	mem[REG_X] = "0";
	mem[REG_Y] = "0";

	int pos = memStart;
	String opc = "";

	for(size_t i=0; i< input.length(); i++){
		char c = input.charAt(i);
		if(c != ' ' && c != '\n'){
			opc += c;
			if(opc.length() == 3){
				mem[pos] = opc;
				pos++;
				opc = "";
			}
		}
	}

	instructionsCount = pos - memStart;

	if(instructionsCount > 0){
		Serial.print("loaded ");
		Serial.print(instructionsCount);
		Serial.print(" instructions on memory");
		Serial.print("\n Start execution? (y/n)");
		stateNow = WAIT_CONF;
	}
}

void printDump(){
	Serial.print("->|");
	for(int i=0; i<4; i++){
		Serial.print(mem[i]);
		Serial.print("|");
	}

	for(int i=memStart; i < memStart + instructionsCount; i++){
		Serial.print(mem[i]);
		Serial.print("|");
	}
	Serial.println();
}

void exec(){
	int pcNow = mem[REG_PC].toInt();

	if(pcNow >= memStart + instructionsCount){
		Serial.println("stopped execution");
		stateNow = FINISH; return;
	}

	String codHex = mem[pcNow];
	int x = charToHex(codHex.charAt(0));
	int y = charToHex(codHex.charAt(1));
	int s = charToHex(codHex.charAt(2));

	mem[REG_X] = String(hexToChar(x));
	mem[REG_Y] = String(hexToChar(y));

	int w = ULA(x, y, s);
	mem[REG_W] = String(hexToChar(w));
	updateLedState(w);
	mem[REG_PC] = String(pcNow + 1);
	printDump();
}

void setup(){
	Serial.begin(9600);

	pinMode(LED_F3, OUTPUT);
	pinMode(LED_F2, OUTPUT);
	pinMode(LED_F1, OUTPUT);
	pinMode(LED_F0, OUTPUT);

	updateLedState(0000);
}

void loop(){
	if(Serial.available() > 0){
		if(stateNow == WAIT_LOAD){
			String input = Serial.readString();
			load(input);
		}else if(stateNow == WAIT_CONF){
			char answer = Serial.read();
			if(answer == 'y' || answer == 'Y' || answer == 's' || answer == 'S'){
				Serial.println("Starting execution");
				stateNow = EXEC;
			}
		}
	}

	if(stateNow == EXEC){
		exec();
		if(stateNow == EXEC) delay(4000);
	}
}

