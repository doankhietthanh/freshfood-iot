#include "ethereum_provider.h"
#include <iomanip>

EthereumProvider::EthereumProvider()
{
}

EthereumProvider::EthereumProvider(const char *privateKey, const char *walletAddress)
{
    this->privateKey = privateKey;
    this->walletAddress = walletAddress;
}

EthereumProvider::~EthereumProvider()
{
}

void EthereumProvider::setupWeb3(Web3 *web3)
{
    this->web3 = web3;
}

void EthereumProvider::setAdress(const char *walletAddress)
{
    this->walletAddress = walletAddress;
}

void EthereumProvider::setPrivateKey(const char *privateKey)
{
    this->privateKey = privateKey;
}

void EthereumProvider::setContractAddress(const char *contractAddress)
{
    this->contractAddress = contractAddress;
}

double EthereumProvider::getBalance(const char *address)
{
    string addressStr = address;
    uint256_t balance = web3->EthGetBalance(&addressStr);
    string balanceStr = Util::ConvertWeiToEthString(&balance, 18); // Eth uses 18 decimal
    double balanceDbl = atof(balanceStr.c_str());
    return balanceDbl;
}

string EthereumProvider::checkStr(string txt)
{
    Contract contract(web3, contractAddress); // contract is on Ropsten

    // Obtain decimals to correctly display ERC20 balance (if you already know this you can skip this step)
    string text = "123";
    string param = contract.SetupContractData("checkStr(string)", &text);
    string result = contract.ViewCall(&param);
    Serial.println(result.c_str());
    string transactionHash = web3->getString(&result);
    Serial.println("TX on Etherscan:");
    Serial.print(ETHERSCAN_TX);
    Serial.println(transactionHash.c_str()); // you can go st
    return transactionHash;
}

string EthereumProvider::sendETHToAddress(uint256_t amount)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;
    string toAddress = "0x6d9A199428e6D587EF64C54cF210A3C4Bb099F76";

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);
    uint256_t weiValue = Util::ConvertToWei(1, 18); // send 0.25 eth
    unsigned long long gasPriceVal = 1000000000ULL;
    uint32_t gasLimitVal = 90000;
    string emptyString = "";

    string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &toAddress, &weiValue, &emptyString);
    Serial.println(result.c_str());
    string transactionHash = web3->getString(&result);
    Serial.println("TX on Etherscan:");
    Serial.print(ETHERSCAN_TX);
    Serial.println(transactionHash.c_str()); // you can go st
    return transactionHash;
}

