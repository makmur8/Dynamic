// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2016-2017 Duality Blockchain Solutions Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "duality/fluid/broadcast.h"
#include "duality/fluid/fluidkeys.h"

#include "base58.h"
#include "clientversion.h"
#include "net.h"
#include "pubkey.h"
#include "timedata.h"
#include "ui_interface.h"
#include "util.h"
#include "utilstrencodings.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"

#include <algorithm>
#include <map>
#include <stdint.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

extern CWallet* pwalletMain;
std::map<uint256, CBroadcast> mapAlerts;
CCriticalSection cs_mapAlerts;

void CUnsignedBroadcast::SetNull()
{
    nVersion = 1;
    nRelayUntil = 0;
    nExpiration = 0;
    nID = 0;
    nCancel = 0;
    setCancel.clear();
    nMinVer = 0;
    nMaxVer = 0;
    setSubVer.clear();
    nPriority = 0;

    strComment.clear();
    strStatusBar.clear();
    strReserved.clear();
}

std::string CUnsignedBroadcast::ToString() const
{
    std::string strSetCancel;
    BOOST_FOREACH(int n, setCancel)
        strSetCancel += strprintf("%d ", n);
    std::string strSetSubVer;
    BOOST_FOREACH(const std::string& str, setSubVer)
        strSetSubVer += "\"" + str + "\" ";
    return strprintf(
        "CBroadcast(\n"
        "    nVersion     = %d\n"
        "    nRelayUntil  = %d\n"
        "    nExpiration  = %d\n"
        "    nID          = %d\n"
        "    nCancel      = %d\n"
        "    setCancel    = %s\n"
        "    nMinVer      = %d\n"
        "    nMaxVer      = %d\n"
        "    setSubVer    = %s\n"
        "    nPriority    = %d\n"
        "    strComment   = \"%s\"\n"
        "    strStatusBar = \"%s\"\n"
        ")\n",
        nVersion,
        nRelayUntil,
        nExpiration,
        nID,
        nCancel,
        strSetCancel,
        nMinVer,
        nMaxVer,
        strSetSubVer,
        nPriority,
        strComment,
        strStatusBar);
}

void CBroadcast::SetNull()
{
    CUnsignedBroadcast::SetNull();
    vchMsg.clear();
    vchSig.clear();
}

bool CBroadcast::IsNull() const
{
    return (nExpiration == 0);
}

uint256 CBroadcast::GetHash() const
{
    return Hash(this->vchMsg.begin(), this->vchMsg.end());
}

bool CBroadcast::IsInEffect() const
{
    return (GetAdjustedTime() < nExpiration);
}

bool CBroadcast::Cancels(const CBroadcast& alert) const
{
    if (!IsInEffect())
        return false; // this was a no-op before 31403
    return (alert.nID <= nCancel || setCancel.count(alert.nID));
}

bool CBroadcast::AppliesTo(int nVersion, const std::string& strSubVerIn) const
{
    // TODO: rework for client-version-embedded-in-strSubVer ?
    return (IsInEffect() &&
            nMinVer <= nVersion && nVersion <= nMaxVer &&
            (setSubVer.empty() || setSubVer.count(strSubVerIn)));
}

bool CBroadcast::AppliesToMe() const
{
    return AppliesTo(PROTOCOL_VERSION, FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<std::string>()));
}

bool CBroadcast::RelayTo(CNode* pnode) const
{
    if (!IsInEffect())
        return false;
    // don't relay to nodes which haven't sent their version message
    if (pnode->nVersion == 0)
        return false;
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        if (AppliesTo(pnode->nVersion, pnode->strSubVer) ||
            AppliesToMe() ||
            GetAdjustedTime() < nRelayUntil)
        {
            pnode->PushMessage(NetMsgType::ALERT, *this);
            return true;
        }
    }
    return false;
}

