#ifndef GRAMMAR_H
#define GRAMMAR_H

/*
	block:        [A-Za-z0-9_]
	restblock:    ';', epsilon
	command:      [A-Za-z0-9_]
	argumentList: '{', '"', [A-Za-z0-9_], epsilon
	argument:     '{', '"', [A-Za-z0-9_]
	char:         [^"], epsilon
	escape:       '\', '{', '}', '"'
	ident:        [A-Za-z0-9_]
*/

#include <cctype>
#include <list>
#include <string>
#include <set>
#include <exception>
#include <boost/lexical_cast.hpp>

#include <iostream> // TEMP

struct SyntaxError : public std::exception
{
	SyntaxError(char const* desc_)
	: desc(desc_)
	{
	}
	
	virtual char const* what() const throw()
	{
		return desc;
	}
	
private:
	char const* desc;
};

struct Def;

struct Element
{
	Element()
	: sub(0), optional(false), mult(Once)
	, func(NoFunc), firstSetIdx(0)
	{
	}
	
	enum RepeatType
	{
		Once,
		Repeat0,
		Repeat1,
	};
	
	enum Func
	{
		NoFunc,
		SkipTo,
		SkipAfter,
		Location,
	};
	
	RepeatType mult;
	bool optional;
	Def* sub;
	std::string code;
	std::string child;
	Func func;
	std::set<int> first;
	int firstSetIdx;
};

struct Option
{
	Option()
	: firstSetIdx(0)
	{
	}
	
	void addElement(Element* el)
	{
		elements.push_back(el);
	}
	
	std::list<Element *> elements;
	std::set<int> first;
	int firstSetIdx;
};

struct Def
{
	Def()
	: inProgress(false)
	{
	}
	
	void addOption(Option* option)
	{
		options.push_back(option);
	}

	std::string firstSet;
	std::list<Option *> options;
	bool inProgress;
};

struct Rule
{
	Rule(std::string name_)
	: def(0), name(name_)
	{
	}
	
	std::string args;
	std::string name;
	Def* def;
};

struct Token
{
	Token(std::string name_)
	: name(name_) //, linecounter(false), skip(false)
	{
	}
	
	std::string name;
	int idx;
	std::string def;
	std::string code;
	std::string copy; // name of token to copy
	std::string code2;
	//bool linecounter;
	//bool skip;
};

template<class BaseT>
struct Grammar : public BaseT
{
	Grammar(BaseT const& base)
	: BaseT(base)
	{
	}
	
	int cur()
	{
		return BaseT::cur();
	}
	
	int next()
	{
		char old = cur();
		do
		{
			BaseT::next();
		} while(firstSkip());
		
		return old;
	}
	
	int nextInLexeme()
	{
		char old = cur();
		BaseT::next();
	
		return old;
	}
	
	void doneLexeme()
	{
		while(firstSkip())
		{
			BaseT::next();
		}
	}
	
	void syntaxError(char const* str = "Syntax error")
	{
		throw SyntaxError(str);
	}
	
	bool firstSkip()
	{
		return cur() == ' ' || cur() == '\n' || cur() == '\r' || cur() == '\t';
	}
	
	bool firstIdent()
	{
		return isalnum(cur()) || cur() == '@';
	}
	
	std::string ident()
	{
		if(firstIdent())
		{
			std::string i;
			for(; firstIdent(); nextInLexeme())
			{
				if(i.size() > 100)
					syntaxError("Too long identifier");
				i += cur();
			}
						
			return i;
		}
		else
			syntaxError("Expected identifier");
		
		return "";
	}

	template<char B, char E>
	void subcode(std::string& s)
	{
		while(true)
		{
			char c = (char)cur();
			s += c;
			nextInLexeme();
			if(c == E)
			{
				return;
			}
			else
			{
				if(c == B)
					subcode<B, E>(s);
				else if(c == -1)
					syntaxError("Missing code block terminator");
			}
		}
	}
	
	template<char B, char E>
	void code(std::string& s)
	{
		if(cur() != B) syntaxError("Expected code block");
		nextInLexeme();
		
		while(cur() != E)
		{
			char c = (char)cur();
			nextInLexeme();
			s += c;
			if(c == B)
				subcode<B, E>(s);
			else if(c == -1)
				syntaxError("Missing code block terminator");
		}
		
		nextInLexeme();
	}
	
	Rule* rulei()
	{
		std::auto_ptr<Rule> rule(new Rule(ident()));
		doneLexeme();
		
		if(cur() == '<')
		{
			code<'<', '>'>(rule->args);
			doneLexeme();
		}
		
		return rule.release();
	}
	
	bool firstElement()
	{
		return firstIdent()
			|| cur() == '('
			|| cur() == '['
			|| cur() == '$';
	}
	
