#ifndef ETHEREUM_PROVIDER_H
#define ETHEREUM_PROVIDER_H

#define ETHERSCAN_TX "https://sepolia.etherscan.io/tx/"

#include <WiFi.h>
#include <Web3.h>
#include <Util.h>
#include <Contract.h>

#define DATA_LENGTH 64

// static Web3 *web3 = new Web3(SEPOLIA_ID, "izbNl_dUUKDalhy6d8SG2z0Sg6Z2dq9G");

class EthereumProvider
{
private:
    const char *privateKey;
    Web3 *web3; // private key of my wallet

    string GenerateIntToHex(int number);
    string RemoveBitZero(const std::string &hex);
    string hexToString(const std::string &hex);

public:
    const char *contractAddress;
    const char *walletAddress;
    EthereumProvider();
    EthereumProvider(const char *privateKey, const char *walletAddress);
    ~EthereumProvider();

    void setupWeb3(Web3 *web3);
    void setAdress(const char *walletAddress);
    void setPrivateKey(const char *privateKey);
    void setContractAddress(const char *contractAddress);
    double getBalance(const char *address);
    string sendETHToAddress(uint256_t amount);
    string checkStr(string txt);

    string registerOwner(string name, string description);
    string addLog(uint256_t productId, string objectId, string hash, string location, string timestamp);
    string transferProduct(uint256_t productId, string newOwner);
    string getProductByOwner(string ownerAddress);
    string getProduct(uint256_t productId);
    string getOwnerByAddress(string ownerAddress, string &ownerName, string &ownerDescription);
};

#endif