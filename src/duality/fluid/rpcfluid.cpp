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

/** Mint given amount of Fluid to given address  */
UniValue issuefluidtoaddress(const UniValue& params, bool fHelp)
{
    if (!EnsureWalletIsAvailable(fHelp))
        return NullUniValue;
    
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "issuefluidtoaddress \"dynamicaddress\" \"amount\"\n"
            "\Mint Fluid Tokens to an address on the network\n"
            "\nArguments:\n"
            "1. \"dynamicaddress\"  (string, required) The dynamic address to mint the coins toward.\n"
            "2. \"account\"         (numeric or string, required) The amount of coins to be minted.\n"
            "\nExamples:\n"
            + HelpExampleCli("issuefluidtoaddress", "\"D5nRy9Tf7Zsef8gMGL2fhWA9ZslrP4K5tf\" \"123.456\"")
            + HelpExampleRpc("issuefluidtoaddress", "\"D5nRy9Tf7Zsef8gMGL2fhWA9ZslrP4K5tf\", \"123.456\"")
        );
    // params[0].get_str() - Address to send 
    // params[1].get_str() - Coins to mint
    
    EnsureWalletIsUnlocked();
    
	if (!InitiateFluidVerify(flParams.uniqueAddressKey))
		return false;
	
	SecureString magicKey;
	
	if (!DerivePrivateKey(flParams.uniqueAddressKey, magicKey))
		return false;
    
    return false;
}

/** Destroy Dynamic from a given address by negating balance */
UniValue sendfluidtovaccum(const UniValue& params, bool fHelp)
{
    if (!EnsureWalletIsAvailable(fHelp))
        return NullUniValue;
    
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw std::runtime_error(
            "sendfluidtovaccum \"dynamicaddress\" \"amount\"\n"
            "\Destroys the amount of Dynamic owned by the address.\n"
            "\nArguments:\n"
            "1. \"dynamicaddress\"  (string, required) The dynamic address to be targeted.\n"
            "2. \"account\"         (numeric or string, required) The amount of coins to be vaccumed.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendfluidtovaccum", "\"D5nRy9Tf7Zsef8gMGL2fhWA9ZslrP4K5tf\" \"123.456\"")
            + HelpExampleRpc("sendfluidtovaccum", "\"D5nRy9Tf7Zsef8gMGL2fhWA9ZslrP4K5tf\", \"123.456\"")
        );

    // params[0].get_str() - Address to send 
    // params[1].get_str() - Coins to mint
    
    EnsureWalletIsUnlocked();
    
	if (!InitiateFluidVerify(flParams.uniqueAddressKey))
		return false;
	
	SecureString magicKey;
	
	if (!DerivePrivateKey(flParams.uniqueAddressKey, magicKey))
		return false;
    
    return false;
}

/** Query amount of Fluid minted to sovereign on network */
UniValue issuedfluid(const UniValue& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw std::runtime_error(
            "issuedfluid\n"
            "\nReturns the amount of Fluid Tokens issued/minted in total\n"
            "\nResult:\n"
            "n    (numeric) The current circulation Fluid Tokens\n"
            "\nExamples:\n"
            + HelpExampleCli("issuedfluid", "")
            + HelpExampleRpc("issuedfluid", "")
        );

    LOCK(cs_main);
    
    // chainActive.Tip()->nFluidSupply; (will require block modification)
    
    return 0 * COIN;
}
