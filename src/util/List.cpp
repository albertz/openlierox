/* OpenLieroX
 * List as a serializeable CustomVar
 * code under LGPL
 * by Albert Zeyer, 2012-03-08
 */

#include "util/List.h"
#include "gusanos/luaapi/classes.h"
#include "CBytestream.h"

DynamicList::DynamicList() {
	typeId = SVT_BOOL;
	thisRef.classId = LuaID<DynamicList>::value;
}

ScriptVar_t DynamicList::defaultValue() const {
	assert(type() != SVT_CustomWeakRefToStatic); // we cannot have any senseful default which can be used by reset() or others
	return ScriptVar_t::FromType(type());
}

void DynamicList::reset() {
	 for(size_t i = 0; i < size(); ++i)
		 writeGeneric(i, defaultValue());
}

CustomVar* DynamicList::copy() const { return new DynamicList(*this); }

void DynamicList::copyFrom(const CustomVar& o) {
	const DynamicList* ol = dynamic_cast<const DynamicList*>(&o);
	assert(ol != NULL);
	*this = *ol;
}

bool DynamicList::operator==(const CustomVar& o) const {
	const DynamicList* ol = dynamic_cast<const DynamicList*>(&o);
	if(ol == NULL) return false;
	if(size() != ol->size()) return false;
	for(size_t i = 0; i < size(); ++i)
		if(getGeneric(i) != ol->getGeneric(i)) return false;
	return true;
}

bool DynamicList::operator<(const CustomVar& o) const {
	const DynamicList* ol = dynamic_cast<const DynamicList*>(&o);
	if(ol == NULL) return this < &o;
	for(size_t i = 0; i < std::max(size(), ol->size()); ++i) {
		if(i >= size()) return true;
		if(i >= ol->size()) return false; // *this > o
		if(getGeneric(i) != ol->getGeneric(i))
			return getGeneric(i) < ol->getGeneric(i);
	}
	// they are equal
	return false;
}

std::string DynamicList::toString() const {
	std::string r = "[";
	for(size_t i = 0; i < size(); ++i) {
		if(i != 0) r += ", ";
		r += getGeneric(i).toString();
	}
	r += "]";
	return r;
}

bool DynamicList::fromString(const std::string & str) { return false; }

Result DynamicList::toBytestream(CBytestream* bs, const CustomVar* diffTo) const {
	const DynamicList* diffToList = NULL;
	if(diffTo) {
		diffToList = dynamic_cast<const DynamicList*>(diffTo);
		assert(diffToList != NULL);
		assert(type() == diffToList->type());
	}

	bs->writeInt(size(), 4);

	size_t numChangesOld = 0;
	size_t numChangesDefault = 0;
	if(diffToList) {
		for(size_t i = 0; i < size(); ++i) {
			ScriptVar_t value = getGeneric(i);
			if(value != ScriptVar_t::FromType(type())) numChangesDefault++;
			ScriptVar_t otherVal(ScriptVar_t::FromType(type()));
			if(i < diffToList->size()) otherVal = diffToList->getGeneric(i);
			if(value != otherVal) numChangesOld++;
		}
	}

	if(diffToList && numChangesOld < numChangesDefault) {
		bs->writeInt(CUSTOMVAR_STREAM_DiffToOld, 1);
		for(size_t i = 0; i < size(); ++i) {
			ScriptVar_t value = getGeneric(i);
			ScriptVar_t otherVal(ScriptVar_t::FromType(type()));
			bool haveOtherVal = false;
			if(i < diffToList->size()) {
				otherVal = diffToList->getGeneric(i);
				haveOtherVal = true;
			}
			if(value == otherVal) continue;

			bs->writeInt(i, 4);
			if(haveOtherVal && otherVal.isCustomType())
				bs->writeVar(value, otherVal.customVar());
			else
				bs->writeVar(value);
		}
		bs->writeInt(-1, 4);
	}
	else {
		bs->writeInt(CUSTOMVAR_STREAM_DiffToDefault, 1);
		for(size_t i = 0; i < size(); ++i)
			bs->writeVar(getGeneric(i));
	}

	return true;
}

Result DynamicList::fromBytestream(CBytestream* bs, bool expectDiffToDefault) {
	size_t newSize = bs->readInt(4);
	if(!canResize() && newSize != size()) {
		errors << "DynamicList::fromBytestream: resize from " << size() << " to " << newSize << " not supported" << endl;
		bs->SkipAll(); // messed up
		return "resize invalid";
	}
	resize(newSize);

	int streamType = bs->readInt(1);
	switch(streamType) {
	case CUSTOMVAR_STREAM_DiffToDefault:
		reset();
		for(size_t i = 0; i < size(); ++i) {
			ScriptVar_t value;
			bs->readVar(value);
			writeGeneric(i, value);
		}
		break;

	case CUSTOMVAR_STREAM_DiffToOld:
		if(expectDiffToDefault) {
			errors << "CustomVar::fromBytestream: got unexpected diffToOld data on " << thisRef.description();
			// continue anyway, there might be still something useful
		}
		while(true) {
			int index = bs->readInt(4);
			if(index < 0) break;
			if((uint)index >= size()) {
				errors << "DynamicList::fromBytestream: got invalid index" << endl;
				bs->SkipAll(); // messed up
				return "invalid index";
			}
			ScriptVar_t value = getGeneric(index);
			bs->readVar(value);
			writeGeneric(index, value);
		}
		break;

	default:
		errors << "CustomVar::fromBytestream: got invalid streamType " << streamType << " for object " << thisRef.description() << endl;
		// It doesn't really make sense to continue.
		// This cannot be a bug, the stream is messed up.
		return "stream messed up. invalid streamtype";
	}

	return true;
}

REGISTER_CLASS(DynamicList, LuaID<CustomVar>::value)
