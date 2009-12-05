#include "grammar.h"

#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <map>
#include <algorithm>
#include <set>
#include <vector>
#include <utility>
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>
#include "util/macros.h"
#include "util/stringbuild.h"

using std::cout;
using std::endl;
using boost::array;
using boost::lexical_cast;

struct TokenSet
{
	TokenSet(std::set<int>& s_, int idx_)
	: s(&s_), idx(idx_)
	{
	}
	
	std::set<int>* s;
	int idx;
};

struct Handler
{
	Handler(std::istream& str_, std::string const& file_)
	: str(str_), tokenNumber(1), setNumber(1), assumeset(0)
	, line(1), file(file_)
	{
		/*
		Token* t = new Token("EOF");
		t->idx = 0;
		t->def = "\"\\000\"";
		tokens.push_back(t);*/
		next();
	}
	
	int cur()
	{
		return c;
	}
	
	void next()
	{
		c = str.get();
		if(c == std::istream::traits_type::eof())
			c = -1;
		if(c == '\n')
			++line;
	}
	
	void addRule(Rule* r)
	{
		rules_[r->name] = r;
	}
	
	void addToken(Token* t, int state)
	{
		t->idx = tokenNumber++;
		tokens.push_back(t);
		states[state].push_back(t);
	}
	
	void addAlias(std::string const& name, std::string const& content)
	{
		aliases.push_back(std::make_pair(name, content));
	}
	
	Token* findToken(std::string const& name)
	{
		foreach(i, tokens)
		{
			if((*i)->name == name)
				return *i;
		}
		
		return 0;
	}
	
	int indexFirstSet(std::set<int>& s)
	{
		foreach(i, tokensets)
		{
			if(*i->s == s)
			{
				return i->idx;
			}
		}
		
		// We must create a new set
		int idx = tokensets.size() + 1;
		tokensets.push_back(TokenSet(s, idx));
		return idx;
	}
	
	bool firstSet(Element& self, std::set<int>& s)
	{
		if(self.sub)
		{
			firstSet(*self.sub, s);
		}
		else if(!self.child.empty() && self.func == Element::NoFunc)
		{
			let_(i, rules_.find(self.child));
			if(i != rules_.end())
				firstSet(*i->second, s);
			else
			{
				let_(j, findToken(self.child));
				if(j)
					s.insert(j->idx);
				else
				{
					error("Identifier '" + self.child + "' not declared");
					exit(1);
				}
			}
		}
		else
			return false;
		return true;
	}
	
	
	void firstSet(Option& self, std::set<int>& s)
	{
		foreach(i, self.elements)
		{
			if(firstSet(**i, s) && !(*i)->optional)
				return;
		}
	}
	
	void firstSet(Def& self, std::set<int>& s)
	{
		if(self.inProgress)
			throw SyntaxError("Left recursive rule(s)");

		self.inProgress = true;
		
		foreach(i, self.options)
		{
			firstSet(**i, s);
		}
		
		self.inProgress = false;
	}
	
	void firstSet(Rule& self, std::set<int>& s)
	{
		firstSet(*self.def, s);
	}
	
	std::string firstSetName(int idx)
	{
		return S_("set_") << idx;
	}
	
	std::string firstSetTest(int idx)
	{
		switch(tokensets[idx - 1].s->size())
		{
			case 0: return "true";
			case 1: return S_("(cur == ") << *tokensets[idx - 1].s->begin() << ')';
			default: return firstSetName(idx) + "[cur]";
		}
		
		return S_("set_") << idx;
	}
	
	std::string firstSetTest(Element& self, bool determined = false)
	{
		if(!self.firstSetIdx)
		{
			firstSet(self, self.first);
			self.firstSetIdx = indexFirstSet(self.first);
		}
		
		if(determined)
			assumeset = tokensets[self.firstSetIdx - 1].s;
		
		return firstSetTest(self.firstSetIdx);
	}
	
	std::string firstSetTest(Option& self, bool determined = false)
	{
		if(!self.firstSetIdx)
		{
			firstSet(self, self.first);
			self.firstSetIdx = indexFirstSet(self.first);
		}
		
		if(determined)
			assumeset = tokensets[self.firstSetIdx - 1].s;
			
		return firstSetTest(self.firstSetIdx);
	}
	
	//Element Option
	
