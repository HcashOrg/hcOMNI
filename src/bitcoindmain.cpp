// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#ifdef _WIN32
#include "config/bitcoin-configWin.h"
#else
#include "config/bitcoin-config.h"
#endif
#endif

int main_actual(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return (main_actual(argc, argv));
}

