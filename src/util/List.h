/* OpenLieroX
 * List as a serializeable CustomVar
 * code under LGPL
 * by Albert Zeyer, 2012-03-08
 */

#ifndef OLX_LIST_H
#define OLX_LIST_H

#include <vector>
#include "util/CustomVar.h"

template<typename T, typename ImplType = std::vector<T> >
struct List : CustomVar {
	virtual CustomVar* copy() const { return new List(*this); }
	virtual bool operator==(const CustomVar&) const { return false; }
	virtual bool operator<(const CustomVar&) const { return false; }
	virtual std::string toString() const { return ""; }
	virtual bool fromString( const std::string & str) { return true; }

	virtual void copyFrom(const CustomVar&) {}
	virtual void fromScriptVar(const ScriptVar_t& v) {}
	virtual Result toBytestream( CBytestream* bs ) const { return true; }
	virtual Result fromBytestream( CBytestream* bs ) { return true; }
};

#endif // OLX_LIST_H
