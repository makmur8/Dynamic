// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DYNAMIC_AMOUNT_H
#define DYNAMIC_AMOUNT_H

#include "serialize.h"

#include <stdlib.h>
#include <string>

typedef int64_t CAmount;

static const CAmount COIN = 100000000;
static const CAmount CENT = 1000000;
static const CAmount SUBCENT = 100;
static const CAmount MIN_TX_FEE = CENT;
static const CAmount MIN_MULTISIG_NAME_FEE = SUBCENT;

// No amount larger than this (in satoshi) is valid.
static const CAmount MAX_MONEY = std::numeric_limits<int64_t>::max();
inline bool MoneyRange(const CAmount& nValue) {
    return (nValue >= 0 && nValue <= MAX_MONEY);
}

#endif //  DYNAMIC_AMOUNT_H
