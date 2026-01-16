#include "config.hpp"
#include <string>
#include <vector>

ConfigNode::ConfigNode(){}

std::vector<std::string> split(const std::string& text) {
    std::istringstream stream(text);
    std::string word;
    std::vector<std::string> words;
    while (stream >> word) words.push_back(word);
    return words;
}

ConfigNode::~ConfigNode(){}

ConfigNode::ConfigNode(const std::string& name) : name(name) {}

void ConfigNode::addValue(const std::string& key, const std::string& value) {values[key].push_back(value);}

void ConfigNode::addChild(const ConfigNode& child) {children.push_back(child);}

ConfigNode& ConfigNode::getLastChild() {return children.back();}

const std::vector<ConfigNode>& ConfigNode::getChildren() const {return children;}

const std::string& ConfigNode::getName() const {return name;}

std::map<std::string, std::vector<std::string> >& ConfigNode::getValues()  {return values;}

bool startsWith(const std::string& path, const std::string& prefix)
{
    if (prefix.length() > path.length()) return false;
    return path.compare(0, prefix.length(), prefix) == 0;
}

std::string ConfigNode::GetLocationValue(ConfigNode& ConfNode, size_t index) 
{
    if (index > ConfNode.children.size())
        return "";
    else
        return ConfNode.children[index].getName();
}

std::string RemoveSlashs(std::string path)
{
    std::string result;
    bool lastWasSlash = false;
    
    result.reserve(path.length());
    
    for (std::string::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        if (*it == '/')
        {
            if (!lastWasSlash)
            {
                result += *it;
                lastWasSlash = true;
            }
        }
        else
        {
            result += *it;
            lastWasSlash = false;
        }
    }
    return result;
}

std::vector<std::string> ConfigNode::   getValuesForKey(ConfigNode& ConfNode, const std::string& key, std::string del) 
{
    std::vector<std::string> emptyResult;
    std::vector<std::string> arr ;

    if (del.empty())
    {
        std::map<std::string, std::vector<std::string> > keys = ConfNode.getValues();
        for (std::map<std::string, std::vector<std::string> >::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            if (it->first == key)
                return it->second;
        }
    }
    else {
        for (size_t i = 0; i < ConfNode.children.size(); i++)
        {
            arr = split(ConfNode.children[i].getName());
            if (arr[1] == del)
            {
                std::vector<std::string> a = getValuesForKey(ConfNode.children[i], key, "");
                if (!a.empty())
                    return a;
                else
                    return getValuesForKey(ConfNode, key, "");
            }
        }
    }
    return emptyResult;
}

std::vector<std::string> ConfigNode::ConfgetValuesForKey(ConfigNode& ConfNode, const std::string& key) 
{
    std::vector<std::string> emptyResult;
    std::vector<std::string> arr ;

    std::map<std::string, std::vector<std::string> > keys = ConfNode.getValues();
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        if (it->first == key)
            return it->second;
    }
    return emptyResult;
}

void ConfigNode::PutName(const std::string &name) {this->name = name;}

void trimSpaces(std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");

    if (first == std::string::npos)
    {
        str.clear();
        return;
    }

    str = str.substr(first, last - first + 1);
}

std::string removeSpaces( std::string &input)
{
	std::string result = "";
	for (size_t i = 0; i < input.length(); i++)
		if (input[i] != ' ') result += input[i];
	return result;
}

std::string removeExtraSpaces(std::string line)
{
	std::stringstream result;
	std::string word;

	std::stringstream ss(line);
	while (ss >> word)
	{
		if (!result.str().empty()) result << " ";
		result << word;
	}

	return result.str();
}

std::string RmComments(std::string buffer) {
    std::string result;
    std::istringstream iss(buffer);
    std::string line;
    while (std::getline(iss, line)) {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) line.erase(commentPos);
        line = removeExtraSpaces(line);
        if (!line.empty()) result += line + " ";
    }
    size_t endPos = result.find_last_not_of(' ');
    if (endPos != std::string::npos) result.erase(endPos + 1);
    return result;
}

void CheckStartServer(std::vector<ConfigNode*> &nodeStack, std::string text, size_t pos)
{
    std::vector<std::string> words = split(text);
    if (pos == 0)
    {
        if (words.size() != 1) throw ("Error: Expected exactly one word in server.");
        if (words[0] != "server") throw ("Error: The first word must be 'server'.");
    }
    else
    {
        std::string name = nodeStack.back()->getName();
        std::vector<std::string> checkwords = split(name);
        if (checkwords[0] == "location") throw ("Error: Location block cannot be nested in another location block.");
        if (words.size() != 2) throw ("Error: Expected exactly two words in location");
        if (words[0] != "location") throw ("Error: The first word must be 'location'.");
    }
}

