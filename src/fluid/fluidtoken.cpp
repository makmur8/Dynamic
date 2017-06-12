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

SecureString PrepareIssuanceAnnouncement(int64_t tokenMintAmt, CDynamicAddress tokenDelivery) {
	
	CDynamicAddress addr(flParams.uniqueAddressKey);
	SecureString processMessage = tokenMintAmt + "::" + tokenDelivery;
	
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << processMessage;

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
		return "";

    return processMessage + ":::" + EncodeBase64(&vchSig[0], vchSig.size());
}


bool AnnounceIssuance(std::string preparedString, CDynamicAddress tokenDelivery) {
	
	CDynamicAddress addr(tokenDelivery);
		
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
		return error("Does not refer to a key");

    CKey vchSecret;
    if (!pwalletMain->GetKey(keyID, vchSecret))
		return error("Private Key Unavailable");
		
	if (!BroadcastMintingOperation(preparedString, CDynamicSecret(vchSecret).ToString()));
		return error("Alert Sending Failed!");
	
	return true;
}

CMutableTransaction FluidToken::CreateTokenIssuanceTransaction() {
	CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = defaultScriptKey;
    txNew.vout[0].nValue = DetermineTokenMinting();
    txNew.vin[0].scriptSig = CScript() << nHeight << OP_0;
    
    return txNew;
}

// Doesn't require the sovreign wallet as it is basic verification
bool VerifyAlertToken(CDynamicAddress signageAddress, SecureString uniqueIdentifier)
{
    CDynamicAddress addr(signageAddress);
    if (!addr.IsValid())
		return false;
		
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
		return false;
	
	/*
	 * Decode Unique Identifier as:
	 * Instruction (which has to be then split later on)
	 * Signage for proof of message
	 */
	
	SecureString digestSignature, messageTokenKey;
	
    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(digestSignature, &fInvalid);

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
	
	return false;
}
