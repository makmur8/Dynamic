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
 
#ifndef FLUID_TOKENS
#define FLUID_TOKENS

#include <string>
#include "support/allocators/secure.h"
#include "base58.h"

extern std::string GetWarnings(const std::string& strFor, bool isItToken);

int64_t FluidTokenIssuanceAmount(int64_t nTime, CDynamicAddress &destination, std::string broadcastMessage = GetWarnings("tokens", true));
bool GenerateFluidToken(CDynamicAddress sendToward, 
						CAmount tokenMintAmt, std::string &issuanceString);
bool VerifyAlertToken(std::string uniqueIdentifier);

bool IsFluidParametersSane();
bool InitiateFluidVerify(CDynamicAddress dynamicAddress);
bool DerivePrivateKey(CDynamicAddress universalKey, SecureString &magicKey);
bool VerifyTokenNullification(const CTxOut& txout);

#endif // FLUID_TOKENS
