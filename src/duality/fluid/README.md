# Fluid Monetary Regulation Protocol Layer

**Fluid** is a system within Dynamic that will allow the regulation of the currency by allowing for *controlled* minting and destruction 
of Dynamic within the network. Fluid allows the network to maintain economic and programmatical sense allowing the economic team 
responsible for maintaining the currency to modifiy the balances of users and businesses in *limited* manners.

## Licence

> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

## Brief

The *sovereign* of the network will be able to mint new coins by integrating it into **regular mining** - generating and transferring 
them to their destination, the *sovereign* also may neutralize te balance of users by attempting to send a negative vout (Value Out) to 
the address not exceeding current balance to render the address sterile.

The *sovereign* by design will be able to mint and nuke balances but cannot spend **existing** coins of users otherwise (nuking is not 
spending the coins as much as it is wiping them off the map).

Obviously, this sounds Orwellian - and it is, so here's my TODO List:

* Implement Address Indexes (Bitcore API) - we need that here
* Money Supply Determination by inserting calculation into blocks (probs, too late for that but ¯\_(ツ)_/¯)
* Coinbase Injection and Transfer in Block Generation to Provisioned Address
* Whatever else comes to mind...

