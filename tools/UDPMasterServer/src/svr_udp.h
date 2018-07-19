
struct HostInfo
{
	HostInfo( std::string _addr, time_t _lastping, std::string _name, int _maxworms, int _numplayers, int _state,
				std::string _version = "OpenLieroX/0.57_beta5", bool _allowsJoinDuringGame = false, std::string _v4address = "" ):
		addr(_addr), lastping(_lastping), name(_name), maxworms(_maxworms), numplayers(_numplayers), state(_state),
		version(_version), allowsJoinDuringGame(_allowsJoinDuringGame), v4address(_v4address) {};

	HostInfo(): lastping(0), maxworms(0), numplayers(0), state(0),
				version("OpenLieroX/0.57_beta5"), allowsJoinDuringGame(false) {};

	std::string addr;
	time_t lastping;
	std::string name;
	unsigned maxworms;
	unsigned numplayers;
	unsigned state;
	std::string version;
	bool allowsJoinDuringGame;
	std::string v4address;
};