void CheckAllError(std::vector<std::string>& KV, const std::string& key, ConfigNode& ConfNode, int max, int mult) {
    int count = 0;
    (void)ConfNode;
    count = KV.size() - 1;
    if (key != KV[0]) return;
    
    std::vector<std::string> helo = ConfNode.ConfgetValuesForKey(ConfNode, key);
    if (!helo.empty())
    {
        if (key == "return")
            throw ("return should have 1 or 2 in the same return");
        if (max != -1 && (int)(helo.size() + count) > max) throw ("Error: Too many values for key '" + key + "'. Maximum allowed is " + intToString(max) + ".");
        if (mult != -1 && (helo.size() + count) % mult != 0) throw ("Error: Number of values for key '" + key + "' must be a multiple of " + intToString(mult) + ".");
    }
    else
    {
        if (max != -1 && (int)(count) > max) throw ("Error: Too many values for key '" + key + "'. Maximum allowed is " + intToString(max) + ".");
        if (mult != -1 && (count) % mult != 0) throw ("Error: Number of values for key '" + key + "' must be a multiple of " + intToString(mult) + ".");
    }
}

void ErrorHandle(std::vector<std::string>& KV, ConfigNode &ConfNode, std::string blockType)
{
    if (blockType == "server")
    {
        CheckAllError(KV, "listen", ConfNode, -1, 1);
        CheckAllError(KV, "error_page", ConfNode, -1, 2);
        CheckAllError(KV, "client_max_body_size", ConfNode, 1, 1);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, 1, -1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, 2, 1);
    }
    else {
        CheckAllError(KV, "allow_methods", ConfNode, 3, 1);
        CheckAllError(KV, "autoindex", ConfNode, 1, 1);
        CheckAllError(KV, "return", ConfNode, 2, 1);
        CheckAllError(KV, "root", ConfNode, 1, 1);
        CheckAllError(KV, "index", ConfNode, 1, -1);
        CheckAllError(KV, "upload_store", ConfNode, 1, 1);
        CheckAllError(KV, "allow_cgi", ConfNode, 3, 1);
    }
}

void AllowedIn(std::vector<std::string> VALID_KEYS, std::vector<std::string>& words, ConfigNode &ConfNode, const std::string& blockType)
{
    std::vector<std::string>::const_iterator it;
    for (it = VALID_KEYS.begin(); it != VALID_KEYS.end(); ++it)
        if (*it == words[0]) 
            break;
    if (it == VALID_KEYS.end())
        throw ("Error: Invalid key '" + words[0] + "' for " + blockType + " block");
    ErrorHandle(words, ConfNode, blockType);
}


int CheckDigit(std::string value)
{
    for (size_t i = 0; value[i] - 1; ++i) {
        if (!std::isdigit(value[i]))
            return 1;
    }

    return 0;
}

void CheckUnit(const std::string& value)
{
    if (value.empty())
        throw ("client_max_body_size is empty");

    size_t len = value.length();
    char unit = value[len - 1];

    if (unit != 'B' && unit != 'K' && unit != 'M' && unit != 'G')
        throw ("Unit of the client_max_body_size is not correct (B, K, M, G)");

    for (size_t i = 0; i < len - 1; ++i) {
        if (!std::isdigit(value[i]))
            throw ("client_max_body_size must be numeric before unit");
    }
}

void MaxBodySizeToBytes(std::vector<std::string>& words)
{
    std::string value = words[1];
    CheckUnit(value);

    char unit = value[value.length() - 1];
    std::string num_str = value.substr(0, value.length() - 1);

    // Remove leading zeros
    size_t start = 0;
    while (start < num_str.length() && num_str[start] == '0')
        ++start;
    num_str = num_str.substr(start);
    if (num_str.empty())    
	{
        words[1] = "0"; 
		return ;
	}

    if (num_str.length() >= 20)
        throw ("client_max_body_size is too large");

    std::stringstream ss(num_str);
    unsigned long long temp = 0;
    ss >> temp;

    if (ss.fail())
        throw ("Invalid number for client_max_body_size");

    unsigned long long multiplier = 1;
    switch (unit)
    {
        case 'B': multiplier = 1; break;
        case 'K': multiplier = 1024ULL; break;
        case 'M': multiplier = 1024ULL * 1024; break;
        case 'G': multiplier = 1024ULL * 1024 * 1024; break;
        default:
            throw ("Unexpected unit");
    }

    if (temp > std::numeric_limits<size_t>::max() / multiplier)
        throw ("Overflow: client_max_body_size too large");

    size_t number = static_cast<size_t>(temp * multiplier);

    std::stringstream result;
    result << number;
    words[1] = result.str();
}

