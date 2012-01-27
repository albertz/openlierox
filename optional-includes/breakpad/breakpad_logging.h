// -DBP_LOGGING_INCLUDE=breakpad_logging.h
// see libs/breakpad/src/processor/logging.h

struct NullLogStream {
	template<typename T> std::ostream& operator<<(const T&) {
		return std::cout;
	}	
};

#define BPLOG_INFO NullLogStream()
