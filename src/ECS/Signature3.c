
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_128TH_ARG(__VA_ARGS__)
#define PP_128TH_ARG( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
         _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
         _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
         _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
         _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
         _121,_122,_123,_124,_125,_126,_127,N,...) N
#define PP_RSEQ_N() \
         127,126,125,124,123,122,121,120, \
         119,118,117,116,115,114,113,112,111,110, \
         109,108,107,106,105,104,103,102,101,100, \
         99,98,97,96,95,94,93,92,91,90, \
         89,88,87,86,85,84,83,82,81,80, \
         79,78,77,76,75,74,73,72,71,70, \
         69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0
//^ i googled these. C is weird but I ain't one to judge. This works well.

#define bitArrayType unsigned short
#define bitChunkSize (sizeof(bitArrayType)*8)
#define sigArrayCount ((TOTAL_COMPONENTS/bitChunkSize)+1)

//Credit to these 3 AMAZING bit functions below that i was too stupid and lazy to create myself goes to aniliitb10 on stack overflow
//https://stackoverflow.com/questions/2525310/how-to-define-and-work-with-an-array-of-bits-in-c

void SetBit( bitArrayType A[],  bitArrayType k )
{
	A[k/bitChunkSize] |= 1 << (k%bitChunkSize);  // Set the bit at the k-th position in A[i]
}

/*
static void ClearBit( bitArrayType A[],  bitArrayType k )                
{
	A[k/bitChunkSize] &= ~(1 << (k%bitChunkSize));
}
*/

int _TestBit( bitArrayType A[],  bitArrayType k )
{
	return ( (A[k/bitChunkSize] & (1 << (k%bitChunkSize) )) != 0 ) ;     
}



typedef struct Signature{
	bitArrayType bits[sigArrayCount];
}Signature;

#define print_bits(x)												\
	do {															\
	size_t a__ = (x);												\
	size_t bits__ = sizeof(x) * 8;									\
	while (bits__--) putchar(a__ &(1ULL << bits__) ? '1' : '0');	\
	} while (0)


void PrintSignature(Signature sigIn){
	//printf("%d\n",bitChunkSize);
	//printf("%d\n",sigArrayCount);
	for(int i = 0; i < sigArrayCount; i++){
		print_bits(sigIn.bits[i]);
		//printf("%d",sigIn.bits[i]);
	}
}

Signature _Sig_Create(int n, ...){
	Signature ret = {0};
	//ret.compCount = n;
	va_list ap;
    va_start(ap, n);
	int currentComp;
	while (n--){
		currentComp = va_arg(ap, int);
		if(currentComp < TOTAL_COMPONENTS){//haha this bounds check used to not be here! hahahahahah hah
			SetBit(ret.bits, currentComp);
			//ret.bits[currentComp] = 1;
		}
	}
	va_end(ap);
	return ret;
}

Signature Sig_Or(Signature sig1, Signature sig2){
	Signature ret = {0};
	for(int i = 0; i < sigArrayCount; i++){
		ret.bits[i] = sig1.bits[i] | sig2.bits[i];
	}
	return ret;
}

Signature Sig_Xor(Signature sig1, Signature sig2){
	Signature ret = {0};
	for(int i = 0; i < sigArrayCount; i++){
		ret.bits[i] = sig1.bits[i] ^ sig2.bits[i];
	}
	return ret;
}

Signature Sig_And(Signature sig1, Signature sig2){
	Signature ret = {0};
	for(int i = 0; i < sigArrayCount; i++){
		ret.bits[i] = sig1.bits[i] & sig2.bits[i];
	}
	return ret;
}

bool Sig_Match(Signature a1, Signature query){
	for(int i = 0; i < sigArrayCount; i++){
		if((a1.bits[i] & query.bits[i]) != query.bits[i]){
			return false;
		}
	}
	return true;
}



bool Sig_Equals(Signature s1, Signature s2){
	for(int i = 0; i < sigArrayCount; i++){
		if(s1.bits[i] != s2.bits[i]){
			return false;
		}
	}
	return true;
}

int Sig_TestBit( Signature A,  bitArrayType k ){
	return _TestBit(A.bits, k);
}

int Sig_GetCompCount(Signature sig){
	
	int ret = 0;
	for(int i = 0; i < TOTAL_COMPONENTS; i++){
		ret += _TestBit(sig.bits, i);
	}
	
	return ret;
}
#define Sig_Create(...) _Sig_Create(PP_NARG(__VA_ARGS__), __VA_ARGS__)