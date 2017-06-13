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

#include "fluidtoken.h"

#include "amount.h"
#include "base58.h"
#include "chain.h"
#include "core_io.h"
#include "util.h"
#include "utilmoneystr.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "main.h"
#include "init.h"
#include "duality/keepass.h"
#include "main.h"
#include "net.h"
#include "netbase.h"
#include "policy/rbf.h"
#include "api/rpc/rpcserver.h"
#include "timedata.h"
#include "fluidkeys.h"

#include <boost/algorithm/string.hpp>

int64_t FluidTokenIssuanceAmount(int64_t nTime, CDynamicAddress &destination, std::string broadcastMessage) {
		
	// Don't even bother with data that isn't even valid!
	if (broadcastMessage == "IncorrectData")
		return 0 * COIN;
	
	if (!IsHex(broadcastMessage))
		return 0 * COIN;

	int len = broadcastMessage.length();
	std::string decodedString;
	
	for(int i=0; i< len; i+=2)
	{
		std::string byte = broadcastMessage.substr(i,2);
		char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
		decodedString.push_back(chr);
	}
	
	if(!VerifyAlertToken(decodedString))
		return 0 * COIN;
	
	std::vector<std::string> strs, ptrs;
	std::string::size_type size, sizeX;
	boost::split(strs, decodedString, boost::is_any_of(" "));
	boost::split(ptrs, strs.at(0), boost::is_any_of("::"));
	
	CAmount coinAmount = std::stoi (ptrs.at(0),&size);
	int64_t issuanceTime = std::stoi (ptrs.at(2),&sizeX);
	std::string recipientAddress = ptrs.at(4);
	
	destination.SetString(recipientAddress);
	
	// if (GetTime() + 15 * 60 < issuanceTime || GetTime() - 15 * 60 > issuanceTime)
	//	return 0 * COIN;
		
	if(!destination.IsValid())
		return 0 * COIN;
	
	if(coinAmount < fluidCore.fluidMintingMinimum)
		return 0 * COIN;
	
	return coinAmount;
}

bool GenerateFluidToken(CDynamicAddress sendToward, 
						CAmount tokenMintAmt, std::string &issuanceString) {
	
	if(!sendToward.IsValid())
		return false;
	
	std::string unsignedMessage;
	unsignedMessage = std::to_string(tokenMintAmt) + "::" + std::to_string(GetTime()) + "::" + sendToward.ToString();

	CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << unsignedMessage;
    
   	CDynamicAddress addr(fluidCore.sovreignAddress);

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
		return false;

	CKey key;
    if (!pwalletMain->GetKey(keyID, key))
		return false;

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
		return false;
	else
		issuanceString = unsignedMessage + " " + EncodeBase64(&vchSig[0], vchSig.size());
	
	if(tokenMintAmt < fluidCore.fluidMintingMinimum)
		return 0 * COIN;
	
	fluidCore.ConvertToHex(issuanceString);
		
    return true;
}

bool VerifyAlertToken(std::string uniqueIdentifier)
{
    CDynamicAddress addr(fluidCore.sovreignAddress);
    
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
		return false;
	
	std::vector<std::string> strs;
	boost::split(strs, uniqueIdentifier, boost::is_any_of(" "));
	
	std::string digestSignature = strs.at(1);
	std::string messageTokenKey = strs.at(0);
		
    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(digestSignature.c_str(), &fInvalid);

    if (fInvalid)
		return false;
	    
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << messageTokenKey;

    CPubKey pubkey;
    
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
		return false;
		
    if (!(CDynamicAddress(pubkey.GetID()) == addr))
		return false;
	
	return true;
}

/** Checks if scriptPubKey is that of the hardcoded addresses */
bool IsItHardcoded(std::string givenScriptPubKey) {
#ifdef ENABLE_WALLET /// Assume that address is valid
	CDynamicAddress address(fluidCore.sovreignAddress);
	
	CTxDestination dest = address.Get();
	CScript scriptPubKey = GetScriptForDestination(dest);
		
	return (givenScriptPubKey == HexStr(scriptPubKey.begin(), scriptPubKey.end()));
#else /// Shouldn't happen as it musn't be called if no wallet
	return false;
#endif
}

/** Does client instance own address for engaging in processes - required for RPC (PS: NEEDS wallet) */
bool InitiateFluidVerify(CDynamicAddress dynamicAddress) {
#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
	CDynamicAddress address(dynamicAddress);
	
	if (address.IsValid()) {
		CTxDestination dest = address.Get();
		CScript scriptPubKey = GetScriptForDestination(dest);
		
		/** Additional layer of verification, is probably redundant */
		if (IsItHardcoded(HexStr(scriptPubKey.begin(), scriptPubKey.end()))) {
			isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
			return ((mine & ISMINE_SPENDABLE) ? true : false); // We must be able to sign transactions with sovereign key
		}
	}
	
	return false;
#else
	// LogPrint that Wallet cannot be accessed, cannot continue ahead!
    return false;
#endif
}
bool IsFluidParametersSane() {
	CDynamicAddress address(fluidCore.sovreignAddress);
	return (address.IsValid());
}

bool DerivePrivateKey(CDynamicAddress universalKey, SecureString &magicKey) {
	LOCK2(cs_main, pwalletMain->cs_wallet);

    std::string strAddress = universalKey.ToString();
    CDynamicAddress address;
    if (!address.SetString(strAddress))
        return false;
		
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        return false;
		
    CKey vchSecret;
    if (!pwalletMain->GetKey(keyID, vchSecret))
		return false;
		
    magicKey = CDynamicSecret(vchSecret).ToString().c_str();
    
    return true;
}

