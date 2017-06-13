/**
 * Copyright 2017 Everybody and Nobody Inc.
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation files 
 * (the "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject 
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FLUID_SOVEREIGN_PARAMETERS
#define FLUID_SOVEREIGN_PARAMETERS

#include <string>

/**
 * Complete Disclosure, these are the respective expected values, to be 
 * referenced when testing.
 * 
 * 16:26:09
 * getnewaddress

 * 16:26:09
 * DR9LkhwxMtQHqbLujKAEYjLPisUBED5Xya

 * 16:26:14
 * validateaddress DR9LkhwxMtQHqbLujKAEYjLPisUBED5Xya
 *
 * 16:26:14
￼* {
 *  "isvalid": true,
 *  "address": "DR9LkhwxMtQHqbLujKAEYjLPisUBED5Xya",
 *  "scriptPubKey": "76a914db6e61a69e67f5a5084f65be89accfaff2001fbd88ac",
 *  "ismine": true,
 *  "iswatchonly": false,
 *  "isscript": false,
 *  "pubkey": "03025bcd506f33baf58947a0e37809e1e2a8b75a7b3548d26ee3ab517c9ad2ac3e",
 *  "iscompressed": true,
 *  "account": "",
 *  "hdkeypath": "m/44'/5'/0'/0/143",
 *  "hdchainid": "bf7e1719bea7d456d8ba3d0a355bc096653107e648281489de68888ee3a0a3fd"
 * }
 * 
 * 16:34:55
 * dumpprivkey DR9LkhwxMtQHqbLujKAEYjLPisUBED5Xya

 * 16:34:55
 * MpJVF3uU8uqiKP39SVCV2zr7edhRMn2vbLo3TrZ5UpyZfPbjPgLD
 */

#include "base58.h"
#include "support/allocators/secure.h"

class FluidParameters {
private:

	// These are obviously kiddish, but keep 'em - must be changed every release!
	std::string uniqueKeyStamp =  "I am a nice little goldfish!";
	std::string uniqueBurnStamp = "I really like to play with jellyfish!";
	
	int64_t DeriveSupplyPercentage(int64_t percentage) {
		return 0 * COIN; // Pending Balance Monitoring Implementation
	}
	
	std::string GenerateHex(std::string input) {
		static const char* const lut = "0123456789ABCDEF";

		size_t len = input.length();

		std::string output;
		output.reserve(2 * len);
		for (size_t i = 0; i < len; ++i)
		{
			const unsigned char c = input[i];
			output.push_back(lut[c >> 4]);
			output.push_back(lut[c & 15]);
		}
		
		return output;
	}
	
public:
	CDynamicAddress sovreignAddress = "DL56874aKzfripr8qyatzymHmiignjqrdJ";
	const char* sovreignPublicKey = "76a914db6e61a69e67f5a5084f65be89accfaff2001fbd88ac";
	
	int64_t fluidMintingMinimum = 100 * COIN;
	int64_t fluidMintingMaximum = DeriveSupplyPercentage(10); // Maximum 10%
	
	std::string uniqueKeyStampHex() { return GenerateHex(uniqueKeyStamp); }
	std::string uniqueBurnStampHex() { return GenerateHex(uniqueBurnStamp); }
	void ConvertToHex(std::string &input) { std::string output = GenerateHex(input); input = output; }
};

extern FluidParameters fluidCore;

#endif // FLUID_SOVEREIGN_PARAMETERS