void CheckListen(std::vector<std::string>& words, ConfigNode &ConfNode)
{
    int is = 0;
    for (std::vector<std::string>::iterator it = words.begin(); it != words.end(); ++it)
    {
        if (*it == "listen" && is == 0)
        {
            is = 1;
            continue;
        }

        std::string port = it->c_str();
        for (size_t i = 0; port[i]; ++i) {
            if (!std::isdigit(port[i]))
                throw ("listen only take digits.");
        }
        if (port.length() > 5)
            throw ("listen port should be > 0 and < 65535.");
        int a = std::atoi(port.c_str());
        if (a > 65535)
            throw ("listen port should be > 0 and < 65535.");
        std::vector<std::string> confPorts = ConfNode.getValuesForKey(ConfNode, "listen", "");
        if (!confPorts.empty())
        {
            for (std::vector<std::string>::iterator Confoneport = confPorts.begin(); Confoneport != confPorts.end(); ++Confoneport) {
                std::string ConPo = Confoneport->c_str();
                if (ConPo == port)
                    throw ("duplicate port in the same server");
            }
        }
    }
}

void CheckEdgeCases(std::vector<std::string>& words, ConfigNode &ConfNode)
{
    if (words[0] == "client_max_body_size")
        MaxBodySizeToBytes(words);
    else if (words[0] == "allow_methods")
    {
        int CheckNono = 0;
        int CheckOthers = 0;
        for (std::vector<std::string>::iterator it = words.begin()+ 1; it != words.end(); ++it) {
            if (*it == "GET" || *it == "POST" || *it == "DELETE")
                CheckOthers = 1;
            else if (*it == "NONE")
                CheckNono = 1;
            else 
                throw ("Error: allow_methods are GET POST DELETE NONE");
        }
        std::vector<std::string > fromconf = ConfNode.ConfgetValuesForKey(ConfNode, "allow_methods");
        if (!fromconf.empty())
            for (std::vector<std::string>::iterator it = fromconf.begin(); it != fromconf.end(); ++it) {
                if (*it == "GET" || *it == "POST" || *it == "DELETE")
                    CheckOthers = 1;
                else if (*it == "NONE")
                    CheckNono = 1;
            }
        if (CheckNono == 1 && CheckOthers == 1)
            throw ("Error: Cant be NONE and other Methods in the same location in allow_methods");
    }
    else if (words[0] == "autoindex")
    {
        if (words[1] != "on" && words[1] != "off")
            throw ("Error: antoindex take only on or off");
    }
    else if (words[0] == "listen")
        CheckListen(words, ConfNode);
}

void AddKV(ConfigNode &ConfNode, std::vector<std::string>& words)
{
    static bool initialized = false;

    static std::vector<std::string> SERVER_VALID_KEYS;
    static std::vector<std::string> LOCATION_VALID_KEYS;

    if (!initialized)
    {
        SERVER_VALID_KEYS.push_back("listen");
        SERVER_VALID_KEYS.push_back("error_page");
        SERVER_VALID_KEYS.push_back("client_max_body_size");
        SERVER_VALID_KEYS.push_back("root");
        SERVER_VALID_KEYS.push_back("index");
        SERVER_VALID_KEYS.push_back("autoindex");
        SERVER_VALID_KEYS.push_back("return");

        LOCATION_VALID_KEYS.push_back("autoindex");
        LOCATION_VALID_KEYS.push_back("allow_methods");
        LOCATION_VALID_KEYS.push_back("return");
        LOCATION_VALID_KEYS.push_back("root");
        LOCATION_VALID_KEYS.push_back("index");
        LOCATION_VALID_KEYS.push_back("allow_cgi");
        LOCATION_VALID_KEYS.push_back("upload_store");

        initialized = true;
    }

    if (words.size() < 2)
        throw ("Error: Invalid key-value pair.");

    std::string nodeName = ConfNode.getName();
    trimSpaces(nodeName);
    std::vector<std::string> locations = split(nodeName);

    if (locations[0] == "server")
        AllowedIn(SERVER_VALID_KEYS, words, ConfNode, locations[0]);
    else if (locations[0] == "location")
        AllowedIn(LOCATION_VALID_KEYS, words, ConfNode, locations[0]);
    else
        throw ("Error: Unknown block type in configuration.");

    CheckEdgeCases(words, ConfNode);
    
    for (size_t i = 1; i < words.size(); ++i)
        ConfNode.addValue(words[0], words[i]);
}