	void output(std::ostream& s, Element& self)
	{
		if(self.optional)
			s << "if(" << firstSetTest(self, true) << ") {\n";
			
		switch(self.mult)
		{
			case Element::Repeat0: s << "while(" << firstSetTest(self, true) << ") {\n"; break;
			case Element::Repeat1: s << "do {\n"; break;
			case Element::Once: break;
		}
				
		if(self.sub)
			output(s, *self.sub);
		
		else if(!self.child.empty())
		{
			switch(self.func)
			{
				case Element::NoFunc:
				{
					let_(i, rules_.find(self.child));
					if(i != rules_.end())
						s << "rule_" << i->first << "(" << self.code << ");\n";
					else
					{
						let_(j, findToken(self.child));
						if(j)
						{
							Token& token = *j;
							//s << "if(cur != " << token.idx << ") syntaxError();\n";
							if(!assumeset || assumeset->find(token.idx) == assumeset->end())
							{
								s << "if(!matchToken(" << token.idx << ")) return;\n";
							}
							if(!self.code.empty())
							{
								if(!token.code.empty())
								{
									if(self.code[0] == '=')
										s << self.code.substr(1) << ".reset(curData.release());\n";
									else	
										s << "std::auto_ptr<" << token.name << "> " << self.code << "(static_cast<" << token.name << "*>(curData.release()));\n";
								}
								else
								{
									error("Token does not return any data");
									exit(1);
								}
							}
							s << "next();\n";
							assumeset = 0;
						}
						else
						{
							error("Identifier '" + self.child + "' not declared");
							exit(1);
						}
					}
				}
				break;
				
				case Element::SkipTo:
				{
					let_(j, findToken(self.child));
					if(!j)
					{
						error("Token '" + self.child + "' not declared");
						exit(1);
					}
					
					s << "while(cur != " << j->idx << " && cur != 0) next();\n";
					assumeset = 0;
				}
				break;
				
				case Element::SkipAfter:
				{
					let_(j, findToken(self.child));
					if(!j)
					{
						error("Token '" + self.child + "' not declared");
						exit(1);
					}
					
					s << "while(cur != " << j->idx << " && cur != 0) next();\nnext();\n";
					assumeset = 0;
				}
				break;
				
				case Element::Location:
				{
					s << "Location " << self.child << "(self->getLoc());\n";
				}
				break;
			}
		}
		else
		{
			s << self.code << endl;
		}
		
		switch(self.mult)
		{
			case Element::Repeat0: s << "}\n"; break;
			case Element::Repeat1: s << "} while(" << firstSetTest(self) << ");\n"; break;
			case Element::Once: break;
		}
		
		if(self.optional)
			s << "}" << endl;
	}
	
	void output(std::ostream& s, Option& self)
	{
		foreach(i, self.elements)
		{
			output(s, **i);
		}
	}
	
	void output(std::ostream& s, Def& self)
	{
		
		if(self.options.size() > 1)
		{
			bool first = true;
			
			foreach(i, self.options)
			{
				if(first)
					first = false;
				else
					s << "else ";
				s << "if(" << firstSetTest(**i, true) << ") {\n";
				output(s, **i);
				s << "}\n";
			}
			
			s << "else { syntaxError(); return; }\n";
		}
		else
		{
			output(s, **self.options.begin());
		}
	}
	
	void output(std::ostream& s, Rule& self)
	{
		s
		<< "void rule_" << self.name << "(" << self.args << ") {\n";
		output(s, *self.def);
		s << "}\n";
	}
	
	void output(std::ostream& s, Token& self)
	{
		if(!self.code.empty() && self.copy.empty())
		{
			s
			<< "struct " << self.name << " : public Token {\n"
			<< "typedef std::auto_ptr<" << self.name << "> ptr;\n"
			<< "#define CONSTRUCT(b_, e_) " << self.name << "(T& g, char const* b_, char const* e_)\n"
			<< self.code
			<<
			"\n"
			"#undef CONSTRUCT\n"
			"};\n";
		}
	}
	