string EthereumProvider::addLog(uint256_t productId, string objectId, string hash, string location, string timestamp)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 300000;
    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    const char *funcName = "addLog(uint256,string,string,string,uint256)";
    string ret = "";
    {
        // convert the function name to hex
        std::string in = Util::ConvertBytesToHex((const uint8_t *)funcName, strlen(funcName));
        std::vector<uint8_t> contractBytes = Util::ConvertHexToVector(&in);
        std::string out = Crypto::Keccak256(&contractBytes);
        out.resize(10);
        ret = out;

        // convert productId to hex
        std::vector<uint8_t> bits = productId.export_bits();
        ret += Util::PlainVectorToString(&bits);

        ret += "00000000000000000000000000000000000000000000000000000000000000a0";
        ret += "00000000000000000000000000000000000000000000000000000000000000e0";
        ret += "0000000000000000000000000000000000000000000000000000000000000120";

        // convert timestamp to hex
        int timestampInt = stoi(timestamp);
        std::stringstream ssTime;
        ssTime << std::hex << std::setw(64) << std::setfill('0') << timestampInt;
        ret += ssTime.str();

        // convert objectId to hex
        ret += GenerateIntToHex(objectId.length());

        in = Util::ConvertBytesToHex((const uint8_t *)objectId.c_str(), objectId.length()).substr(2);
        size_t digits = in.length();
        ret += in + string(64 - digits, '0');

        // convert hash to hex
        ret += GenerateIntToHex(hash.length());

        in = Util::ConvertBytesToHex((const uint8_t *)hash.c_str(), hash.length()).substr(2);
        digits = in.length();
        ret += in + string(64 - digits, '0');

        // convert location to hex
        ret += GenerateIntToHex(location.length());

        in = Util::ConvertBytesToHex((const uint8_t *)location.c_str(), location.length()).substr(2);
        digits = in.length();
        if (digits > 64)
            ret += in + string(128 - digits, '0');
        else
            ret += in + string(64 - digits, '0');
    }

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &contractAddressStr, &zeroValue, &ret);
        Serial.println(result.c_str());
        string transactionHash = web3->getString(&result);
        Serial.println("TX on Etherscan:");
        Serial.print(ETHERSCAN_TX);
        Serial.println(transactionHash.c_str()); // you can go st
        return transactionHash;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::registerOwner(string name, string description)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 1000000;
    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    // string p =
    //     contract.SetupContractData(
    //         "registerOwner(string,string)", &n, &d);

    // string ret = "0xa4fde7c60000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000000000000000008000000000000000000000000000000000000000000000000000000000000000055468616e6800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000075175794e686f6e00000000000000000000000000000000000000000000000000";

    const char *funcName = "registerOwner(string,string)";
    string ret = "";
    {
        // convert the function name to hex
        std::string in = Util::ConvertBytesToHex((const uint8_t *)funcName, strlen(funcName));
        std::vector<uint8_t> contractBytes = Util::ConvertHexToVector(&in);
        std::string out = Crypto::Keccak256(&contractBytes);
        out.resize(10);
        // end of convert the function name to hex

        ret = out;
        ret += "000000000000000000000000000000000000000000000000000000000000004";
        ret += "00000000000000000000000000000000000000000000000000000000000000080";

        ret += GenerateIntToHex(name.length());

        in = Util::ConvertBytesToHex((const uint8_t *)name.c_str(), name.length()).substr(2);
        size_t digits = in.length();
        ret += in + string(64 - digits, '0');

        ret += GenerateIntToHex(description.length());

        in = Util::ConvertBytesToHex((const uint8_t *)description.c_str(), description.length()).substr(2);
        digits = in.length();
        ret += in + string(64 - digits, '0');
    }

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &contractAddressStr, &zeroValue, &ret);
        Serial.print("SendTransaction: ");
        Serial.println(result.c_str());
        string transactionHash = web3->getString(&result);
        Serial.println("TX on Etherscan:");
        Serial.print(ETHERSCAN_TX);
        Serial.println(transactionHash.c_str()); // you can go st
        return transactionHash;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::transferProduct(uint256_t productId, string newOwner)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 300000;
    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    const char *funcName = "transferProduct(uint256,address)";
    string ret = contract.SetupContractData(funcName, &productId, &newOwner);

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &contractAddressStr, &zeroValue, &ret);
        Serial.println(result.c_str());
        string transactionHash = web3->getString(&result);
        Serial.println("TX on Etherscan:");
        Serial.print(ETHERSCAN_TX);
        Serial.println(transactionHash.c_str()); // you can go st
        return transactionHash;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::getProductByOwner(string ownerAddress)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 300000;
    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    const char *funcName = "getProductByOwner(address)";
    string ret = contract.SetupContractData(funcName, &myAddress);

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        Serial.println("TX on Etherscan:");

        string result = contract.ViewCall(&ret);
        Serial.println(result.c_str());

        string product = web3->getString(&result);
        Serial.print(ETHERSCAN_TX);
        Serial.println(product.c_str());
        return product;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::getProduct(uint256_t productId)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 300000;
    string myAddress = this->walletAddress;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&myAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    const char *funcName = "getProduct(uint256)";
    string ret = contract.SetupContractData(funcName, &productId);

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        Serial.println("TX on Etherscan:");

        string result = contract.ViewCall(&ret);
        Serial.println(result.c_str());

        string product = web3->getString(&result);
        Serial.print(ETHERSCAN_TX);
        Serial.println(product.c_str());
        return product;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::getOwnerByAddress(string ownerAddress, string &ownerName, string &ownerDescription)
{
    Contract contract(web3, this->contractAddress);
    contract.SetPrivateKey(this->privateKey);

    unsigned long long gasPriceVal = 22000000000ULL;
    uint32_t gasLimitVal = 300000;
    string contractAddressStr = this->contractAddress;

    uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&ownerAddress);
    Serial.print("Nonce: ");
    Serial.println(nonceVal);

    const char *funcName = "getOwnerByAddress(address)";
    string ret = contract.SetupContractData(funcName, &ownerAddress);

    Serial.print("input:::");
    Serial.println(ret.c_str());

    uint256_t zeroValue = 0;

    try
    {
        Serial.println("TX on Etherscan:");

        string result = contract.ViewCall(&ret);
        Serial.println(result.c_str());

        string transactionHash = web3->getString(&result);

        transactionHash = transactionHash.substr(2, transactionHash.length());

        string hex1 = transactionHash.substr((transactionHash.length() - DATA_LENGTH * 4), DATA_LENGTH * 2);

        string hex2 = transactionHash.substr((transactionHash.length() - DATA_LENGTH * 2));

        ownerName = RemoveBitZero(hex1);
        ownerDescription = RemoveBitZero(hex2);
        return transactionHash;
    }
    catch (const char *e)
    {
        Serial.println(e);
        return "Transaction is rejected";
    }
}

string EthereumProvider::GenerateIntToHex(int number)
{
    std::stringstream ss;
    ss << std::hex << number;
    std::string numHex(ss.str());

    return string(64 - numHex.length(), '0') + numHex;
}

string EthereumProvider::hexToString(const std::string &hex)
{
    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

string EthereumProvider::RemoveBitZero(const std::string &hex)
{
    string length;
    for (int i = 0; i < 64; i++)
    {
        if (hex[i] == '0')
            continue;
        length.push_back(hex[i]);
    }

    int lengthDup = stoi(length, 0, 16) * 2;

    string data;
    for (int i = 64; i < 64 + lengthDup; i++)
    {
        data.push_back(hex[i]);
    }

    string result = hexToString(data);

    return result;
}