bool CBroadcast::Sign()
{
    CDataStream sMsg(SER_NETWORK, CLIENT_VERSION);
    sMsg << *(CUnsignedBroadcast*)this;
    vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());
    
    // This shouldn't be necessary, seriously
    /* if (!vchSecret.SetString(GetArg("-alertkey", "")))
    {
        printf("CBroadcast::SignAlert() : vchSecret.SetString failed\n");
        return false;
    } */
	
	// LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);

   	CDynamicAddress addr(fluidCore.sovreignAddress);

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        printf("CBroadcast::SignAlert() : addr.GetKeyID failed\n");
        return false;
    }
    
	CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        printf("CBroadcast::SignAlert() : pwalletMain->GetKey failed\n");
        return false;
    }
    
    if (!key.SignCompact(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
    {
        printf("CBroadcast::SignAlert() : key.SignCompact failed\n");
        return false;
    }

    return true;
}

bool CBroadcast::CheckSignature(const std::vector<unsigned char>& alertKey) const
{
//    CPubKey key(alertKey);
//    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
//        
    CDynamicAddress addr(fluidCore.sovreignAddress);
   
    CKeyID keyID;
    
    if (!addr.GetKeyID(keyID))
		return error("CBroadcast::CheckSignature(): key id derivation failed");
	
    CPubKey pubkey;
    
    if (!pubkey.RecoverCompact(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CBroadcast::CheckSignature(): verify signature failed");
		
    if (!(CDynamicAddress(pubkey.GetID()) == addr))
        return error("CBroadcast::CheckSignature(): verify pubkey address failed");
	
    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedBroadcast*)this;
    return true;
}

CBroadcast CBroadcast::getAlertByHash(const uint256 &hash)
{
    CBroadcast retval;
    {
        LOCK(cs_mapAlerts);
        std::map<uint256, CBroadcast>::iterator mi = mapAlerts.find(hash);
        if(mi != mapAlerts.end())
            retval = mi->second;
    }
    return retval;
}

bool CBroadcast::ProcessAlert(const std::vector<unsigned char>& alertKey, bool fThread)
{
    if (!CheckSignature(alertKey))
        return false;
    if (!IsInEffect())
        return false;

    // alert.nID=max is reserved for if the alert key is
    // compromised. It must have a pre-defined message,
    // must never expire, must apply to all versions,
    // and must cancel all previous
    // alerts or it will be ignored (so an attacker can't
    // send an "everything is OK, don't panic" version that
    // cannot be overridden):
    int maxInt = std::numeric_limits<int>::max();
    if (nID == maxInt)
    {
        if (!(
                nExpiration == maxInt &&
                nCancel == (maxInt-1) &&
                nMinVer == 0 &&
                nMaxVer == maxInt &&
                setSubVer.empty() &&
                nPriority == maxInt &&
                strStatusBar == "URGENT: Alert key compromised, upgrade required"
                ))
            return false;
    }

    {
        LOCK(cs_mapAlerts);
        // Cancel previous alerts
        for (std::map<uint256, CBroadcast>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
        {
            const CBroadcast& alert = (*mi).second;
            if (Cancels(alert))
            {
                LogPrint("alert", "cancelling alert %d\n", alert.nID);
                uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
                mapAlerts.erase(mi++);
            }
            else if (!alert.IsInEffect())
            {
                LogPrint("alert", "expiring alert %d\n", alert.nID);
                uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
                mapAlerts.erase(mi++);
            }
            else
                mi++;
        }

        // Check if this alert has been cancelled
        BOOST_FOREACH(PAIRTYPE(const uint256, CBroadcast)& item, mapAlerts)
        {
            const CBroadcast& alert = item.second;
            if (alert.Cancels(*this))
            {
                LogPrint("alert", "alert already cancelled by %d\n", alert.nID);
                return false;
            }
        }

        // Add to mapAlerts
        mapAlerts.insert(std::make_pair(GetHash(), *this));
        // Notify UI and -alertnotify if it applies to me
        if(AppliesToMe())
        {
            uiInterface.NotifyAlertChanged(GetHash(), CT_NEW);
            Notify(strStatusBar, fThread);
        }
    }

    LogPrint("alert", "accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
    return true;
}

void
CBroadcast::Notify(const std::string& strMessage, bool fThread)
{
    std::string strCmd = GetArg("-alertnotify", "");
    if (strCmd.empty()) return;

    // Alert text should be plain ascii coming from a trusted source, but to
    // be safe we first strip anything not in safeChars, then add single quotes around
    // the whole string before passing it to the shell:
    std::string singleQuote("'");
    std::string safeStatus = SanitizeString(strMessage);
    safeStatus = singleQuote+safeStatus+singleQuote;
    boost::replace_all(strCmd, "%s", safeStatus);

    if (fThread)
        boost::thread t(runCommand, strCmd); // thread runs free
    else
        runCommand(strCmd);
}
