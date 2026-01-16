#ifndef CONFIG_HPP
#define CONFIG_HPP


#include "../allincludes.hpp"

class ConfigNode {
	private:
		std::vector<ConfigNode> ConfigPars;
		std::string name;
		std::map<std::string, std::vector<std::string> > values;
		std::vector<ConfigNode> children;
	public:
		ConfigNode();
		ConfigNode(const std::string& name);
		~ConfigNode();
		
		void addValue(const std::string& key, const std::string& value);
		void addChild(const ConfigNode& child);
		ConfigNode& getLastChild();
		const std::vector<ConfigNode>& getChildren() const;
		const std::string& getName() const;
		void PutName(const std::string& name);
		
		std::map<std::string, std::vector<std::string> >& getValues() ;
		static ConfigNode GetServer(std::vector<ConfigNode> ConfigPars, int RealPort);
		static std::vector<std::string> getValuesForKey(ConfigNode& ConfNode, const std::string& key, std::string del)  ;
		static std::vector<std::string> ConfgetValuesForKey(ConfigNode& ConfNode, const std::string& key)  ;
		static std::string GetLocationValue(ConfigNode& ConfNode, size_t index);
		std::string GetRightLocation(std::string path);
};


void StructConf(std::string ConfigFilePath, std::vector<ConfigNode> &ConfigPars);


#endif