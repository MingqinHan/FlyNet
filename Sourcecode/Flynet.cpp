#include "stdlib.h"
#include "memory.h"
#include <iostream>
#include "ap_int.h"
#define CH 5
#define CHin 5
#define CHout 5
#define R 13
#define C 13
#define Rin 29
#define Cin 29
#define K 5

#define R2 13
#define C2 13

#define R3 100
#define C3 100


typedef struct W3s{
	double W3[845][100];
	ap_uint<1> user;
	ap_uint<1> last;


}W3s;

typedef struct Outs{
	double Out[10];
	ap_uint<1> user;
	ap_uint<1> last;


}Outs;

using namespace std;
void Load_In(double* In_ddr, double In[1][Rin][Cin])
{
	for(int L_ri=0; L_ri<Rin; L_ri++)
	{
#pragma HLS PIPELINE
		for(int L_ci=0; L_ci<Cin; L_ci++)
		{
			for(int L_chi=0; L_chi<1; L_chi++)
			{
				In[L_chi][L_ri][L_ci] = *In_ddr++;
			}
		}
	}
	return;
}

void Load_W(double* W_ddr, double W[1][5][K][K])
{
	for(int L_cho=0; L_cho<1; L_cho++)
	{
#pragma HLS PIPELINE
		for(int L_chi=0; L_chi<5; L_chi++)
		{
			for(int L_kr=0; L_kr<K; L_kr++)
			{
				for(int L_kc=0; L_kc<K; L_kc++)
				{
					W[L_cho][L_chi][L_kr][L_kc] = *W_ddr++;
				}
			}
		}
	}	
	return;
}
void Load_W3(double *W3_ddr,double W3[845][100]){

	for(int i=0;i<845;i++){

		for(int j=0;j<100;j++){

			W3[i][j]=*W3_ddr++;

		}
	}

}
void Load_W5(double *W5_ddr,double W5[100][10]){

	for(int i=0;i<100;i++){

		for(int j=0;j<10;j++){

			W5[i][j]=*W5_ddr++;
		}
	}

}

void Load_bias3(double *bias3_ddr,double bias3[100]){

	for(int i=0;i<100;i++){

		bias3[i]=*bias3_ddr++;

	}

}

void Load_bias5(double *bias5_ddr,double bias5[10]){

	for(int i=0;i<10;i++){

		bias5[i]=*bias5_ddr++;

	}

}
void Convolution(double In[1][Rin][Cin], double W[1][5][K][K], double Out[5][R][C])
{
	//padding
//	int padding_=1;
//	double In_[1][29][29];
//	for(int i=0;i<29;i++){
//		for(int j=0;j<29;j++){
//			if ((i == 0) || (j == 0))
//				In_[1][i][j] = 0;
//			else
//				In_[1][i][j]=In[1][i-1][j-1];
//
//		}
//	}
	Kernel_Row:
	for(int kr=0; kr<K; kr++)					
	{
		Kernel_Column:
		for(int kc=0; kc<K; kc++)				
		{
			Row:
			for(int r=0,rin=0; r<R; r++,rin+=2)
			{
				Column:
				for(int c=0,cin=0; c<C; c++,cin+=2)
				{		
#pragma HLS PIPELINE
					Output_Channel_HW:
					for(int cho_hw=0; cho_hw<5; cho_hw++)
					{
						Input_Channel_HW:
						for(int chi_hw=0; chi_hw<1; chi_hw++)
						{
							Out[cho_hw][r][c] += In[chi_hw][rin+kr][cin+kc] * W[chi_hw][cho_hw][kr][kc];
						}
					}
				}
			}
		}
	}
	return;
}


void SquareLayer2(double In[5][R][C], double Out[5][R2][C2]){

	for(int c=0;c<CH;c++){

	for(int i=0;i<R;i++){

		for(int j=0;j<C;j++){

			double temp=In[c][i][j];

			Out[c][i][j]=temp*temp;

		}
	}

	}
}

void Layer3(double In[5][R2][C2],double W[845][100],double Out[R3],double bias[100]){


	double In_[845]={0};
	//transpose
	int index=0;
	for(int k=0;k<5;k++){
		for(int i=0;i<R2;i++){
			for(int j=0;j<C2;j++){
				In_[index]=In[k][i][j];
				index++;
			}
		}
	}


	//matrix multiplex
	for(int i1=0;i1<100;i1++){
		for(int j1=0;j1<845;j1++){
			Out[i1]+=(In_[j1]*W[j1][i1]);
		}
		Out[i1]+=bias[i1];
	}
}

void SquareLayer4(double In[3], double Out[R3]){
	for(int i=0;i<R3;i++){
			double temp=In[i];
			Out[i]=temp*temp;
	}
}


void Layer5(double In[R3],double W[100][10],double Out[10],double bias[10]){
	//matrix multiplex
	for(int i=0;i<10;i++){
		Out[i]=0;
		for(int j=0;j<100;j++){
			Out[i]+=In[j]*W[j][i];
		}
		Out[i]+=bias[i];
	}
}

