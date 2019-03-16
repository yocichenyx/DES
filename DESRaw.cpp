// DESRaw.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
using namespace std;

bitset<64> key1, key2, key3, key4, key5;//64位密钥
bitset<48> subkey1[16], subkey2[16], subkey3[16], subkey4[16], subkey5[16];

//初始置换矩阵8*8，将左右32位调换位置
int IP[] = { 
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6,
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17, 9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7 };

//结尾逆置换矩阵8*8，交换左右
int IP_1[] = { 
	40, 8, 48, 16, 56, 24, 64, 32,
	39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41, 9, 49, 17, 57, 25 };

//密钥置换8*7，64->56，去掉奇偶校验位
int PC_1[] = { 
	57, 49, 41, 33, 25, 17, 9,
	1, 58, 50, 42, 34, 26, 18,
	10, 2, 59, 51, 43, 35, 27,
	19, 11, 3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,
	7, 62, 54, 46, 38, 30, 22,
	14, 6, 61, 53, 45, 37, 29,
	21, 13, 5, 28, 20, 12, 4 };

//（左移后）密钥压缩置换8*6，56->48，得到每轮的子密钥
int PC_2[] = { 
	14, 17, 11, 24, 1, 5,
	3, 28, 15, 6, 21, 10,
	23, 19, 12, 4, 26, 8,
	16, 7, 27, 20, 13, 2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32 };

//每轮左移的位数（求子密钥）
int shiftBits[] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

/*-----密码函数 F(Ri-1, Ki)所用表------*/
//Ri-1 32位，而key是48位，无法异或，顾需要32->48
//扩展表8*6---E盒
int E[] = 
{ 
	32, 1, 2, 3, 4, 5,
	4, 5, 6, 7, 8, 9,
	8, 9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32, 1 
};

