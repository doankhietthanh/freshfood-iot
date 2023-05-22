#ifndef ETHEREUM_PROVIDER_H
#define ETHEREUM_PROVIDER_H

#define ETHERSCAN_TX "https://sepolia.etherscan.io/tx/"

#include <WiFi.h>
#include <Web3.h>
#include <Util.h>
#include <Contract.h>

// static Web3 *web3 = new Web3(SEPOLIA_ID, "izbNl_dUUKDalhy6d8SG2z0Sg6Z2dq9G");

class EthereumProvider
{
private:
    const char *privateKey;
    Web3 *web3; // private key of my wallet

    string GenerateIntToHex(int number);

public:
    const char *contractAddress;
    const char *walletAddress;
    EthereumProvider(const char *privateKey, const char *walletAddress);
    ~EthereumProvider();

    void setupWeb3(Web3 *web3);
    void setContractAddress(const char *contractAddress);
    double getBalance(const char *address);
    string sendETHToAddress(uint256_t amount);
    string checkStr(string txt);

    string addLog(uint256_t productId, string objectId, string hash, string location, uint256_t timestamp);
    string registerOwner(string name, string description);
};

#endif