int onlyspace(std::string &text)
{
    for (int i = 0; text[i]; i++) {
        if(!std::isspace(text[i]))
            return 1;
    }
    return 0;
}
void processClosingBrace(std::string &text, std::vector<ConfigNode*> &nodeStack)
{
    if (onlyspace(text) == 1)
        throw ("Error: unexpected data before '}'");
    if (nodeStack.empty())
        throw ("Error: Unmatched closing brace '}'.");
    ConfigNode &lastOne = *nodeStack.back();
    if(lastOne.getName() == "server")
    {
        if (lastOne.getValuesForKey(lastOne, "listen", "").empty() || lastOne.getValuesForKey(lastOne, "root", "").empty())
            throw ("Error: server should have listen and root");
    }
    if(startsWith(lastOne.getName(), "location"))
    {
        if (lastOne.getValuesForKey(lastOne, "allow_methods", "").empty())
            throw ("Error: location should have allow_methods");
    }
    std::vector<std::string> a = lastOne.getValuesForKey(lastOne, "allow_methods", "");
    for (std::vector<std::string>::iterator it = a.begin(); it != a.end(); ++it)
    {
        if (*it == "POST")
            if(lastOne.getValuesForKey(lastOne, "upload_store", "").empty())
               throw ("Error: you need upload_store when u have POST in allow_methods");
    }
    if (!nodeStack.empty())
        nodeStack.pop_back();
    else
        throw ("Error: Unmatched closing brace '}'.");
}

void processSemicolon(std::string &text, std::vector<ConfigNode*> &nodeStack)
{
    if (nodeStack.empty())
        throw ("Error: No active node for key-value pair.");
    std::vector<std::string> words = split(text);
    if (!words.empty())
        AddKV(*nodeStack.back(), words);
}

void processOpeningBrace(std::string &text, std::vector<ConfigNode*> &nodeStack, bool &isRootNameSet, size_t pos)
{
    CheckStartServer(nodeStack, text, pos);
    trimSpaces(text);
    if (nodeStack.size() == 1 && !isRootNameSet)
    {
        nodeStack.back()->PutName(text);
        isRootNameSet = true;
    }
    else
    {
        std::vector<std::string> tokens = split(text);
        if (tokens.size() > 1 && tokens[0] == "location")
        {
            tokens[1] = RemoveSlashs(tokens[1]);
            text = tokens[0] + " " + tokens[1];
        }

        nodeStack.back()->addChild(ConfigNode(text));
        ConfigNode* childPtr = &(nodeStack.back()->getLastChild());

        if (tokens.size() > 1 && tokens[0] == "location")
        {
            std::string newLoc = tokens[1];
            const std::vector<ConfigNode>& siblings = nodeStack.back()->getChildren();
            for (size_t i = 0; i + 1 < siblings.size(); ++i)
            {
                std::vector<std::string> siblingTokens = split(siblings[i].getName());
                if (siblingTokens.size() > 1 && siblingTokens[0] == "location")
                {
                    std::string existingLoc = siblingTokens[1];
                    if (existingLoc == newLoc)
                        throw ("Error: Duplicate location '" + newLoc + "' in the same server block.");
                }
            }
        }

        nodeStack.push_back(childPtr);
    }
}

void checkContent(std::string buffer, std::vector<ConfigNode> &ConfigPars)
{
    std::string delimiters = "{};";
    size_t pos = 0;
    std::string text;
    ConfigNode ConfNode;
    std::vector<ConfigNode*> nodeStack;

    bool isRootNameSet = false;
    if (buffer.size() == 0) throw ("Error: Empty configuration file.");
    while (pos < buffer.size())
    {
        size_t delimiterPos = buffer.find_first_of(delimiters, pos);
        if (delimiterPos == std::string::npos) throw ("Error: Could not find any delimiters '{};'");
        text = buffer.substr(pos, delimiterPos - pos);
        std::string delimiter = buffer.substr(delimiterPos, 1);

        if (delimiter == "{")
        {
            if (nodeStack.empty())
            {
                ConfigPars.push_back(ConfNode);
                ConfNode = ConfigNode();
                nodeStack.push_back(&ConfNode);
                buffer = buffer.substr(pos);
                pos = 0;
                delimiterPos = 0;
                isRootNameSet = false;
                continue;
            }
            processOpeningBrace(text, nodeStack, isRootNameSet, pos);
        }
        else if (delimiter == "}") processClosingBrace(text,nodeStack);
        else if (delimiter == ";") processSemicolon(text, nodeStack);
        pos = delimiterPos + 1;
    }
    ConfigPars.push_back(ConfNode);
    if (nodeStack.size() != 0) throw ("Error: Unmatched opening brace '{'.");
}

