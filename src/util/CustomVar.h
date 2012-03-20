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

#define CUSTOMVAR_STREAM_DiffToDefault 1
#define CUSTOMVAR_STREAM_DiffToOld 2

// for custom variables types (not just string/bool/int/float)
// This can be used in the scriptable vars system.
struct CustomVar : BaseObject {
	typedef ::Ref<CustomVar> Ref;
	virtual ~CustomVar() {}

	virtual void reset();
	virtual CustomVar* copy() const;
	virtual bool operator==(const CustomVar&) const;
	virtual bool operator<(const CustomVar&) const;
	virtual std::string toString() const;
	virtual bool fromString(const std::string & str);

	virtual void copyFrom(const CustomVar&);
	virtual void fromScriptVar(const ScriptVar_t& v);
	virtual Result toBytestream( CBytestream* bs, const CustomVar* diffTo = NULL ) const;
	virtual Result fromBytestream( CBytestream* bs, bool expectDiffToDefault = true );

	Result ToBytestream( CBytestream* bs, const CustomVar* diffTo = NULL ) const; // includes type-signature
	static Ref FromBytestream( CBytestream* bs );
	bool operator!=(const CustomVar& o) const { return !(*this == o); }

	static LuaReference metaTable;
	static void initMetaTable();
	virtual LuaReference getMetaTable() const { return metaTable; }
};

struct NullCustomVar : CustomVar {
	virtual CustomVar* copy() const { return new NullCustomVar(); }
	virtual std::string toString() const { return "NullCustomVar"; }
	virtual bool fromString(const std::string & str) { return false; }
};

#endif
