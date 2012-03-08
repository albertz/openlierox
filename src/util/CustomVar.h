/*
 *  CustomVar.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 24.02.10.
 *  code under LGPL
 *
 */

#ifndef __OLX_UTIL_CUSTOMVAR_H__
#define __OLX_UTIL_CUSTOMVAR_H__

#include <string>
#include "Ref.h"
#include "util/BaseObject.h"
#include "util/Result.h"

class CBytestream;
class ScriptVar_t;

// for custom variables types (not just string/bool/int/float)
// This can be used in the scriptable vars system.
struct CustomVar : BaseObject {
	typedef ::Ref<CustomVar> Ref;
	virtual ~CustomVar() {}

	virtual CustomVar* copy() const;
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const = 0;
	virtual bool fromString(const std::string & str) = 0;

	virtual void copyFrom(const CustomVar&);
	virtual void fromScriptVar(const ScriptVar_t& v);
	virtual Result toBytestream( CBytestream* bs ) const;
	virtual Result fromBytestream( CBytestream* bs );

	Result ToBytestream( CBytestream* bs ) const; // includes type-signature
	static Ref FromBytestream( CBytestream* bs );
	bool operator!=(const CustomVar& o) const { return !(*this == o); }
};

#endif