void Offload_Out(double* Out_ddr, double Out[4][R][C])
{
	for(int L_ro=0; L_ro<R; L_ro++)
	{
#pragma HLS PIPELINE
		for(int L_co=0; L_co<C; L_co++)
		{
			for(int L_cho=0; L_cho<4; L_cho++)
			{
				*Out_ddr++ = Out[L_cho][L_ro][L_co];
			}
		}
	}
	return;
}

//void Process(double* In_ddr, double* W_ddr, double In[1][Rin][Cin], double W[1][5][K][K],double W3[845][100],double W5[100][10],double bias3[100], double bias5[10],double Out[10])
//{
//#pragma HLS DATAFLOW
//	//Load In tile from DRAM
//	Load_In(In_ddr, In);
//	//Load W tile from DRAM
//	Load_W(W_ddr, W);
//
//
//	double Out1[5][R][C];
//	double Out2[5][R2][C2];
//	double Out3[R3];
//	double Out4[100];
//
//	Convolution(In, W, Out1);
//	SquareLayer2(Out1, Out2);
//	Layer3(Out2,W3,Out3,bias3);
//	SquareLayer4(Out3, Out4);
//	Layer5(Out4,W5,Out,bias5);
//	return;
//}
void Process(double In[1][Rin][Cin], double W[1][5][K][K],double W3[845][100],double W5[100][10],double bias3[100], double bias5[10],double Out[10])
{
	//double *In_ddr, double* W_ddr, double *W3_ddr,double *W5_ddr,double *bias3_ddr,double *bias5_ddr,
//#pragma HLS DATAFLOW
	//Load In tile from DRAM
	//#pragma HLS DATAFLOW
	//	Load In tile from DRAM
//	Load_In(input.In_ddr, In);
//	//Load W tile from DRAM
//	Load_W(input.W_ddr, W);
//	Load_W3(input.W3_ddr,W3);
//	Load_W5(input.W5_ddr,W5);
//	Load_bias3(input.bias3_ddr,bias3);
//	Load_bias5(input.bias5_ddr,bias5);
	double Out1[5][R][C]={0};
	double Out2[5][R2][C2]={0};
	double Out3[R3]={0};
	double Out4[100]={0};

	Convolution(In,W, Out1);
	SquareLayer2(Out1, Out2);
	Layer3(Out2,W3,Out3,bias3);
	SquareLayer4(Out3, Out4);
	Layer5(Out4,W5,Out,bias5);
	return;
}
void flynet(double In[1][Rin][Cin], double W[1][5][K][K],W3s W3,double W5[100][10],double bias3[100], double bias5[10],Outs Out[10])
{
//#pragma HLS DATA_PACK variable=input
//double* In_ddr, double* W_ddr, double *W3_ddr,double *W5_ddr,double *bias3_ddr,double *bias5_ddr,
//#pragma HLS INTERFACE m_axi depth=841 port=In offset=slave bundle=INPUTI
//#pragma HLS INTERFACE m_axi depth=125 port=W offset=slave bundle=INPUTW
//#pragma HLS INTERFACE m_axi depth=845100 port=W3 offset=slave bundle=INPUTW
//#pragma HLS INTERFACE m_axi depth=1000 port=W5 offset=slave bundle=INPUTW
//#pragma HLS INTERFACE m_axi depth=100 port=bias3 offset=slave bundle=INPUTb
//#pragma HLS INTERFACE m_axi depth=10 port=bias5 offset=slave bundle=INPUT
//#pragma HLS INTERFACE m_axi depth=10  port=Out offset=slave bundle=OUTPUT
//#pragma HLS INTERFACE s_axilite port=return
//


#pragma HLS INTERFACE m_axi depth=841 port=In offset=0x100 bundle=INPUTI
#pragma HLS INTERFACE m_axi depth=125 port=W  offset=0x4000 bundle=INPUTW
#pragma HLS INTERFACE axis port=W3  bundle=INPUTW3
#pragma HLS INTERFACE m_axi depth=1000 port=W5  offset=0x10000 bundle=INPUTW
#pragma HLS INTERFACE m_axi depth=100 port=bias3 offset=0x15000 bundle=INPUTb
#pragma HLS INTERFACE m_axi depth=10 port=bias5 offset=0x16000 bundle=INPUT
#pragma HLS INTERFACE m_axi depth=10  port=Out offset=0x17000  bundle=OUTPUT
#pragma HLS INTERFACE s_axilite port=return
//	static double In[1][Rin][Cin];
//#pragma HLS array_partition variable=In complete dim=1
//	static double Out[10];
//#pragma HLS array_partition variable=Out complete dim=1
//	static double W[1][5][K][K];
//#pragma HLS array_partition variable=W complete dim=1
//#pragma HLS array_partition variable=W complete dim=2
//
//	//static double W3[845][100];
//
//	static double W5[100][10];
//
//	static double bias3[100];
//
//	static double bias5[10];


	Process(In, W,W3.W3,W5,bias3,bias5,Out->Out);

		//Offload Out tile to DRAM
		//Offload_Out(Out_ddr, Out);

	return;
}
