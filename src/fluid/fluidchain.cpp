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

#include "fluidchain.h"

using namespace std;

std::pair<std::string, std::string> splitInTwo(std::string val, std::string delimiter=":::") {
    std::string arg;
    std::string::size_type pos = val.find(delimiter);
    if(val.npos != pos) {
        arg = val.substr(pos + 1);
        val = val.substr(0, pos);
    }
    return make_pair(val, arg);
}

bool DetermineTokenMinting(SecureString deriveToken, int64_t coinAmount) {
	
	std::string determinationToken = GetWarnings(strStatusBar, true);
	std::pair<std::string, std::string> splitPros = splitInTwo(determinationToken, "::");
    
	std::string coinNumber = splitPros.first
	std::string digestProofs = splitPros.second;
	
	return true;
}