	void output(std::ostream& s, std::string name)
	{
		s <<
		"#include <memory>\n"
		"#include <cstddef>\n"
		"#include <cstdlib>\n"
		"#include <cstring>\n"
		"#include <iostream>\n"
		"using std::auto_ptr;\n"
		;
		
		s << prolog << '\n';
		
		if(!namespace_.empty())
			s << "namespace " << namespace_ << "{\n";
		
		s <<
		"template<class T>\n"
		"struct " << name << " {\n"
		"#define self (static_cast<T *>(this))\n"
		"~" << name << "() { free(buffer); }\n"
		"struct Token";
		
		if(!tokenbase.empty())
			s << " : public " << tokenbase << ' ';
		
		s <<
		"{ typedef std::auto_ptr<Token> ptr;\n"
		<< tokencontent <<
		"\nvirtual ~Token() {}\n"
		"};\n\n"
		;
		
		foreach(i, tokens)
		{
			Token& token = **i;

			output(s, token);
		}
		
		s <<
		"void next() {\n"
		"#define YYCTYPE char\n"
		"#define YYCURSOR curp\n"
		"#define YYLIMIT limit\n"
		"#define YYMARKER marker\n"
		"#define YYFILL(n) fill(n)\n"
		"retry:\n"
		"begin = curp;\n"
		"goto append; append:\n"
		"/*!re2c\n";
		foreach(i, aliases)
		{
			s << i->first << " = " << i->second << ";\n";
		}
		s <<
		"*/\nswitch(state) {";
		
		foreach(state, states)
		{
			s <<
			"case " << state->first << ":\n"
			"/*!re2c\n"
			;

			foreach(i, state->second)
			{
				Token& token = **i;
				
				s << token.def << "{ ";
					
				/*
				if(token.linecounter)
					s << "++this->line; ";
				*/
				if(!token.code2.empty())
				{
					s <<
					"\n#define linecounter() (++this->line)\n"
					"#define skip() goto retry\n"
					"#define append() goto append\n"
					"#define setstate(x_) (state = (x_))\n"
					<< token.code2 << // Append this token at the beginning of others
					"\n#undef setstate\n"
					"#undef append\n"
					"#undef skip\n"
					"#undef linecounter\n";
				}
				
				if(!token.copy.empty())
				{
					let_(j, findToken(token.copy));
					if(!j)
					{
						error("Token not found");
						exit(1);
					}
						
					s <<
					"\n#define b (begin)\n"
					"#define e (YYCURSOR)\n";
					s << " cur = " << j->idx << "; ";
					if(!token.code.empty())
						s << "curData.reset(new " << j->name << "(*self, " << token.code << ")); ";
					else if(!j->code.empty())
						s << "curData.reset(new " << j->name << "(*self, b, e)); ";
					
					s <<
					//"std::cout << \"" << j->name << " \"; \n" 
					"return; \n"
					"\n#undef b\n"
					"#undef e\n";
				}
				else
				{
					s << " cur = " << token.idx << "; ";
					if(!token.code.empty())
						s << "curData.reset(new " << token.name << "(*self, begin, YYCURSOR)); ";
		
					//s << " std::cout << \"" << token.idx << " \"; ";
					s <<
					//"std::cout << \"" << token.name << " \"; \n" 
					"return; ";
				}
			
				s << "}\n";
			}
			
			s <<
			"\"\\000\" { cur = 0; return; }\n"
			"[\\000-\\377] { semanticError(std::string(\"Unknown character '\") + *begin + \"'\"); goto retry; }\n"
			"*/\n"
			"break;\n";
			
		}
		s <<
		"}\n"
		"#undef YYCTYPE\n"
		"#undef YYCURSOR\n"
		"#undef YYLIMIT\n"
		"#undef YYMARKER\n"
		"#undef YYFILL\n"
		"}\n";
		
		s <<
		"void fill(size_t s) {\n"
		//"std::cout << \"fill begin (\" << (void*)begin << \", \" << (void*)marker << \", \" << (void*)curp << \", \" << (void*)limit << \")\\n\";"
		"size_t l = limit - begin;\n"
		"if(buffer)\n"
		"	memmove(buffer, begin, l);\n" // Move the beginning of the token to the beginning of the buffer
		"size_t newSize = std::max(static_cast<size_t>(1024), l + s);\n"
		"buffer = (char *)realloc(buffer, newSize);\n"
		"size_t toRead = newSize - l;\n"
		//"std::cout << \"to read: \" << toRead;\n"
		"size_t amountRead = self->read(&buffer[l], toRead);\n"
		//"std::cout << \", amount read: \" << amountRead << '\\n';\n"
		"if(amountRead == 0) { memset(&buffer[l], 0, toRead); }\n" // Add null chars at the end
		"else newSize = l+amountRead;\n"
		//"std::cout << \"fill size: \" << newSize << '\\n';\n"
		"ptrdiff_t offs = buffer - begin;\n"
		"curp += offs;\n"
		"marker += offs;\n"
		"begin = buffer;\n"
		"limit = buffer + newSize;\n"
		//"std::cout << \"fill end (\" << (void*)begin << \", \" << (void*)marker << \", \" << (void*)curp << \", \" << (void*)limit << \")\" << std::endl;"
		"}\n";
		
		
		/*
		"struct SyntaxError : public std::exception {\n"
		"SyntaxError(char const* msg_) : msg(msg_) {}\n"
		"virtual char const* what() const throw() { return msg; }\n"
		"char const* msg;};\n"*/
		s <<
		"struct ParsingAborted : public std::exception { };\n"
		
		"void fatalError(std::string const& msg = \"Syntax error\") {\n"
		"self->reportError(msg, self->getLoc());\n"
		"throw ParsingAborted();\n}\n"
		
		"void syntaxError(std::string const& msg = \"Syntax error\") {\n"
		"syntaxError(msg, self->getLoc());\n"
		"}\n"
		
		"void syntaxError(std::string const& msg, Location loc) {\n"
		"self->reportError(msg, loc);\n"
		"syncTokens = true;\n"
		"error = true;\n"
		"}\n"
		
		"void semanticError(std::string const& msg = \"Semantic error\") {\n"
		"semanticError(msg, self->getLoc());\n"
		"}\n"
		
		"void semanticError(std::string const& msg, Location loc) {\n"
		"self->reportError(msg, loc);\n"
		"error = true;\n"
		"}\n"
		
		"void semanticWarning(std::string const& msg = \"Semantic warning\") {\n"
		"semanticWarning(msg, self->getLoc());\n"
		"}\n"
		
		"void semanticWarning(std::string const& msg, Location loc) {\n"
		"self->reportError(\"warning: \" + msg, loc);\n"
		"}\n"
		
		"bool matchToken(int token) {\n"
		"if(syncTokens) {\n"
		"while(cur != token && cur != 0) next();\n"
		"if(cur == 0 && token != 0) fatalError(\"Unexpected end of file\");\n"
		"syncTokens = false;"
		"} else {\n"
		"if(cur != token) {\n"
		"syntaxError(\"Unexpected token\");\n"
		"return false;\n }"
		"}\n"
		"return true; }\n"
		
		"bool optionalMatch(int token) {\n"
		"if(syncTokens) {\n"
		"while(cur != token && cur != 0) next();\n"
		"syncTokens = false;"
		"if(cur != token) return false;\n"
		"} else {\n"
		"if(cur != token) {\n"
		"return false;\n }"
		"}\n"
		"return true; }\n"
		
		"bool full() { return cur == 0 && !error; }\n";

		foreach(i, rules_)
		{
			Rule& rule = *i->second;

			output(s, rule);
		}
		
		/*
		foreach(i, sets)
		{
			s << "bool " << i->first << "[" << tokenNumber << "];\n";
		}*/
		
		foreach(i, tokensets)
		{
			if(i->s->size() > 1)
				s << "bool " << firstSetName(i->idx) << "[" << tokenNumber << "];\n";
		}
		
		s <<
		name << "() : cur(-1), begin(0), marker(0), buffer(0), curp(0), limit(0), line(1), syncTokens(false), error(false), state(0) {\n";
		
		foreach(i, tokensets)
		{
			if(i->s->size() > 1)
			{
				std::string name = firstSetName(i->idx);
				s << "memset(" << name << ", 0, sizeof(bool)*" << tokenNumber << ");\n";
				foreach(j, *i->s)
				{
					s << name << "[" << *j << "] = true;\n";
				}
			}
		}
		
		s << "}\n";
		
		s <<
		"int cur;\n"
		"std::auto_ptr<Token> curData;\n"
		"char* curp;\n"
		"char* limit;\n"
		"char* marker;\n"
		"char* begin;\n"
		"char* buffer;\n"
		"int line;\n"
		"int state;\n"
		"bool syncTokens;\n"
		"bool error;\n"
		"#undef self\n"
		"};\n";
		
		if(!namespace_.empty())
			s << "}\n";
	}
	
