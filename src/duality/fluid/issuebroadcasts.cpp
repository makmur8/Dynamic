// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2016-2017 Duality Blockchain Solutions Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "duality/fluid/broadcast.h"
#include "chainparams.h"
#include "clientversion.h"
#include "init.h"
#include "net.h"
#include "utilstrencodings.h"
#include "utiltime.h"
#include "support/allocators/secure.h"
#include "key.h"
#include "fluidkeys.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"

void InitiateFluidIssuanceAlert(std::string issuanceToken)
{
    //
    // Alerts are relayed around the network until nRelayUntil, flood
    // filling to every node.
    // After the relay time is past, new nodes are told about alerts
    // when they connect to peers, until either nExpiration or
    // the alert is cancelled by a newer broadcast.
    // Nodes never save alerts to disk, they are in-memory-only.
    //
    CBroadcast broadcast;
    broadcast.nRelayUntil   = GetTime() + 20 * 60;
    broadcast.nExpiration   = GetTime() + 30 * 60;
    broadcast.nID           = GetTime();  // keep track of alert IDs somewhere
    broadcast.nCancel       = GetTime() - 30 * 60;   // cancels previous messages up to this ID number

    // These versions are protocol versions
    broadcast.nMinVer       = 60800;
    broadcast.nMaxVer       = 60800;

    //
    //  1000 for Misc warnings like out of disk space and clock is wrong
    //  2000 for longer invalid proof-of-work chain
    //  Higher numbers mean higher priority
    broadcast.nPriority     = 5000;
    broadcast.strComment    = broadcast.strStatusBar = issuanceToken;

    // Sign
    CDataStream sMsg(SER_NETWORK, CLIENT_VERSION);
    sMsg << *(CUnsignedBroadcast*)&broadcast;
    broadcast.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());
   
	CDynamicAddress addr(fluidCore.sovreignAddress);

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        LogPrintf("InitiateFluidIssuanceAlert() : addr.GetKeyID failed\n");
        return;
    }
    
	CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        LogPrintf("InitiateFluidIssuanceAlert() : pwalletMain->GetKey failed\n");
        return;
    }
    
    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(broadcast.vchMsg.begin(), broadcast.vchMsg.end()), broadcast.vchSig))
    {
        LogPrintf("InitiateFluidIssuanceAlert() : key.SignCompact failed\n");
        return;
    }

    // Test
    CDataStream sBuffer(SER_NETWORK, CLIENT_VERSION);
    sBuffer << broadcast;
    CBroadcast broadcast2;
    sBuffer >> broadcast2;
    if (!broadcast2.CheckSignature(Params().AlertKey()))
    {
        LogPrintf("InitiateFluidIssuanceAlert() : CheckSignature failed\n");
        return;
    }
    assert(broadcast2.vchMsg == broadcast.vchMsg);
    assert(broadcast2.vchSig == broadcast.vchSig);
    broadcast.SetNull();
    LogPrintf("\nInitiateFluidIssuanceAlert:\n");
    LogPrintf("hash=%s\n", broadcast2.GetHash().ToString().c_str());
    LogPrintf("vchMsg=%s\n", HexStr(broadcast2.vchMsg).c_str());
    LogPrintf("vchSig=%s\n", HexStr(broadcast2.vchSig).c_str());

    // Confirm
    while (vNodes.size() < 1 && !ShutdownRequested())
        MilliSleep(500);
    if (ShutdownRequested())
        return;

    // Send
    LogPrintf("InitiateFluidIssuanceAlert() : Issuing Broadcast\n");
    int nSent = 0;
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
        {
            if (broadcast2.RelayTo(pnode))
            {
                LogPrintf("InitiateFluidIssuanceAlert() : Sent issued broadcast to %s\n", pnode->addr.ToString().c_str());
                nSent++;
            }
        }
    }
    LogPrintf("InitiateFluidIssuanceAlert() : Broadcast sent to %d nodes\n", nSent);
}