void CheckServerPorts( std::vector<ConfigNode>& ConfigPars)
{
    if (ConfigPars.empty())
        throw ("Error: No servers defined.");

    std::set<std::string> usedPorts;
    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string> ports = ConfigNode::ConfgetValuesForKey(ConfigPars[i], "listen");

        if (ports.empty())
            throw ("Error: server block missing 'listen' directive.");

        bool hasUnique = false;

        for (size_t j = 0; j < ports.size(); j++)
        {
            std::string &port = ports[j];
            if (usedPorts.find(port) == usedPorts.end())
            {
                hasUnique = true;
            }
        }
        if (!hasUnique)
        {
            throw ("Error: server " + intToString(i+1) +
                   " has no unique listen port (all taken by previous servers).");
        }
        for (size_t j = 0; j < ports.size(); j++)
            usedPorts.insert(ports[j]);
    }
}

void StructConf(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars)
{
	std::ifstream infile(ConfigFilePath);
	if (!infile.is_open())
		throw ("Error: Could not open configuration file.");
	std::stringstream buffer;
	buffer << infile.rdbuf();
	infile.close();
	checkContent(RmComments(buffer.str()), ConfigPars);
    if (!ConfigPars.empty()) ConfigPars.erase(ConfigPars.begin());
    CheckServerPorts(ConfigPars);
}

ConfigNode GetTheServer(std::vector<ConfigNode> ConfigPars, std::string PortOrHostInConfig, std::string port)
{
    for (size_t i = 0; i < ConfigPars.size(); i++)
    {
        std::vector<std::string> ValuesOfPortOrHost = ConfigNode::ConfgetValuesForKey(ConfigPars[i], PortOrHostInConfig);
        if (!ValuesOfPortOrHost.empty())
        {
            for (std::vector<std::string>::iterator it = ValuesOfPortOrHost.begin(); it != ValuesOfPortOrHost.end(); ++it)
            {
                if (*it == port)
                    return ConfigPars[i];
            }
        }
    }
    return ConfigPars[0];		
}

ConfigNode ConfigNode::GetServer(std::vector<ConfigNode> ConfigPars, int RealPort)
{
    return  GetTheServer(ConfigPars, "listen", intToString(RealPort));
}


std::vector<std::string> split_path(const std::string& path)
{
    std::vector<std::string> components;
    std::string current;
    
    if (path.empty() || path == "/") {
        components.push_back("/");
        return components;
    }

    std::vector<std::string> parts;
    std::string temp;
    for (std::string::size_type i = 0; i < path.length(); ++i) {
        if (path[i] == '/') {
            if (!temp.empty()) {
                parts.push_back(temp);
                temp.clear();
            }
        } else {
            temp += path[i];
        }
    }
    if (!temp.empty()) {
        parts.push_back(temp);
    }

    current = "";
    for (std::vector<std::string>::size_type i = 0; i < parts.size(); ++i) {
        current += "/" + parts[i];
        components.push_back(current + "/");
    }

    return components;
}



std::string ConfigNode::GetRightLocation(std::string path)
{
    std::vector<ConfigNode> locations = getChildren();

    std::string RightLocation = "";
    for (std::vector<ConfigNode>::iterator Alllocations = locations.begin(); Alllocations != locations.end(); ++Alllocations)
    {
        std::vector<std::string> tokens = split(Alllocations->name.c_str());
        std::string loc = RemoveSlashs(tokens[1]);

        std::vector<std::string> SplitPath = split_path(RemoveSlashs(path));

        for (std::vector<std::string>::iterator IterSplitPath = SplitPath.begin(); IterSplitPath != SplitPath.end(); ++IterSplitPath)
        {
            std::string chunkPath = (*IterSplitPath).c_str();
            if (SplitPath.size() == 1 && loc == chunkPath)
                return loc;
            if (startsWith(chunkPath, loc))
            {
                if (loc.length() > RightLocation.length())
                    RightLocation = loc;
            }
        }
    }
    return RightLocation;
}