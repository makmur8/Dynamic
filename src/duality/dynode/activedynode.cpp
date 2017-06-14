// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2016-2017 Duality Blockchain Solutions Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "duality/dynode/activedynode.h"

#include "duality/dynode/dynode.h"
#include "duality/dynode/dynode-sync.h"
#include "duality/dynode/dynodeman.h"
#include "protocol.h"


extern CWallet* pwalletMain;

// Keep track of the active Dynode
CActiveDynode activeDynode;

void CActiveDynode::ManageState()
{
    LogPrint("Dynode", "CActiveDynode::ManageState -- Start\n");
    if(!fDyNode) {
        LogPrint("Dynode", "CActiveDynode::ManageState -- Not a Dynode, returning\n");
        return;
    }

    if(Params().NetworkIDString() != CBaseChainParams::REGTEST && !dynodeSync.IsBlockchainSynced()) {
        nState = ACTIVE_DYNODE_SYNC_IN_PROCESS;
        LogPrintf("CActiveDynode::ManageState -- %s: %s\n", GetStateString(), GetStatus());
        return;
    }

    if(nState == ACTIVE_DYNODE_SYNC_IN_PROCESS) {
        nState = ACTIVE_DYNODE_INITIAL;
    }

    LogPrint("Dynode", "CActiveDynode::ManageState -- status = %s, type = %s, pinger enabled = %d\n", GetStatus(), GetTypeString(), fPingerEnabled);

    if(eType == DYNODE_UNKNOWN) {
        ManageStateInitial();
    }

    if(eType == DYNODE_REMOTE) {
        ManageStateRemote();
    } else if(eType == DYNODE_LOCAL) {
        // Try Remote Start first so the started local Dynode can be restarted without recreate Dynode broadcast.
        ManageStateRemote();
        if(nState != ACTIVE_DYNODE_STARTED)
            ManageStateLocal();
    }

    SendDynodePing();
}

std::string CActiveDynode::GetStateString() const
{
    switch (nState) {
        case ACTIVE_DYNODE_INITIAL:         return "INITIAL";
        case ACTIVE_DYNODE_SYNC_IN_PROCESS: return "SYNC_IN_PROCESS";
        case ACTIVE_DYNODE_INPUT_TOO_NEW:   return "INPUT_TOO_NEW";
        case ACTIVE_DYNODE_NOT_CAPABLE:     return "NOT_CAPABLE";
        case ACTIVE_DYNODE_STARTED:         return "STARTED";
        default:                               return "UNKNOWN";
    }
}

std::string CActiveDynode::GetStatus() const
{
    switch (nState) {
        case ACTIVE_DYNODE_INITIAL:         return "Node just started, not yet activated";
        case ACTIVE_DYNODE_SYNC_IN_PROCESS: return "Sync in progress. Must wait until sync is complete to start Dynode";
        case ACTIVE_DYNODE_INPUT_TOO_NEW:   return strprintf("Dynode input must have at least %d confirmations", Params().GetConsensus().nDynodeMinimumConfirmations);
        case ACTIVE_DYNODE_NOT_CAPABLE:     return "Not capable Dynode: " + strNotCapableReason;
        case ACTIVE_DYNODE_STARTED:         return "Dynode successfully started";
        default:                                return "Unknown";
    }
}

std::string CActiveDynode::GetTypeString() const
{
    std::string strType;
    switch(eType) {
    case DYNODE_UNKNOWN:
        strType = "UNKNOWN";
        break;
    case DYNODE_REMOTE:
        strType = "REMOTE";
        break;
    case DYNODE_LOCAL:
        strType = "LOCAL";
        break;
    default:
        strType = "UNKNOWN";
        break;
    }
    return strType;
}

bool CActiveDynode::SendDynodePing()
{
    if(!fPingerEnabled) {
        LogPrint("Dynode", "CActiveDynode::SendDynodePing -- %s: Dynode ping service is disabled, skipping...\n", GetStateString());
        return false;
    }

    if(!dnodeman.Has(vin)) {
        strNotCapableReason = "Dynode not in Dynode list";
        nState = ACTIVE_DYNODE_NOT_CAPABLE;
        LogPrintf("CActiveDynode::SendDynodePing -- %s: %s\n", GetStateString(), strNotCapableReason);
        return false;
    }

    CDynodePing dnp(vin);
    if(!dnp.Sign(keyDynode, pubKeyDynode)) {
        LogPrintf("CActiveDynode::SendDynodePing -- ERROR: Couldn't sign Dynode Ping\n");
        return false;
    }

    // Update lastPing for our Dynode in Dynode list
    if(dnodeman.IsDynodePingedWithin(vin, DYNODE_MIN_DNP_SECONDS, dnp.sigTime)) {
        LogPrintf("CActiveDynode::SendDynodePing -- Too early to send Dynode Ping\n");
        return false;
    }

    dnodeman.SetDynodeLastPing(vin, dnp);

    LogPrintf("CActiveDynode::SendDynodePing -- Relaying ping, collateral=%s\n", vin.ToString());
    dnp.Relay();

    return true;
}