	Element* element()
	{
		std::auto_ptr<Element> el(new Element);

		switch(cur())
		{
			case '(':
				next();
				el->sub = def();
				if(cur() != ')') syntaxError("Expected ')'");
				next();
			break;
			
			case '[':
				next();
				el->sub = def();
				el->optional = true;
				if(cur() != ']') syntaxError("Expected ']'");
				next();
			break;
			
			case '$':
				next();
				code<'(', ')'>(el->code);
				doneLexeme();
			break;
			
			default:
				std::string t = ident();
				doneLexeme();
				if(t == "@skipto")
				{
					el->func = Element::SkipTo;
					el->child = ident();
					doneLexeme();
				}
				else if(t == "@skipafter")
				{
					el->func = Element::SkipAfter;
					el->child = ident();
					doneLexeme();
				}
				else if(t == "@location")
				{
					el->func = Element::Location;
					el->child = ident();
					doneLexeme();
				}
				else
				{
					el->child = t;
					if(cur() == '<')
					{
						code<'<', '>'>(el->code);
					}
					doneLexeme();
				}
			break;
		}
		
		return el.release();
	}
	
	Option* option()
	{
		std::auto_ptr<Option> op(new Option);
		
		do
		{
			std::auto_ptr<Element> el(element());
			switch(cur())
			{
				case '+': el->mult = Element::Repeat1; next(); break;
				case '*': el->mult = Element::Repeat0; next(); break;
				default :  el->mult = Element::Once; break;
			}
			op->addElement(el.release());
		} while(firstElement());
		
		return op.release();
	}
	
	Def* def()
	{
		std::auto_ptr<Def> d(new Def);
		
		if(cur() == '|') // Optional first bar
			next();
		
		d->addOption(option());
		while(cur() == '|')
		{
			next();
			d->addOption(option());
		}
		
		return d.release();
	}
		
	void rules()
	{
		if(cur() == '{')
		{
			code<'{', '}'>(this->prolog);
			doneLexeme();
		}
		
		while(firstIdent() || cur() == '%')
		{
			if(cur() == '%')
			{
				next();
				std::string opt = ident();
				doneLexeme();
				
				if(opt == "tokenbase")
				{
					code<'{', '}'>(this->tokenbase);
					doneLexeme();
				}
				else if(opt == "alias")
				{
					
					std::string name = ident();
					doneLexeme();
					std::string content;
					code<'{', '}'>(content);
					doneLexeme();
					this->addAlias(name, content);
				}
				else if(opt == "namespace")
				{
					this->namespace_ = ident();
					doneLexeme();
				}
				else if(opt == "token")
				{
					code<'{', '}'>(this->tokencontent);
					doneLexeme();
				}
				else
					syntaxError("Unknown option");
				
				if(cur() != ';') syntaxError("Expected ';' after option");
				next();
			}
			else
			{
				std::string id = ident();
				doneLexeme();
				int state = 0;
				
				if(firstIdent())
				{
					std::string statestr = ident();
					doneLexeme();
					state = boost::lexical_cast<int>(statestr);
				}
				
				if(cur() == '<')
				{
					std::auto_ptr<Rule> rule(new Rule(id));
					
					code<'<', '>'>(rule->args);
					doneLexeme();
					
					if(cur() != '=') syntaxError("Expected '=' in rule");
					next();
					
					rule->def = def();
					
					if(cur() != ';') syntaxError("Expected ';' after rule");
					next();
					
					this->addRule(rule.release());
				}
				else if(cur() == ':')
				{
					next();
					
					std::auto_ptr<Token> token(new Token(id));
					
					bool inStr = false, ignore = false;
	
					while(true)
					{
						int c = cur();
						
						if(!ignore)
						{
							if(c == '\\')
							{
								ignore = true;
							}
							else if(c == '"')
							{
								inStr = !inStr;
							}
							else if(c == '{' || c == ';' || c == '=')
							{
								if(!inStr)
									break;
							}
						}
						else
							ignore = false;
						
						if(c == -1)
							syntaxError("Expected end of regular expression");
						
						nextInLexeme();
						
						token->def += (char)c;
					}
					
					if(cur() == '{')
					{
						code<'{', '}'>(token->code);
						doneLexeme();
					}
					else if(cur() == '=')
					{
						next();
						
						while(true)
						{
							if(firstIdent())
							{
								std::string t = ident();
								doneLexeme();
								token->copy = t;
								code<'<', '>'>(token->code);
								doneLexeme();
							}
							else if(cur() == '{')
							{
								std::string str;
								code<'{', '}'>(str);
								doneLexeme();
								token->code2 += str;
							}
							else
								break;
							
							/*
							if(t == "@linecounter")
								token->linecounter = true;
							else if(t == "@skip")
								token->skip = true;
							else
							{
								token->copy = t;
								code<'<', '>'>(token->code);
								doneLexeme();
								//break;
							}*/
						}
					}
					
					if(cur() != ';') syntaxError("Expected ';' after token");
					next();
					
					this->addToken(token.release(), state);
				}
				else
					syntaxError("Expected rule parameters or token definition");
			}
		}
		//std::list<Node *>& nodes
	}
};


#endif //GRAMMAR_H

