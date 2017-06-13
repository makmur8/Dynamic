// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2016-2017 Duality Blockchain Solutions Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "duality/privatesend/psnotificationinterface.h"

#include "duality/dynode/dynodeman.h"
#include "duality/dynode/dynode-payments.h"
#include "duality/dynode/dynode-sync.h"
#include "duality/governance/governance.h"
#include "duality/instantsend/instantsend.h"
#include "duality/privatesend/privatesend.h"

CPSNotificationInterface::CPSNotificationInterface()
{
}

CPSNotificationInterface::~CPSNotificationInterface()
{
}

void CPSNotificationInterface::UpdatedBlockTip(const CBlockIndex *pindex)
{
    dnodeman.UpdatedBlockTip(pindex);
    privateSendPool.UpdatedBlockTip(pindex);
    instantsend.UpdatedBlockTip(pindex);
    dnpayments.UpdatedBlockTip(pindex);
    governance.UpdatedBlockTip(pindex);
    dynodeSync.UpdatedBlockTip(pindex);
}

void CPSNotificationInterface::SyncTransaction(const CTransaction &tx, const CBlock *pblock)
{
    instantsend.SyncTransaction(tx, pblock);
}