void CActiveDynode::ManageStateInitial()
{
    LogPrint("Dynode", "CActiveDynode::ManageStateInitial -- status = %s, type = %s, pinger enabled = %d\n", GetStatus(), GetTypeString(), fPingerEnabled);
    // Check that our local network configuration is correct
    if (!fListen) {
        // listen option is probably overwritten by smth else, no good
        nState = ACTIVE_DYNODE_NOT_CAPABLE;
        strNotCapableReason = "Dynode must accept connections from outside. Make sure listen configuration option is not overwritten by some another parameter.";
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: %s\n", GetStateString(), strNotCapableReason);
        return;
    }

    LogPrint("Dynode", "CActiveDynode::ManageStateInitial -- status = %s, type = %s, pinger enabled = %d\n", GetStatus(), GetTypeString(), fPingerEnabled);
    // Check that our local network configuration is correct
    BOOST_FOREACH(CNode* pnode, vNodes) {
    if (pnode->addr.IsIPv6()) {
        // listen option is probably overwritten by smth else, no good
        LogPrintf("Dynodes cannot use IPv6, you must use IPv4 for Dynode connectivity.");
        return;
        }
    }

    bool fFoundLocal = false;
    {
        LOCK(cs_vNodes);

        // First try to find whatever local address is specified by externalip option
        fFoundLocal = GetLocal(service) && CDynode::IsValidNetAddr(service);
        if(!fFoundLocal) {
            // nothing and no live connections, can't do anything for now
            if (vNodes.empty()) {
                nState = ACTIVE_DYNODE_NOT_CAPABLE;
                strNotCapableReason = "Can't detect valid external address. Will retry when there are some connections available.";
                LogPrintf("CActiveDynode::ManageStateInitial -- %s: %s\n", GetStateString(), strNotCapableReason);
                return;
            }
            // We have some peers, let's try to find our local address from one of them
            BOOST_FOREACH(CNode* pnode, vNodes) {
                if (pnode->fSuccessfullyConnected && pnode->addr.IsIPv4()) {
                    fFoundLocal = GetLocal(service, &pnode->addr) && CDynode::IsValidNetAddr(service);
                    if(fFoundLocal) break;
                }
            }
        }
    }

    if(!fFoundLocal) {
        nState = ACTIVE_DYNODE_NOT_CAPABLE;
        strNotCapableReason = "Can't detect valid external address. Please consider using the externalip configuration option if problem persists. Make sure to use IPv4 address only.";
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: %s\n", GetStateString(), strNotCapableReason);
        return;
    }
    
    int mainnetDefaultPort = Params(CBaseChainParams::MAIN).GetDefaultPort();
    
    if(Params().NetworkIDString() == CBaseChainParams::MAIN) {
        if(service.GetPort() != mainnetDefaultPort) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = strprintf("Invalid port: %u - only 31300 is supported on mainnet.", service.GetPort());
            LogPrintf("CActiveDynode::ManageStatus() - not capable: %s\n", strNotCapableReason);
            return;
        }
    }

    if(Params().NetworkIDString() != CBaseChainParams::MAIN) {
        if(service.GetPort() == mainnetDefaultPort) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = strprintf("Invalid port: %u - 31300 is only supported on mainnet.", service.GetPort());
            LogPrintf("CActiveDynode::ManageStatus() - not capable: %s\n", strNotCapableReason);
            return;
        }
    }

    LogPrintf("CActiveDynode::ManageStateInitial -- Checking inbound connection to '%s'\n", service.ToString());

    if(!ConnectNode((CAddress)service, NULL, true)) {
        nState = ACTIVE_DYNODE_NOT_CAPABLE;
        strNotCapableReason = "Could not connect to " + service.ToString();
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: %s\n", GetStateString(), strNotCapableReason);
        return;
    }

    // Default to REMOTE
    eType = DYNODE_REMOTE;

    // Check if wallet funds are available
    if(!pwalletMain) {
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: Wallet not available\n", GetStateString());
        return;
    }

    if(pwalletMain->IsLocked()) {
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: Wallet is locked\n", GetStateString());
        return;
    }

    if(pwalletMain->GetBalance() < 1000*COIN) {
        LogPrintf("CActiveDynode::ManageStateInitial -- %s: Wallet balance is < 1000 DYN\n", GetStateString());
        return;
    }

    // Choose coins to use
    CPubKey pubKeyCollateral;
    CKey keyCollateral;

    // If collateral is found switch to LOCAL mode
    if(pwalletMain->GetDynodeVinAndKeys(vin, pubKeyCollateral, keyCollateral)) {
        eType = DYNODE_LOCAL;
    }

    LogPrint("Dynode", "CActiveDynode::ManageStateInitial -- End status = %s, type = %s, pinger enabled = %d\n", GetStatus(), GetTypeString(), fPingerEnabled);
}

