#include "./pars_config/config.hpp"
#include "./AllServer/HttpServer.hpp"




int ConfigeFileFunc(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars)
{
	try
	{
		if (ConfigFilePath.size() < 6 ||  ConfigFilePath.substr(ConfigFilePath.length() - 5) != ".conf")
	        throw ("Error: Config file does not have the correct extension. {.conf}");
		StructConf(ConfigFilePath, ConfigPars);
	}
	catch (char const *e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	catch (const  std::string e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	return 0; 
}

int StartServerFunc(std::vector<ConfigNode> ConfigPars)
{
	(void)ConfigPars;
	try
	{
		HttpServer server;
		server.setup_server(ConfigPars);
		server.run(ConfigPars);
	}
	catch (char const *e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	catch (const  std::string e)
	{
		std::cerr << e << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc <= 2)
	{
		signal(SIGPIPE, SIG_IGN);
		std::string ConfigFilePath;
		if (argc == 2)
			ConfigFilePath = argv[1];
		else
			ConfigFilePath = "./default/default.conf";
		std::vector<ConfigNode> ConfigPars;
		ConfigNode obj;
		if (ConfigeFileFunc(ConfigFilePath, ConfigPars) == 1)
			return 1;
		if (StartServerFunc(ConfigPars) == 1)
			return 1;
	}
	else
	{
		std::cerr << "./webserv [configuration file]" << std::endl;
		return (1);
	}
	return 0;
}

