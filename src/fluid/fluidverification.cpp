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

#include "fluidverification.h"

/** Checks if scriptPubKey is that of the hardcoded addresses */
bool IsItHardcoded(std::string scriptKeyHex) {

	FluidParameters flParams;

#ifdef ENABLE_WALLET /// Assume that address is valid
	CDynamicAddress address(flParams.uniqueAddressKey);
	
	CTxDestination dest = address.Get();
	CScript scriptPubKey = GetScriptForDestination(dest);
		
	return (scriptKeyHex == HexStr(scriptPubKey.begin(), scriptPubKey.end()))
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

/** Are Hardcoded Sovereign Addresses Valid? (NOTE: Does not need wallet) */
bool IsFluidParametersSane() {
	FluidParameters flParams;
	CDynamicAddress address(flParams.uniqueAddressKey);
	
	return (address.IsValid());
}

bool DerivePrivateKey(CDynamicAddress universalKey, SecureString magicKey) {
	LOCK2(cs_main, pwalletMain->cs_wallet);

    std::string strAddress = universalKey;
    CDynamicAddress address;
    if (!address.SetString(strAddress))
        return false;
		
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        return false;
		
    CKey vchSecret;
    if (!pwalletMain->GetKey(keyID, vchSecret))
		return false;
		
    SecureString magicKey = CDynamicSecret(vchSecret).ToString();
    
    return true;
}

bool FluidVerification::VerifyTokenNullification(const CTxOut& txout) {
	if (txout.nValue < 0)
		return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-negative");
}
