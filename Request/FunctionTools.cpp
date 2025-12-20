#include "Request.hpp"

std::string	RandomString()
{
	std::string			Ret;
	const std::string	Chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	while(Ret.size() != 15) // generante random 15 chars string
		Ret += Chars[rand() % (Chars.size() - 1)];
	return Ret;
}

void	Appender(std::string &Buffer, const std::string &PrevBuffer, const std::string &tmp)
{
	Buffer.clear();
	Buffer.reserve(PrevBuffer.size() + tmp.size());
	Buffer.append(PrevBuffer.data(), PrevBuffer.size());
	Buffer.append(tmp.data(), tmp.size());
}

void	PrintCrlfString(std::string Buffer)
{
	std::cout << "|";

	for (size_t i = 0; i < Buffer.size(); i++)
	{
		if (Buffer[i] == '\r')
			std::cout << "R";
		else if (Buffer[i] == '\n')
			std::cout << "N";
		else
			std::cout << Buffer[i];
	}

	std::cout << "|\n";
}

// --------------#	URI TOOLS #-------------- //


bool	IsHexa(char c)
{
	return std::isdigit(c) || (std::tolower(c) >= 'a' && std::tolower(c) <= 'f');
}

bool	IsValide(char c)
{
	return std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' ||
		   c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' ||
		   c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
}

std::string	HexaToChar(std::string	Hexa)
{
	std::string tmp = "0x" + Hexa; // Convert to Hexa form

	char	Helpervar = static_cast<char>(std::stod(tmp)); // Convert Hexa to Char
	return std::string(1, Helpervar); // calling constructor string with 1 character
}

std::string	PurePath(std::string &Path)
{
    size_t posLength = std::string::npos;
    size_t cgiPos = Path.find(".cgi");
    size_t phpPos = Path.find(".php");
    size_t pyPos = Path.find(".py");
    if (cgiPos != std::string::npos && cgiPos < phpPos && cgiPos < pyPos)
        posLength = cgiPos + 4;
    else if (phpPos != std::string::npos && phpPos < cgiPos && phpPos < pyPos)
        posLength = phpPos + 4;
    else if (pyPos != std::string::npos && pyPos < cgiPos && pyPos < phpPos)
        posLength = pyPos + 3;

    return Path.substr(0, posLength);
}


// --------------#	PRINTER	 #-------------- //

void	PrintHeaders(std::map<std::string, std::string> Headers)
{
	for (std::map<std::string, std::string>::const_iterator it = Headers.begin(); it != Headers.end(); ++it)
	std::cout << "key{" << it->first << "}		value:" << it->second << std::endl;
	std::cout  << "-----------------------------------------------------" << std::endl;
}

// --------------#	PARSERS	 #-------------- //

bool	ValidContentLength(const std::string& value)
{
	if (value.empty())					return false;
	for (size_t i = 0; i < value.size(); i++)
	    if (!std::isdigit(value[i]))	return false;
	double ret = strtod(value.c_str(), NULL);
	if (ret < 0 || ret > SIZE_T_MAX)	return false;

	return true;
}

bool	ValidFieldName(const std::string& name)
{
    if (name.empty())
		return false;
    for (size_t i = 0; i < name.size(); ++i)
        if (!IsValide(name[i]))
			return false;
    return true;
}

bool	ValidFieldValue(const std::string& value)
{
    for (size_t i = 0; i < value.size(); i++)
        if (!std::isprint(value[i]))
			return false;
    return true;
}

bool	ValidBoundary(std::string	&value)
{
	const std::string	&BChars = "\'()+_,-./:=? ";

	if (value.size() > 2 && (value.front() == '"' && value.back() == '"'))
	{
		value.erase(0, 1);
		value.pop_back();
	}
	for (size_t	i = 0; i < value.size(); i++)
	{
		if (!std::isalnum(value[i]))
		{
			if (BChars.find(value[i]) == std::string::npos)
					return false;
		}
	}
	return true;
}

void	TrimSpaces(std::string& str)
{
    size_t start = 0;

    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    size_t end = str.size();

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;

	str = str.substr(start, end - start);
}

// --------------#	COUNTERS	 #-------------- //

size_t	CrlfCounter(std::string	&str, size_t pos)
{
	size_t Count = 0;

	for (size_t i = 0; i + 1 < pos; i++)
	{
		if (str[i] == '\r' && str[i + 1] == '\n')
			Count++;
	}
	return Count;
}