//S盒8*4*16，每个盒子将6->4 => 将异或完的结果压缩成32位
int S_BOX[8][4][16] = {
	{
		{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
		{0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
		{4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
		{15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
	},
	{
		{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
		{3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
		{0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
		{13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
	},
	{
		{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
		{13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
		{13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
		{1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
	},
	{
		{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
		{13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
		{10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
		{3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
	},
	{
		{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
		{14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
		{4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
		{11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
	},
	{
		{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
		{10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
		{9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
		{4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
	},
	{
		{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
		{13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
		{1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
		{6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
	},
	{
		{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
		{1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
		{7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
		{2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
	}
};

//P置换表，将要压缩完成的32位进行置换
int P[] = 
{ 
	16, 7, 20, 21,
	29, 12, 28, 17,
	1, 15, 23, 26,
	5, 18, 31, 10,
	2, 8, 24, 14,
	32, 27, 3, 9,
	19, 13, 30, 6,
	22, 11, 4, 25 
};

/*
*F函数
*para:右32位，本轮字密钥
*32->48 ^ 子密钥48 => P => 返回
*/
bitset<32> F(bitset<32> R, bitset<48> k)
{
	bitset<48> expandR;
	// 第一步：扩展置换，32 -> 48
	for (int i = 0; i < 48; ++i)
		expandR[47 - i] = R[32 - E[i]];
	// 第二步：异或
	expandR = expandR ^ k;
	// 第三步：查找S_BOX置换表
	bitset<32> output;
	int x = 0;
	for (int i = 0; i < 48; i = i + 6)
	{
		int row = expandR[47 - i] * 2 + expandR[47 - i - 5];
		int col = expandR[47 - i - 1] * 8 + expandR[47 - i - 2] * 4 + expandR[47 - i - 3] * 2 + expandR[47 - i - 4];
		int num = S_BOX[i / 6][row][col];
		bitset<4> binary(num);
		output[31 - x] = binary[3];
		output[31 - x - 1] = binary[2];
		output[31 - x - 2] = binary[1];
		output[31 - x - 3] = binary[0];
		x += 4;
	}
	// 第四步：P-置换，32 -> 32
	bitset<32> tmp = output;
	for (int i = 0; i < 32; ++i)
		output[31 - i] = tmp[32 - P[i]];
	return output;
}

/*
*密钥两部分左移
*para:半部分，左移位数
*/
bitset<28> leftShift(bitset<28> k, const int shift)
{
	bitset<28> tmp = k;
	for (int i = 27; i >= 0; --i)
	{
		if (i - shift < 0)
			k[i] = tmp[i - shift + 28];
		else
			k[i] = tmp[i - shift];
	}
	return k;
}

/*
*生成子密钥*16
*/
void formatSubkey(bitset<64> key, bitset<48> subKey[16])
{
	bitset<56> realKey;
	bitset<28> left;
	bitset<28> right;
	bitset<48> compressKey;
	// 去掉奇偶标记位，将64位密钥变成56位
	for (int i = 0; i < 56; ++i)
		realKey[55 - i] = key[64 - PC_1[i]];
	// 生成子密钥，保存在 subKeys[16] 中
	for (int round = 0; round < 16; ++round)
	{
		// 前28位与后28位
		for (int i = 28; i < 56; ++i)
			left[i - 28] = realKey[i];
		for (int i = 0; i < 28; ++i)
			right[i] = realKey[i];
		// 左移
		left = leftShift(left, shiftBits[round]);
		right = leftShift(right, shiftBits[round]);
		// 压缩置换，由56位得到48位子密钥
		for (int i = 28; i < 56; ++i)
			realKey[i] = left[i - 28];
		for (int i = 0; i < 28; ++i)
			realKey[i] = right[i];
		for (int i = 0; i < 48; ++i)
			compressKey[47 - i] = realKey[56 - PC_2[i]];
		subKey[round] = compressKey;
	}
}

//char数组转为二进制
bitset<64> charToBinary(const char s[8])
{
	bitset<64> bits;
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			bits[i * 8 + j] = ((s[i] >> j) & 1);
	return bits;
}

/*一重加密算法*/
bitset<64> encrypt(bitset<64> &plain, bitset<48> subKey[16])
{
	bitset<64> cipher;
	bitset<64> currentBits;
	bitset<32> left;
	bitset<32> right;
	bitset<32> newLeft;
	//初始置换IP
	for (int i = 0; i < 64; ++i)
		currentBits[63 - i] = plain[64 - IP[i]];
	//获取 Li 和 Ri
	for (int i = 32; i < 64; ++i)
		left[i - 32] = currentBits[i];
	for (int i = 0; i < 32; ++i)
		right[i] = currentBits[i];
	//共16轮迭代
	for (int round = 0; round < 16; ++round)
	{
		newLeft = right;
		right = left ^ F(right, subKey[round]);
		left = newLeft;
	}
	//合并L16和R16，注意合并为 R16L16
	for (int i = 0; i < 32; ++i)
		cipher[i] = left[i];
	for (int i = 32; i < 64; ++i)
		cipher[i] = right[i - 32];
	//结尾置换IP-1
	currentBits = cipher;
	for (int i = 0; i < 64; ++i)
		cipher[63 - i] = currentBits[64 - IP_1[i]];
	//返回密文
	return cipher;
}

/*一重解密算法*/
bitset<64> decrypt(bitset<64> &cipher, bitset<48> subKey[16])
{
	bitset<64> plain;
	bitset<64> currentBits;
	bitset<32> left;
	bitset<32> right;
	bitset<32> newLeft;
	// 初始置换IP
	for (int i = 0; i < 64; ++i)
		currentBits[63 - i] = cipher[64 - IP[i]];
	//获取 Li 和 Ri
	for (int i = 32; i < 64; ++i)
		left[i - 32] = currentBits[i];
	for (int i = 0; i < 32; ++i)
		right[i] = currentBits[i];
	//共16轮迭代（子密钥逆序应用）
	for (int round = 0; round < 16; ++round)
	{
		newLeft = right;
		right = left ^ F(right, subKey[15 - round]);
		left = newLeft;
	}
	//合并L16和R16，注意合并为 R16L16
	for (int i = 0; i < 32; ++i)
		plain[i] = left[i];
	for (int i = 32; i < 64; ++i)
		plain[i] = right[i - 32];
	//结尾置换IP-1
	currentBits = plain;
	for (int i = 0; i < 64; ++i)
		plain[63 - i] = currentBits[64 - IP_1[i]];
	//返回明文
	return plain;
}


//测试
/*
int main(int argc, char* argv[])
{
	//string file_plain = "D:\\homework\\IS\\photo.jpg";
	string file_cipher = "D:\\homework\\IS\\cipher.txt";
	string file_newplain = "D:\\homework\\IS\\newplain.txt";

	string file_plain = argv[1];
	//string k = argv[2];
	//string file_cipher = argv[2];
	//string file_newplain = argv[3];
	//********************************************
	//给出密钥
	string k = "12345678";
	//密钥二进制化
	key = charToBinary(k.c_str());
	//生成16轮子密钥
	formatSubkey();

	//加密文件：不断读文件->加密->写入文件
	bitset<64> plain;
	ifstream in;
	ofstream out;
	in.open(file_plain, ios::binary);
	out.open(file_cipher, ios::binary);
	while (in.read((char*)&plain, sizeof(plain)))
	{
		//明文64位加密
		bitset<64> cipher = encrypt(plain);
		//密文64位写入
		out.write((char*)&cipher, sizeof(cipher));
		plain.reset();
	}
	in.close();
	out.close();

	//解密文件：不断读文件->解密->写入文件
	bitset<64> cipher_1;
	in.open(file_cipher, ios::binary);
	out.open(file_newplain, ios::binary);
	while (in.read((char*)&cipher_1, sizeof(cipher_1)))
	{
		//明文64位加密
		bitset<64> newplain = decrypt(cipher_1);
		//密文64位写入
		out.write((char*)&newplain, sizeof(newplain));
		newplain.reset();
	}
	in.close();
	out.close();

	return 0;
}*/

//密钥的生成与管理
void formatKey()
{
	//给出密钥
	string k1 = "87654321";
	string k2 = "11111111";
	string k3 = "22222222";
	string k4 = "12398765";
	string k5 = "12345678";
	//密钥二进制化
	key1 = charToBinary(k1.c_str());
	key2 = charToBinary(k2.c_str());
	key3 = charToBinary(k3.c_str());
	key4 = charToBinary(k4.c_str());
	key5 = charToBinary(k5.c_str());
	//生成16轮子密钥
	formatSubkey(key1, subkey1);
	formatSubkey(key2, subkey2);
	formatSubkey(key3, subkey3);
	formatSubkey(key4, subkey4);
	formatSubkey(key5, subkey5);
}

//究极加解密
void integED(int pattern, string file_plain, string file_newplain)
{
	//密文路径
	string file_cipher = "D:\\homework\\IS\\cipher.txt";
	//密钥生成
	formatKey();

	/*加密文件：不断读文件->加密->写入文件*/
	ifstream in;
	ofstream out;
	in.open(file_plain, ios::binary);
	out.open(file_cipher, ios::binary);
	bitset<64> plain;
	while (!in.eof())
	{
		in.read((char*)&plain, sizeof(plain));
		bitset<64> cipher;
		//选项
		switch (pattern)
		{
		//一重一密
		case 1:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
		}
		break;
		//两重两密
		case 2:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//两重加密
			cipher = encrypt(cipher, subkey2);
		}
		break;
		//三重两密
		case 32:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//二重解密
			cipher = decrypt(cipher, subkey2);
			//三重加密
			cipher = encrypt(cipher, subkey1);
		}
		break;
		//三重三密
		case 33:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//二重解密
			cipher = decrypt(cipher, subkey2);
			//三重加密
			cipher = encrypt(cipher, subkey3);
		}
		break;
		//五重三密
		case 53:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//二重解密
			cipher = decrypt(cipher, subkey2);
			//三重加密
			cipher = encrypt(cipher, subkey3);
			//四重解密
			cipher = decrypt(cipher, subkey2);
			//五重加密
			cipher = encrypt(cipher, subkey1);
		}
		break;
		//五重五密
		case 55:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//二重解密
			cipher = decrypt(cipher, subkey2);
			//三重加密
			cipher = encrypt(cipher, subkey3);
			//四重解密
			cipher = decrypt(cipher, subkey4);
			//五重加密
			cipher = encrypt(cipher, subkey5);
		}
		break;
		//七重四密
		case 7:
		{
			//一重加密
			cipher = encrypt(plain, subkey1);
			//二重解密
			cipher = decrypt(cipher, subkey2);
			//三重加密
			cipher = encrypt(cipher, subkey3);
			//四重解密
			cipher = decrypt(cipher, subkey4);
			//五重加密
			cipher = encrypt(cipher, subkey3);
			//六重解密
			cipher = decrypt(cipher, subkey2);
			//七重加密
			cipher = encrypt(cipher, subkey1);
		}
		break;
		default:
			break;
		}
		out.write((char*)&cipher, sizeof(cipher));
		plain.reset(); // 置0
	}
	in.close();
	out.close();

	// ------------- 解密 --------------
	in.open(file_cipher, ios::binary);
	out.open(file_newplain, ios::binary);
	while (!in.eof())
	{
		in.read((char*)&plain, sizeof(plain));
		bitset<64> temp;
		//选项
		switch (pattern)
		{
		//一重一密
		case 1:
		{
			//一重解密
			temp = decrypt(plain, subkey1);
		}
		break;
		//一重一密
		case 2:
		{
			//一重解密
			temp = decrypt(plain, subkey2);
			//两重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		//三重两密
		case 32:
		{
			//一重解密
			temp = decrypt(plain, subkey1);
			//二重加密
			temp = encrypt(temp, subkey2);
			//三重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		//三重三密
		case 33:
		{
			//一重解密
			temp = decrypt(plain, subkey3);
			//二重加密
			temp = encrypt(temp, subkey2);
			//三重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		//五重三密
		case 53:
		{
			//一重解密
			temp = decrypt(plain, subkey1);
			//二重加密
			temp = encrypt(temp, subkey2);
			//三重解密
			temp = decrypt(temp, subkey3);
			//四重加密
			temp = encrypt(temp, subkey2);
			//五重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		//五重五密
		case 55:
		{
			//一重解密
			temp = decrypt(plain, subkey5);
			//二重加密
			temp = encrypt(temp, subkey4);
			//三重解密
			temp = decrypt(temp, subkey3);
			//四重加密
			temp = encrypt(temp, subkey2);
			//五重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		//七重四密
		case 7:
		{
			// 一重解密
			temp = decrypt(plain, subkey1);
			//二重加密
			temp = encrypt(temp, subkey2);
			//三重解密
			temp = decrypt(temp, subkey3);
			//四重加密
			temp = encrypt(temp, subkey4);
			//五重解密
			temp = decrypt(temp, subkey3);
			//六重加密
			temp = encrypt(temp, subkey2);
			//七重解密
			temp = decrypt(temp, subkey1);
		}
		break;
		default:
			break;
		}
		out.write((char*)&temp, sizeof(temp));
		plain.reset(); // 置0
	}
	in.close();
	out.close();
}

//修改
int main(int argc, char* argv[])
{
	string file_plain = argv[1];
	string file_newplain = argv[2];
	int pattern = atoi(argv[3]);
	integED(pattern, file_plain, file_newplain);
	return 0;
}