void CActiveDynode::ManageStateRemote()
{
    LogPrint("Dynode", "CActiveDynode::ManageStateRemote -- Start status = %s, type = %s, pinger enabled = %d, pubKeyDynode.GetID() = %s\n", 
             GetStatus(), fPingerEnabled, GetTypeString(), pubKeyDynode.GetID().ToString());

    dnodeman.CheckDynode(pubKeyDynode, true);
    dynode_info_t infoDn = dnodeman.GetDynodeInfo(pubKeyDynode);
    if(infoDn.fInfoValid) {
        if(infoDn.nProtocolVersion != PROTOCOL_VERSION) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = "Invalid protocol version";
            LogPrintf("CActiveDynode::ManageStateRemote -- %s: %s\n", GetStateString(), strNotCapableReason);
            return;
        }
        if(service != infoDn.addr) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = "Broadcasted IP doesn't match our external address. Make sure you issued a new broadcast if IP of this Dynode changed recently.";
            LogPrintf("CActiveDynode::ManageStateRemote -- %s: %s\n", GetStateString(), strNotCapableReason);
            return;
        }
        if(!CDynode::IsValidStateForAutoStart(infoDn.nActiveState)) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = strprintf("Dynode in %s state", CDynode::StateToString(infoDn.nActiveState));
            LogPrintf("CActiveDynode::ManageStateRemote -- %s: %s\n", GetStateString(), strNotCapableReason);
            return;
        }
        if(nState != ACTIVE_DYNODE_STARTED) {
            LogPrintf("CActiveDynode::ManageStateRemote -- STARTED!\n");
            vin = infoDn.vin;
            service = infoDn.addr;
            fPingerEnabled = true;
            nState = ACTIVE_DYNODE_STARTED;
        }
    }
    else {
        nState = ACTIVE_DYNODE_NOT_CAPABLE;
        strNotCapableReason = "Dynode not in Dynode list";
        LogPrintf("CActiveDynode::ManageStateRemote -- %s: %s\n", GetStateString(), strNotCapableReason);
    }
}

void CActiveDynode::ManageStateLocal()
{
    LogPrint("Dynode", "CActiveDynode::ManageStateLocal -- status = %s, type = %s, pinger enabled = %d\n", GetStatus(), GetTypeString(), fPingerEnabled);
    if(nState == ACTIVE_DYNODE_STARTED) {
        return;
    }

    // Choose coins to use
    CPubKey pubKeyCollateral;
    CKey keyCollateral;

    if(pwalletMain->GetDynodeVinAndKeys(vin, pubKeyCollateral, keyCollateral)) {
        int nInputAge = GetInputAge(vin);
        if(nInputAge < Params().GetConsensus().nDynodeMinimumConfirmations){
            nState = ACTIVE_DYNODE_INPUT_TOO_NEW;
            strNotCapableReason = strprintf(_("%s - %d confirmations"), GetStatus(), nInputAge);
            LogPrintf("CActiveDynode::ManageStateLocal -- %s: %s\n", GetStateString(), strNotCapableReason);
            return;
        }

        {
            LOCK(pwalletMain->cs_wallet);
            pwalletMain->LockCoin(vin.prevout);
        }

        CDynodeBroadcast dnb;
        std::string strError;
        if(!CDynodeBroadcast::Create(vin, service, keyCollateral, pubKeyCollateral, keyDynode, pubKeyDynode, strError, dnb)) {
            nState = ACTIVE_DYNODE_NOT_CAPABLE;
            strNotCapableReason = "Error creating Dynode broadcast: " + strError;
            LogPrintf("CActiveDynode::ManageStateLocal -- %s: %s\n", GetStateString(), strNotCapableReason);
            return;
        }

        fPingerEnabled = true;
        nState = ACTIVE_DYNODE_STARTED;

        //update to Dynode list
        LogPrintf("CActiveDynode::ManageStateLocal -- Update Dynode List\n");
        dnodeman.UpdateDynodeList(dnb);
        dnodeman.NotifyDynodeUpdates();

        //send to all peers
        LogPrintf("CActiveDynode::ManageStateLocal -- Relay broadcast, vin=%s\n", vin.ToString());
        dnb.Relay();
    }
}