	void error(std::string const& msg)
	{
		std::cerr << file << ':' << line << ": " << msg << endl;
	}
	
	int c;
	std::istream& str;
	std::string file;
	std::map<std::string, Rule *> rules_;
	std::list<Token *> tokens;
	std::map<int, std::list<Token *> > states;
	std::list<std::pair<std::string, std::string> > aliases;  
	//std::map<std::string, std::set<int> > sets;
	std::vector<TokenSet> tokensets;
	std::set<int>* assumeset;
	int setNumber;
	int tokenNumber;
	std::string prolog;
	int assumeToken;
	
	std::string tokenbase;
	std::string tokencontent;
	std::string namespace_;
	int line;
	//int maxToken;
};

//#define PARSER

int main(int argc, char const* argv[])
{
	if(argc < 3)
	{
		cout << "Usage: parsergen <source> <target>\n";
		return 0;
	}
	
	std::ifstream i(argv[1]);
	Grammar<Handler> handler((Handler(i, argv[1])));
		
	try
	{
		
		
		handler.rules();
		cout << "Parsed file" << endl;
		std::ofstream o(argv[2]);
		handler.output(o, "TGrammar");
	}
	catch(SyntaxError& err)
	{
		std::cerr << argv[1] << ':' << handler.line << ": " << err.what() << endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << ">:O" << endl;
		return 1;
	}
}
